#pragma once

#include "Common.h"
#include "Message.h"
#include "Player.h"

#include <chrono>
#include <deque>
#include <memory>
#include <unordered_map>

namespace Common
{
enum class Action : uint32_t;
struct NetworkMessage;

class Game
{
public:
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

public:
    struct Params
    {
        // Max height of the game board
        uint32_t height{ 64 };

        // Max width of the game board
        uint32_t width{ 64 };

        // Maximum number of players in the game.
        uint32_t maxPlayers{ 8 };

        // Duration to assume lack of any network traffic is a timeout.
        std::chrono::milliseconds playerTimeout{
            std::chrono::milliseconds(2000) };
    };

public:
    Game(Params params);
    virtual ~Game();

    const Params& GetParams() const { return mParams; }
    Params& GetParams() { return mParams; }
    virtual void OnMessage(Action action, NetworkMessage& msg);
    virtual bool Tick();

protected:
    struct Game::Event
    {
        NetworkMessage msg;
        Action action{ Action::None };
    };

    struct AcknowledgeEvent : public Game::Event
    {
        AcknowledgeMessage ack;
    };

    struct LoginEvent : public Game::Event
    {
        LoginMessage login;
    };

    struct PingEvent : public Game::Event
    {
        PingMessage ping;
    };

protected:
    virtual void HandleLogin(LoginEvent* ev) = 0;
    virtual void HandlePing(PingEvent* ev) = 0;
    virtual void HandleAcknowledge(AcknowledgeEvent* ev) = 0;

protected:
    struct PlayerState
    {
        // Player which this state represents
        std::unique_ptr<Player> player;
        // Network Information
        std::string address;
        uint32_t port{ 0 };
        // Connection time
        std::chrono::steady_clock::time_point connectStart;
        // Last Message time
        std::chrono::steady_clock::time_point lastMessage;
        // Message state
        uint32_t nextMessage{ 0 };
        uint32_t ackCount{ 0 };
        // Message Buffer
        std::unordered_map<uint32_t, NetworkBuffer> messages;
    };

    struct PositionState
    {
        // Position in the game
        Position pos;
        // Player at this poistion (if one is currently there)
        Player* player{ nullptr };
    };

    bool IsValidPosition(uint32_t x, uint32_t y) const;
    PositionState* GetPosition(uint32_t x, uint32_t y) const;

    const PlayerState* GetPlayerById(uint32_t id) const;
    PlayerState* GetPlayerById(uint32_t id);

    // Used by the Server Loop
    std::pair<PlayerState*, bool> CreatePlayer(
        const std::string& address,
        uint32_t port);

    // Used by the Client Loop
    PlayerState* CreatePlayer(uint32_t id);

private:
    Params mParams;
    std::deque<std::unique_ptr<Event>> mEvents;

private:
    // Game State
    std::unique_ptr<PositionState[]> mState;
    std::unordered_map<uint32_t, PlayerState> mPlayers;
    uint32_t mNextPlayerId{ 1 };
};
}
