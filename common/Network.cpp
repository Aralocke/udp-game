#include "Network.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

namespace Common
{
WinSock::WinSock()
{
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
    {
        ::exit(EXIT_FAILURE);
    }
    if (HIBYTE(data.wVersion) != 2 || LOBYTE(data.wVersion) != 2)
    {
        ::exit(EXIT_FAILURE);
    }
}

WinSock::~WinSock()
{
    WSACleanup();
}

std::string AddressToString(const void* address)
{
    char buffer[32] = { 0 };
    if (!inet_ntop(AF_INET, address, buffer, sizeof(buffer)))
    {
        return {};
    }

    std::string str(buffer, StringLength(buffer, sizeof(buffer)));
    return str;
}

bool CreateSocketPair(Socket& reader, Socket& writer)
{
    Socket listener{ kInvalidSocket };
    Socket client{ kInvalidSocket };
    Socket server{ kInvalidSocket };

    SCOPE_GUARD([&] { closesocket(listener); });
    SCOPE_GUARD([&] { closesocket(client); });
    SCOPE_GUARD([&] { closesocket(server); });

    struct sockaddr_in addr;

    // Create a listening socket
    if (listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        listener == kInvalidSocket)
    {
        int err = WSAGetLastError();
        std::cout << "Failed to create socket pair listener socket: ["
            << err << "] " << ErrorToString(err) << '\n';

        return false;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = 0; // Use any available port
    
    if (int res = bind(listener, (struct sockaddr*)&addr, sizeof(addr));
        res == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        std::cout << "Failed to bind socket pair listener socket: ["
            << err << "] " << ErrorToString(err) << '\n';

        return false;
    }

    // Start listening
    
    if (int res = listen(listener, 1);  res == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        std::cout << "Failed to listen on socket '" << listener
            << "' with error: [" << err << "] " << ErrorToString(err) << '\n';

        return false;
    }

    // Get the port number
    int addrlen = sizeof(addr);
    if (int res = getsockname(listener, (struct sockaddr*)&addr, &addrlen);
        res == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        std::cout << "Failed getsockname on socket '" << listener
            << "' with error: [" << err << "] " << ErrorToString(err) << '\n';

        return false;
    }

    // Create the connecting socket
    if (client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        client == kInvalidSocket)
    {
        int err = WSAGetLastError();
        std::cout << "Failed to create socket pair reader socket: ["
            << err << "] " << ErrorToString(err) << '\n';

        return false;
    }

    // Connect to the listening socket
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (int res = connect(client, (struct sockaddr*)&addr, sizeof(addr));
        res == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        std::cout << "Failed to connect socket pair reader socket: ["
            << err << "] " << ErrorToString(err) << '\n';

        return false;
    }

    // Accept the connection
    if (server = accept(listener, nullptr, nullptr); server == kInvalidSocket)
    {
        int err = WSAGetLastError();
        std::cout << "Failed to accept on listener socket pair socket: ["
            << err << "] " << ErrorToString(err) << '\n';

        return false;
    }

    if (!SetNonBlocking(client))
    {
        int err = WSAGetLastError();
        std::cout << "Failed to set reader socket non-blocking: [" << err
            << "] " << ErrorToString(err) << '\n';

        return false;
    }

    if (!SetNonBlocking(server))
    {
        int err = WSAGetLastError();
        std::cout << "Failed to set writer socket non-blocking: [" << err
            << "] " << ErrorToString(err) << '\n';

        return false;
    }

    // Return the connected sockets
    reader = std::exchange(client, kInvalidSocket);
    writer = std::exchange(server, kInvalidSocket);

    return true;
}

bool DrainSocket(Socket sock)
{
    constexpr size_t kBufferSize = 16;
    char buffer[kBufferSize] = { 0 };

    DWORD flags = 0;

    WSABUF buf;
    buf.buf = buffer;
    buf.len = kBufferSize;

    for (;;)
    {
        DWORD received = 0;
        int32_t result = WSARecv(
            sock,
            &buf,
            1,
            &received,
            &flags,
            NULL, NULL);

        if (result == SOCKET_ERROR)
        {
            int err = WSAGetLastError();

            if (err != WSAEWOULDBLOCK)
            {
                std::cout << "Failed to drain socket '" << sock << "': [" << err
                    << "] " << ErrorToString(err) << '\n';
                return false;
            }

            // We have no more data left to read
            break;
        }
    }

    return true;
}

bool SendMessage(
    const char* address,
    uint32_t port,
    const NetworkBuffer& buffer)
{
    assert(buffer.Size() <= kNetworkBufferSize);
    SockAddrStorage storage = { 0 };

    auto* sockaddr = ToSockAddr(storage.data(), address, port);
    int socklen = storage.size();

    Socket sock = WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, 0);
    if (sock == kInvalidSocket)
    {
        int err = WSAGetLastError();
        std::cout << "Failed to create client socket: [" << err
            << "] " << ErrorToString(err) << '\n';

        return false;
    }

    SCOPE_GUARD([&] { closesocket(sock); });

    WSABUF buf;
    buf.buf = reinterpret_cast<char*>(
        const_cast<uint8_t*>(buffer.Data()));
    buf.len = static_cast<unsigned long>(buffer.Size());

    DWORD bytesSent = 0;
    int result = WSASendTo(
        sock,
        &buf, 1,
        &bytesSent,
        0,
        sockaddr, socklen,
        NULL, NULL);

    if (result == SOCKET_ERROR)
    {
        int err = WSAGetLastError();
        std::cout << "Failed to send data to '" << sock << "' ("
            << address << ':' << port << "): [" << err << "] "
            << ErrorToString(err) << '\n';

        return false;
    }

    return true;
}

bool SetNonBlocking(Socket sock)
{
    unsigned long value = 1;
    return ::ioctlsocket(sock, FIONBIO, &value) != SOCKET_ERROR;
}

sockaddr* ToSockAddr(
    uint8_t* storage,
    const char* address,
    uint32_t port)
{
    assert(storage);
    assert(address);
    assert(port >= 0 && port <= 65535);

    sockaddr* addr = reinterpret_cast<sockaddr*>(storage);
    sockaddr_in* in = reinterpret_cast<sockaddr_in*>(addr);

    memset(in, 0, kAddr4SockLen);

    in->sin_family = AF_INET;
    in->sin_port = htons(port);

    if (inet_pton(AF_INET, address, &in->sin_addr) == 1)
    {
        return addr;
    }

    return nullptr;
}
}
