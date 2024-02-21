#include "GameLoop.h"

#include "Network.h"

#include <iostream>

namespace Client
{
GameLoop::GameLoop(Params params)
    : Game(std::move(params.commonParams))
    , mParams(std::move(params))
{ }

GameLoop::~GameLoop() = default;

void GameLoop::HandleLogin(LoginEvent* ev)
{
    using namespace Common;

    LoginMessage& login = ev->login;

    std::string address = AddressToString(login.address);
    std::cout << "Received session '" << login.session
        << "' response from server" << '\n';

    if (mState == State::New)
    {
        // Handle the login / new seession for the current loop player.

        if (login.session == LoginMessage::kInvalidSession)
        {
            std::cout << "Received invalid session ID from server\n";
            return;
        }

        mThisPlayer = CreatePlayer(login.session);

        if (!mThisPlayer)
        {
            std::cout << "Failed to create player session '" << login.session
                << "' a player with that ID already exists\n";
            return;
        }

        // Not filled in by the call to CreatePlayer()
        mThisPlayer->address = address;
        mThisPlayer->port = login.port;

        std::cout << "Registered with server as player '" << mThisPlayer->player->GetId()
            << "'\n";
        mState = State::LoggedIn;
    }
    else
    {
        // Add the new player to the game loop
        // This is multi-client

        // use the Created-By-Id variant because we don't have a player address
        // to add.
        PlayerState* state = Game::CreatePlayer(login.session);

        if (!state)
        {
            std::cout << "Conflicted login for new player ID '" << login.session
                << "' player already exists\n";
            return;
        }

        std::cout << "New player to the game registered as playerID '"
            << login.session << "'\n";
    }
}

void GameLoop::HandlePing(PingEvent* ev)
{
}

void GameLoop::HandleAcknowledge(AcknowledgeEvent* ev)
{}

bool GameLoop::Tick()
{
    using namespace std::chrono;
    using namespace std::chrono_literals;

    bool result = Game::Tick();

    switch (mState)
    {
    case State::New:
        TryLogin();
        break;
    case State::LoggedIn:
        break;
    case State::Disconnected:
        result = false;
        break;
    }

    if (result && mState != State::New)
    {
        TryPing();
    }

    return result;
}

void GameLoop::TryLogin()
{
    using namespace Common;
    using namespace std::chrono;
    using namespace std::chrono_literals;

    if (mState != State::New)
    {
        return;
    }

    if (steady_clock::now() < mLoginAttemptTime + 500ms)
    {
        // Only try to login every 100ms or until we succeed
        return;
    }

    NetworkBuffer buffer;
    size_t offset = 0;
    {
        Span<uint8_t> data(buffer.Data(), buffer.Capacity());

        LoginMessage login;
        login.message.messageId = 0;
        login.session = 0;
        {
            SockAddrStorage storage = { 0 };
            sockaddr* address = ToSockAddr(
                storage.data(),
                mParams.clientAddress.c_str(),
                mParams.clientPort);

            const sockaddr_in* addr4 = reinterpret_cast<const sockaddr_in*>(address);
            memcpy(login.address, &addr4->sin_addr.s_addr, sizeof(login.address));
            login.port = mParams.clientPort;
        }

        offset = Serializer<LoginMessage>::Serialize(login, data);
    }
    buffer.SetOffset(offset);

    if (!SendMessage(
        mParams.serverAddress.c_str(),
        mParams.serverPort,
        buffer))
    {
        std::cout << "Failed to send login message\n";
        return;
    }

    std::cout << "Sent login message\n";
    mLoginAttemptTime = steady_clock::now();
}

void GameLoop::TryPing()
{
    using namespace Common;
    using namespace std::chrono;
    using namespace std::chrono_literals;

    if (mState != State::New)
    {
        return;
    }

    if (steady_clock::now() < mLastPing + 100ms)
    {
        // Only try to ping every 100ms
        return;
    }

    NetworkBuffer buffer;
    size_t offset = 0;
    {
        Span<uint8_t> data(buffer.Data(), buffer.Capacity());

        PingMessage ping;
        ping.message.messageId = mThisPlayer->nextMessage++;
        ping.messageId = mThisPlayer->ackCount;
        ping.playerId = mThisPlayer->player->GetId();

        offset = Serializer<PingMessage>::Serialize(ping, data);
    }
    buffer.SetOffset(offset);

    if (!SendMessage(
        mParams.serverAddress.c_str(),
        mParams.serverPort,
        buffer))
    {
        std::cout << "Failed to send ping message\n";
        return;
    }

    mLastPing = steady_clock::now();
}
}
