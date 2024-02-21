#pragma once

#include "Common.h"
#include "Network.h"

#include <optional>

namespace Common
{
    // Define the values for each action
    enum class Action : uint32_t
    {
        None = 0,
        Acknowledge,
        Login,
        Ping,
    };

    // Define a message header which should encapsulate the payload
    // structure and information being sent. We check this header
    // first to ensure a valid message over the wire.
#pragma pack(push, 1)
    struct MessageHeader
    {
        static constexpr uint8_t kMagicBytes[2] = { 0xBE, 0xEF };

        uint8_t magic[2];
        uint64_t hash;
        uint32_t payloadSize;
    };
#pragma pack(pop)

    constexpr size_t kMessageHeaderSize = sizeof(MessageHeader);

    bool DeserializeHeader(
        Span<const uint8_t> data,
        MessageHeader& header);

    void SerializeHeader(
        MessageHeader& header,
        Span<uint8_t> buffer,
        Span<const uint8_t> data);

    // Basic Message type that all other messages inherit from.
    // This contains the basic values for the header, action type,
    // and the data span which has been validated to match the message
    // header.
#pragma pack(push, 1)
    struct Message
    {
        MessageHeader header = { { 0, 0 }, 0, 0 };
        Action action{ Action::None }; // uint32_t
        uint32_t messageId = 0;
        // Span<const uint8_t> data;

        Message() = default;
        Message(Action action) : action(action) { }
    };
#pragma pack(pop)

    constexpr size_t kMessageSize = sizeof(Message); // kMessageHeaderSize + 8;

    // Defines a Login message which is the basic message to initiate a
    // game session. This is like a new player joining the game.
#pragma pack(push, 1)
    struct LoginMessage
    {
        static constexpr uint32_t kInvalidSession{ uint32_t(~0) };

        Message message;
        uint32_t session{ kInvalidSession };
        uint8_t address[4] = { 0, 0, 0, 0 };  // uint32_t
        uint16_t port = 0;

        LoginMessage() : message(Action::Login) { }
    };
#pragma pack(pop)

    constexpr size_t kLoginMessageSize = sizeof(LoginMessage);  // kMessageSize + 10;
    constexpr size_t kLoginMessagePayload = kLoginMessageSize - kMessageSize;

    // Defines a Ping messsage. The value of the ping is the last
    // acknowledged mesage ID from the other side.
#pragma pack(push, 1)
    struct PingMessage
    {
        static constexpr uint32_t kInvalidPlayer{ uint32_t(~0) };

        Message message;
        uint32_t playerId{ kInvalidPlayer };
        uint64_t messageId{ 0 };

        PingMessage() : message(Action::Ping) { }
    };
#pragma pack(pop)

    constexpr size_t kPingMessageSize = sizeof(PingMessage);  // kMessageSize + 12;
    constexpr size_t kPingMessagePayload = kPingMessageSize - kMessageSize;

    // Defines a Pong/Acknowledge messsage. The value of the messageId is the last
    // mesage ID from the other side.
#pragma pack(push, 1)
    struct AcknowledgeMessage
    {
        Message message;
        uint64_t messageId{ 0 };

        AcknowledgeMessage() : message(Action::Acknowledge) { }
    };
#pragma pack(pop)

    constexpr size_t kAckMessageSize = sizeof(AcknowledgeMessage);  // kMessageSize + 8;
    constexpr size_t kAckMessagePayload = kAckMessageSize - kMessageSize;

    // Parse a Message from the NetworkMessage. The data pointed to in the
    // Message object and any subsequent message types is owned by the
    // given NetworkMessage and that object must outlive the returned Message.
    template<typename T>
    struct Serializer
    {
        static std::optional<T> Deserialize(Span<const uint8_t> input);
        static size_t Serialize(T& message, Span<uint8_t> output);
    };
}
