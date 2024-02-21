#include "Server.h"

#include <cassert>
#include <iostream>
#include <thread>

namespace Common
{
UdpServer::UdpServer(std::string address, uint32_t port)
    : mAddress(std::move(address))
    , mPort(port)
{
    assert(!mAddress.empty());
    assert(mPort > 0 && mPort <= 65535);
}

UdpServer::~UdpServer()
{
    using namespace Common;

    Shutdown();
    if (mSocket != kInvalidSocket)
    {
        ::closesocket(mSocket);
        mSocket = kInvalidSocket;
    }
}

bool UdpServer::Initialize()
{
    using namespace Common;

    std::lock_guard lock(mMutex);

    if (mSocket != kInvalidSocket)
    {
        return true;
    }

    SockAddrStorage storage;
    sockaddr* addr = ToSockAddr(storage.data(), mAddress.c_str(), mPort);

    if (!addr)
    {
        const int err = WSAGetLastError();

        std::cout << "Failed to convert address '" << mAddress << ':' << mPort << "': [" << err
            << "] " << ErrorToString(err) << '\n';

        return false;
    }

    // Create the listening socket
    mSocket = WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, 0);

    if (mSocket == kInvalidSocket)
    {
        const int err = WSAGetLastError();

        std::cout << "Failed to create listening socket: [" << err
            << "] " << ErrorToString(err) << '\n';

        return false;
    }

    if (!SetNonBlocking(mSocket))
    {
        const int err = WSAGetLastError();

        std::cout << "Failed to set non-blocking on listening socket '" << mSocket
            << "': [" << err << "] " << ErrorToString(err) << '\n';

        return false;
    }

    if (int result = ::bind(mSocket, addr, storage.size());
        result == SOCKET_ERROR)
    {
        const int err = WSAGetLastError();

        std::cout << "Failed to bind socket '" << mSocket << "' to address '"
            << mAddress << ':' << mPort << "' : [" << err << "] "
            << ErrorToString(err) << '\n';

        return false;
    }

    return true;
}

void UdpServer::OnRecv(RecvFn fn)
{
    mRecvFn = std::move(fn);
}

void UdpServer::Run(
    std::chrono::steady_clock::duration interval,
    std::function<bool()> tick)
{
    using namespace Common;
    using namespace std::chrono;

    std::unique_lock lock(mMutex);

    assert(mSocket != kInvalidSocket);
    assert(duration_cast<milliseconds>(interval).count() > 0);
    assert(tick);
    assert(mRecvFn);
    assert(!mShutdown);

    steady_clock::time_point lastTick;

    std::cout << "UDP Server running on '" << mAddress << ':' << mPort << "'\n";
    SCOPE_GUARD([] { std::cout << "UDP Server stopped\n"; });

    while (!mShutdown)
    {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(mSocket, &reads);

        struct timeval timeout;
        memset(&timeout, 0, sizeof(timeval));
        {
            auto secs = duration_cast<seconds>(interval);

            timeout.tv_sec = static_cast<long>(secs.count());
            timeout.tv_usec = static_cast<long>(
                duration_cast<microseconds>(
                    interval - secs).count());
        }

        lock.unlock();

        auto start = steady_clock::now();
        int result = ::select(0 /* ignored */, &reads, nullptr, nullptr, &timeout);
        auto stop = steady_clock::now();

        lock.lock();

        if (mShutdown)
        {
            return;
        }
        else if (result == SOCKET_ERROR)
        {
            // And error occured while waiting for network activity
            const int err = WSAGetLastError();

            if (err != WSAEINTR)
            {
                std::cout << "Error reading from server socket: [" << err << "] "
                    << ErrorToString(err) << '\n';
                return;
            }
        }
        else if (result == 0)
        {
            // Timeout occurred listening for new sockets. We waited the full
            // interval and nothing happened so we can just fall through here.
        }
        else
        {
            if (FD_ISSET(mSocket, &reads))
            {
                // Read from the socket until we get an EWOULDBLOCK
                // This generates events for the game loop to process on
                // the next call to tick().

                while (true)
                {
                    // We have something available on the on the socket to
                    // read from.
                    NetworkMessage msg;

                    SockAddrStorage storage = { 0 };
                    auto* address = reinterpret_cast<sockaddr*>(storage.data());
                    int socklen = int(storage.size());

                    int result = recvfrom(
                        mSocket,
                        reinterpret_cast<char*>(msg.buffer.Data()),
                        int(msg.buffer.Capacity()),
                        0,
                        address,
                        &socklen);

                    if (result == SOCKET_ERROR)
                    {
                        int err = WSAGetLastError();

                        // Specifically break out of the while loop if we encounter this
                        // error. We have no more messages to process.
                        if (err == WSAEWOULDBLOCK)
                        {
                            break;
                        }

                        // The error list here is some of the common transient errors that
                        // may occur and can be ignored for UDP networking since they are
                        // connectionless.
                        else if (err != WSAECONNRESET && err != WSAECONNREFUSED)
                        {
                            std::cout << "Failed to recvfrom on server socket '" << mSocket
                                << "': [" << err << "] " << ErrorToString(err) << '\n';
                            continue;
                        }

                        // Assume anything else is a fatal error and exit.
                        return;
                    }

                    const sockaddr_in* add4 = reinterpret_cast<const sockaddr_in*>(
                        storage.data());

                    msg.buffer.SetOffset(size_t(result));
                    msg.address = AddressToString(&add4->sin_addr);
                    msg.port = ntohs(add4->sin_port);

                    mRecvFn(msg);
                }
            }
        }

        // If the amount of time since the last tick has exceeded the interval
        // time we will tick the callback once.
        bool ticked = false;
        if (steady_clock::now() - lastTick > interval)
        {
            if (!tick())
            {
                // This is the only way for the Game/loops to break the server and cause
                // and exit for the application.
                return;
            }
            lastTick = steady_clock::now();
            ticked = true;
        }

        // For now we assume if we tick that the next tick should be a fuill
        // interval from the last one. A slight optimization could be added here
        // to ensure we tick again but for simplicity this ensures we have events
        // process.

        auto wait = (ticked)
            ? interval
            : std::min(stop - start, interval);

        mCond.wait_for(lock, wait);
    }
}

void UdpServer::Shutdown()
{
    bool expected = false;
    if (mShutdown.compare_exchange_strong(expected, true))
    {
        Wakeup();
    }
}

void UdpServer::Wakeup()
{
    bool expected = false;
    if (mNotified.compare_exchange_strong(expected, true))
    {
        std::cout << "Wakeup()\n";
    }

    mCond.notify_all();
}
}
