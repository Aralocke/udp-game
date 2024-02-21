#pragma once

#include "Common.h"

#include <array>
#include <memory>

#include <WinSock2.h>
#include <ws2tcpip.h>

#ifdef ERROR
#undef ERROR
#endif
#ifdef INVALID_SOCKET
#undef INVALID_SOCKET
#endif
#ifdef SendMessage
#undef SendMessage
#endif

namespace Common
{
// RAII Wrapper for WinSock Initialization
struct WinSock
{
    WSADATA data;

    WinSock();
    ~WinSock();
};

// Socket Definitions
using Socket = intptr_t;
constexpr Socket kInvalidSocket = Socket(~0);

// Address Handling
using SockAddrStorage = std::array<uint8_t, sizeof(sockaddr_storage)>;
constexpr size_t kAddr4SockLen = sizeof(sockaddr_in);

// Message Handling
// Network Buffer is 1472. The standard MTU is 1500 bytes, a UDP header
// is 8 bytes, and the IP header is 20 bytes (1500-20-8=1472).
constexpr size_t kNetworkBufferSize = 1472;

// Structure to wrap data transfered over the network. Moveable, not copyable.
class NetworkBuffer final
{
public:
    NetworkBuffer(const NetworkBuffer&) = delete;
    NetworkBuffer& operator=(const NetworkBuffer&) = delete;

public:
    NetworkBuffer()
        : mBuffer(std::make_unique<uint8_t[]>(kNetworkBufferSize))
        , mSize(kNetworkBufferSize)
        , mOffset(0)
    { }

    NetworkBuffer(NetworkBuffer&& other) noexcept
        : mBuffer(std::move(other.mBuffer))
        , mSize(std::exchange(other.mSize, 0))
        , mOffset(std::exchange(other.mOffset, 0))
    { }

    NetworkBuffer& operator=(NetworkBuffer&& other) noexcept
    {
        if (mBuffer.get() != other.mBuffer.get())
        {
            if (mBuffer)
            {
                mBuffer.reset();
            }
            mBuffer = std::move(other.mBuffer);
            mSize = std::exchange(other.mSize, 0);
            mOffset = std::exchange(other.mOffset, 0);
        }
        return *this;
    }

    size_t Capacity() const { return mSize; }
    const uint8_t* Data() const { return mBuffer.get(); }
    uint8_t* Data() { return mBuffer.get(); }
    size_t Size() const { return mOffset; }

    void SetOffset(size_t offset) { mOffset = offset; }

private:
    std::unique_ptr<uint8_t[]> mBuffer;  // Data storage
    size_t mSize{ 0 };  // Size of the allocation
    size_t mOffset{ 0 };  // Number of used bytes
};

// Define a stanndard message type for dealing with data read from the
// network.
struct NetworkMessage
{
    std::string address;
    uint32_t port{ 0 };
    NetworkBuffer buffer;
    
    NetworkMessage() = default;
    NetworkMessage(NetworkMessage&& other) noexcept
        : address(std::move(other.address))
        , port(other.port)
        , buffer(std::move(other.buffer))
    { }

    NetworkMessage& operator=(NetworkMessage&& other) noexcept
    {
        if (this != &other)
        {
            address = std::move(other.address);
            port = std::exchange(other.port, 0);
            buffer = std::move(other.buffer);
        }
        return *this;
    }

    NetworkMessage(const NetworkMessage&) = delete;
    NetworkMessage& operator=(const NetworkMessage&) = delete;
};

std::string AddressToString(const void* address);

bool CreateSocketPair(Socket& reader, Socket& writer);

bool DrainSocket(Socket sock);

// Send an encoded NetworkBuffer to the address:port target given.
// This function is not asynchronous and will create the socket
// and send the message now.
bool SendMessage(
    const char* address,
    uint32_t port,
    const NetworkBuffer& buf);

bool SetNonBlocking(Socket sock);

sockaddr* ToSockAddr(
    uint8_t* storage,
    const char* address,
    uint32_t port);
}
