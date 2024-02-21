#pragma once

#include "Game.h"

namespace Client
{
class GameLoop : public Common::Game
{
private:
    enum class State : uint32_t
    {
        New = 0,
        LoggedIn,
        TimedOut,
        Disconnected
    };

public:
    struct Params
    {
        Common::Game::Params commonParams;
        std::string clientAddress;
        uint32_t clientPort;
        std::string serverAddress;
        uint32_t serverPort;
    };

    GameLoop(Params params);
    ~GameLoop() override;

public:
    void HandleLogin(LoginEvent* ev) override;
    void HandlePing(PingEvent* ev) override;
    void HandleAcknowledge(AcknowledgeEvent* ev) override;

public:
    bool Tick() override;

private:
    void TryLogin();
    void TryPing();

private:
    Params mParams;
    State mState{ State::New };
    PlayerState* mThisPlayer{ nullptr };

private:
    std::chrono::steady_clock::time_point mLoginAttemptTime;
    uint32_t mLoginAttempts{ 0 };
    std::chrono::steady_clock::time_point mLastPing;
};
}
