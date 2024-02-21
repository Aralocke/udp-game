#include "TestMessages.h"

#include "Message.h"

#include <cassert>
#include <iostream>

namespace Tests
{
void TestMessageSerializer()
{
    using namespace Common;

    NetworkBuffer buffer;

    Span<uint8_t> data{ buffer.Data(), buffer.Capacity() };
    Span<const uint8_t> constData{ buffer.Data(), buffer.Capacity() };
    Span<const uint8_t> payload;

    {
        Message message;
        message.action = Action::Login;
        message.messageId = 99;

        size_t serializedSize = Serializer<Message>::Serialize(
            message,
            data);
        assert(serializedSize == kMessageSize);

        payload = constData.Subspan(kMessageHeaderSize, serializedSize);

        Span<uint8_t> headerData = data.Subspan(0, kMessageHeaderSize);
        SerializeHeader(message.header, headerData, payload);
    }

    std::optional<Message> result = Serializer<Message>::Deserialize(
        constData);

    assert(result.has_value());
    assert(result->action == Action::Login);
    assert(result->messageId == 99);
    assert(result->header.payloadSize == payload.size);
}

void TestLoginMessageSerializer()
{
    using namespace Common;

    NetworkBuffer buffer;

    Span<uint8_t> data{ buffer.Data(), buffer.Capacity() };
    Span<const uint8_t> constData{ buffer.Data(), buffer.Capacity() };

    LoginMessage login;
    login.message.messageId = 99;
    login.session = 99;

    size_t serializedSize = Serializer<LoginMessage>::Serialize(
        login,
        data);
    assert(serializedSize == kLoginMessageSize);

    Span<const uint8_t> payload = constData.Subspan(kMessageHeaderSize, serializedSize);

    std::optional<LoginMessage> result = Serializer<LoginMessage>::Deserialize(
        constData);

    assert(result.has_value());
    assert(result->message.action == login.message.action);
    assert(result->message.messageId == login.message.messageId);
    assert(result->message.header.payloadSize == kMessageHeaderSize + payload.size);
    assert(result->message.header.hash == login.message.header.hash);
}

void TestPingMessageSerializer()
{
    using namespace Common;

    NetworkBuffer buffer;

    Span<uint8_t> data{ buffer.Data(), buffer.Capacity() };
    Span<const uint8_t> constData{ buffer.Data(), buffer.Capacity() };

    PingMessage ping;
    ping.message.messageId = 99;
    ping.playerId = 99;
    ping.messageId = 99;

    size_t serializedSize = Serializer<PingMessage>::Serialize(
        ping,
        data);
    assert(serializedSize == kPingMessageSize);

    Span<const uint8_t> payload = constData.Subspan(kMessageHeaderSize, serializedSize);

    std::optional<PingMessage> result = Serializer<PingMessage>::Deserialize(
        constData);

    assert(result.has_value());
    assert(result->message.action == ping.message.action);
    assert(result->message.messageId == ping.message.messageId);
    assert(result->message.header.payloadSize == kMessageHeaderSize + payload.size);
    assert(result->message.header.hash == ping.message.header.hash);
    assert(result->messageId == ping.messageId);
}

void TestAckMessageSerializer()
{
    using namespace Common;

    NetworkBuffer buffer;

    Span<uint8_t> data{ buffer.Data(), buffer.Capacity() };
    Span<const uint8_t> constData{ buffer.Data(), buffer.Capacity() };

    AcknowledgeMessage ack;
    ack.message.messageId = 99;
    ack.messageId = 99;

    size_t serializedSize = Serializer<AcknowledgeMessage>::Serialize(
        ack,
        data);
    assert(serializedSize == kAckMessageSize);

    Span<const uint8_t> payload = constData.Subspan(kMessageHeaderSize, serializedSize);

    std::optional<AcknowledgeMessage> result = Serializer<AcknowledgeMessage>::Deserialize(
        constData);

    assert(result.has_value());
    assert(result->message.action == ack.message.action);
    assert(result->message.messageId == ack.message.messageId);
    assert(result->message.header.payloadSize == kMessageHeaderSize + payload.size);
    assert(result->message.header.hash == ack.message.header.hash);
    assert(result->messageId == ack.messageId);
}

void MessageTests()
{
    std::cout << "Running message tests...\n";
    TestMessageSerializer();
    TestLoginMessageSerializer();
    TestAckMessageSerializer();
    TestPingMessageSerializer();
    std::cout << "All message tests completed\n";
}
}
