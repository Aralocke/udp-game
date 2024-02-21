#pragma once

#include "Network.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

namespace Common
{
class UdpServer final
{
public:
    UdpServer(const UdpServer&) = delete;
    UdpServer& operator=(const UdpServer&) = delete;

public:
    UdpServer(std::string address, uint32_t port);
    ~UdpServer();

public:
    bool Initialize();
    void Run(
        std::chrono::steady_clock::duration interval,
        std::function<bool()> tick);
    void Shutdown();

public:
    using RecvFn = std::function<void(Common::NetworkMessage&)>;

    void OnRecv(RecvFn fn);

private:
    void Wakeup();

private:
    Common::Socket mSocket{ Common::kInvalidSocket };

    std::string mAddress;
    uint32_t mPort{ 8088 };

    std::atomic<bool> mShutdown{ false };
    std::atomic<bool> mNotified{ false };
    std::mutex mMutex;
    std::condition_variable mCond;

private:
    RecvFn mRecvFn;
};
}
