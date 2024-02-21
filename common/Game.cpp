#include "Game.h"

#include "Network.h"

#include <cassert>
#include <iostream>

namespace Common
{
Game::Game(Params params)
    : mParams(params)
{
    mState = std::make_unique<PositionState[]>(
        params.height * params.width);

    for (uint32_t i = 0; i < mParams.height; ++i)
    {
        for (uint32_t j = 0; j < mParams.width; ++j)
        {
            PositionState* p = GetPosition(j, i);
            p->pos = Position{ j, i };
        }
    }
}

Game::~Game()
{ }

std::pair<Game::PlayerState*, bool> Game::CreatePlayer(
    const std::string& address,
    uint32_t port)
{
    using namespace std::chrono;

    for (auto& [id, state] : mPlayers)
    {
        if (state.address == address && state.port == port)
        {
            return std::make_pair(&state, false);
        }
    }

    uint32_t id = mNextPlayerId++;
    auto [it, inserted] = mPlayers.try_emplace(id, PlayerState{});
    PlayerState* state = &it->second;

    {
        assert(inserted);
        state->player = std::make_unique<Player>(id, *this);
        state->address = address;
        state->port = port;
        state->connectStart = steady_clock::now();
        state->lastMessage = steady_clock::now();
    }

    return std::make_pair(state, true);
}

Game::PlayerState* Game::CreatePlayer(uint32_t id)
{
    using namespace std::chrono;

    PlayerState* state = GetPlayerById(id);

    if (!state)
    {
        auto [it, inserted] = mPlayers.try_emplace(id, PlayerState{});
        PlayerState* state = &it->second;

        state->player = std::make_unique<Player>(id, *this);
        state->connectStart = steady_clock::now();
        state->lastMessage = steady_clock::now();

        return state;
    }

    return nullptr;
}

Game::PlayerState* Game::GetPlayerById(uint32_t id)
{
    if (auto it = mPlayers.find(id); it != mPlayers.end())
    {
        return &it->second;
    }
    return nullptr;
}

const Game::PlayerState* Game::GetPlayerById(uint32_t id) const
{
    if (auto it = mPlayers.find(id); it != mPlayers.end())
    {
        return &it->second;
    }
    return nullptr;
}

Game::PositionState* Game::GetPosition(
    uint32_t x,
    uint32_t y) const
{
    assert(mState);
    assert(IsValidPosition(x, y));
    return &mState[y * mParams.height + x];
}

bool Game::IsValidPosition(uint32_t x, uint32_t y) const
{
    return x < mParams.width && y < mParams.height;
}

void Game::OnMessage(Action action, NetworkMessage& msg)
{
    Span<const uint8_t> data(msg.buffer.Data(), msg.buffer.Size());

    switch (action)
    {
    case Action::Login:
    {
        std::optional<LoginMessage> login = Serializer<LoginMessage>::Deserialize(data);

        if (!login)
        {
            std::cout << "Failed to parse login message from '" << msg.address << ':'
                << msg.port << "'\n";
            return;
        }

        auto ev = std::make_unique<LoginEvent>();
        ev->action = action;
        ev->msg = std::move(msg);
        ev->login = std::move(*login);

        mEvents.emplace_back(std::move(ev));
        break;
    }
    case Action::Ping:
    {
        std::optional<PingMessage> ping = Serializer<PingMessage>::Deserialize(data);

        if (!ping)
        {
            std::cout << "Failed to parse ping message from '" << msg.address << ':'
                << msg.port << "'\n";
            return;
        }

        auto ev = std::make_unique<PingEvent>();
        ev->action = action;
        ev->msg = std::move(msg);
        ev->ping = std::move(*ping);

        mEvents.emplace_back(std::move(ev));
        break;
    }
    case Action::Acknowledge:
    {
        std::optional<AcknowledgeMessage> ack
            = Serializer<AcknowledgeMessage>::Deserialize(data);

        if (!ack)
        {
            std::cout << "Failed to parse acknowledge message from '" << msg.address << ':'
                << msg.port << "'\n";
            return;
        }

        auto ev = std::make_unique<AcknowledgeEvent>();
        ev->action = action;
        ev->msg = std::move(msg);
        ev->ack = std::move(*ack);

        mEvents.emplace_back(std::move(ev));
        break;
    }
    default:
        break;
    }
}

bool Game::Tick()
{
    while (!mEvents.empty())
    {
        std::unique_ptr<Event> ev = std::move(mEvents.front());
        mEvents.pop_front();

        switch (ev->action)
        {
        case Action::Login:
            HandleLogin(static_cast<LoginEvent*>(ev.get()));
            break;
        case Action::Ping:
            HandlePing(static_cast<PingEvent*>(ev.get()));
            break;
        case Action::Acknowledge:
            HandleAcknowledge(static_cast<AcknowledgeEvent*>(ev.get()));
            break;
        }
    }

    return true;
}
}
