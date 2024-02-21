#include "GameLoop.h"

#include <iostream>

namespace Server
{
GameLoop::GameLoop(Common::Game::Params params)
    : Game(std::move(params))
{ }

GameLoop::~GameLoop() = default;

void GameLoop::HandleLogin(LoginEvent* ev)
{
    using namespace Common;

    std::string address = AddressToString(ev->login.address);
    auto [state, created] = CreatePlayer(address, ev->login.port);

    if (created)
    {
        std::cout << "Created new player entry for '" << address << ':'
            << ev->login.port << "'" << '\n';
    }
    else
    {
        std::cout << "Existing player entry found for '" << address << ':'
            << ev->login.port << "'" << '\n';
    }

    NetworkBuffer buffer;
    Span<uint8_t> data{ buffer.Data(), buffer.Capacity() };

    LoginMessage login;
    login.message.messageId = state->nextMessage++;
    memcpy(login.address, ev->login.address, sizeof(login.address));
    login.port = ev->login.port;
    login.session = state->player->GetId();
    buffer.SetOffset(Serializer<LoginMessage>::Serialize(login, data));

    if (!SendMessage(address.c_str(), ev->login.port, buffer))
    {
        std::cout << "Failed to send login message back to client '" << address << ':'
            << ev->login.port << "'" << '\n';
    }
    else
    {
        std::cout << "Sent login message back to client '" << address << ':'
            << ev->login.port << "'" << '\n';

        state->messages.try_emplace(
            login.message.messageId,
            std::move(buffer));
    }
}

void GameLoop::HandlePing(PingEvent* ev)
{
    using namespace Common;
    using namespace std::chrono;

    PingMessage& ping = ev->ping;

    if (ping.playerId == PingMessage::kInvalidPlayer)
    {
        std::cout << "Invalid playerId on ping from '" << ev->msg.address
            << ':' << ev->msg.port << "'\n";
        return;
    }

    PlayerState* state = GetPlayerById(ping.playerId);

    if (!state)
    {
        std::cout << "Unknown playerId '" << ping.playerId << "' on ping from '"
            << ev->msg.address << ':' << ev->msg.port << "'\n";
        return;
    }

    state->lastMessage = steady_clock::now();

    // TODO: Do something with the ping.messageId and ping.message.messageId
    //       values. If the client is behind on ACKs we should have the buffered
    //       messages to resend.

    NetworkBuffer buffer;
    Span<uint8_t> data{ buffer.Data(), buffer.Capacity() };

    AcknowledgeMessage ack;
    ack.message.messageId = state->nextMessage++;
    ack.messageId = ping.messageId;
    buffer.SetOffset(Serializer<AcknowledgeMessage>::Serialize(ack, data));

    if (!SendMessage(state->address.c_str(), ev->msg.port, buffer))
    {
        std::cout << "Failed to send acknowledge message back to client '"
            << state->address << ':' << ev->msg.port << "'" << '\n';
    }
    else
    {
        std::cout << "Sent acknowledge message back to client '"
            << state->address << ':' << ev->msg.port << "'" << '\n';

        state->messages.try_emplace(
            ack.message.messageId,
            std::move(buffer));
    }
}

void GameLoop::HandleAcknowledge(AcknowledgeEvent* ev)
{}
}
