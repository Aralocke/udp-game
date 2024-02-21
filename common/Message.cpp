#include "Message.h"

namespace Common
{
uint64_t FNV1A_64(const void* data, size_t size)
{
    static constexpr uint64_t kFnv1aSeed{ 14695981039346656037ULL };
    static constexpr uint64_t kFnv1aPrime{ 1099511628211ULL };

    const auto* d = static_cast<const uint8_t*>(data);
    uint64_t seed = kFnv1aSeed;

    while (size-- > 0)
    {
        seed = (seed ^ *d++) * kFnv1aPrime;
    }

    return seed;
}

class MemoryReader final
{
public:
    MemoryReader(const void* data, size_t size)
        : mData(static_cast<const uint8_t*>(data))
        , mSize(size)
    { }

    ~MemoryReader() = default;

    size_t Offset() const { return mOffset; }

    size_t Remaining() const { return mSize - mOffset; }

    void Reset() { Seek(0); }

    void Seek(size_t offset)
    {
        assert(offset < mSize);
        mOffset = offset;
    }

    void Skip(size_t count)
    {
        Seek(mOffset + count);
    }

    size_t Size() const { return mSize; }

    Span<const uint8_t> GetSpan() const
    {
        return { mData + mOffset, Remaining() };
    }

public:
    uint8_t Read() { return Read(mOffset); }
    uint8_t Read(size_t offset)
    {
        if (offset <= mSize)
        {
            Seek(offset);
            return mData[mOffset++];
        }
        return 0;
    }

    uint16_t Read16_BE()
    {
        if (mOffset + 2 <= mSize)
        {
            uint16_t value = (uint16_t(mData[mOffset]) << 8)
                + uint16_t(mData[mOffset + 1]);

            mOffset += 2;
            return value;
        }
        return 0;
    }
    uint16_t Read16_BE(size_t offset)
    {
        if (offset + 2 <= mSize)
        {
            Seek(offset);
            uint16_t value = (uint16_t(mData[mOffset]) << 8)
                + uint16_t(mData[mOffset + 1]);

            mOffset += 2;
            return value;
        }
        return 0;
    }

    uint32_t Read32_BE()
    {
        if (mOffset + 4 <= mSize)
        {
            uint32_t value = (uint32_t(mData[mOffset]) << 24)
                + (uint32_t(mData[mOffset + 1]) << 16)
                + (uint32_t(mData[mOffset + 2]) << 8)
                + uint32_t(mData[mOffset + 3]);

            mOffset += 4;
            return value;
        }
        return 0;
    }
    uint32_t Read32_BE(size_t offset)
    {
        if (offset + 4 <= mSize)
        {
            Seek(offset);
            uint32_t value = (uint32_t(mData[mOffset]) << 24)
                + (uint32_t(mData[mOffset + 1]) << 16)
                + (uint32_t(mData[mOffset + 2]) << 8)
                + uint32_t(mData[mOffset + 3]);

            mOffset += 4;
            return value;
        }
        return 0;
    }

    uint64_t Read64_BE()
    {
        if (mOffset + 8 <= mSize)
        {
            uint64_t value = uint64_t(uint64_t(mData[mOffset]) << 56)
                + uint64_t(uint64_t(mData[mOffset + 1]) << 48)
                + uint64_t(uint64_t(mData[mOffset + 2]) << 40)
                + uint64_t(uint64_t(mData[mOffset + 3]) << 32)
                + uint64_t(uint64_t(mData[mOffset + 4]) << 24)
                + uint64_t(uint64_t(mData[mOffset + 5]) << 16)
                + uint64_t(uint64_t(mData[mOffset + 6]) << 8)
                + uint64_t(mData[mOffset + 7]);

            mOffset += 8;
            return value;
        }
        return 0;
    }
    uint64_t Read64_BE(size_t offset)
    {
        if (offset + 4 <= mSize)
        {
            Seek(offset);
            uint64_t value = uint64_t(uint64_t(mData[mOffset]) << 56)
                + uint64_t(uint64_t(mData[mOffset + 1]) << 48)
                + uint64_t(uint64_t(mData[mOffset + 2]) << 40)
                + uint64_t(uint64_t(mData[mOffset + 3]) << 32)
                + uint64_t(uint64_t(mData[mOffset + 4]) << 24)
                + uint64_t(uint64_t(mData[mOffset + 5]) << 16)
                + uint64_t(uint64_t(mData[mOffset + 6]) << 8)
                + uint64_t(mData[mOffset + 7]);

            mOffset += 8;
            return value;
        }
        return 0;
    }

    Span<const uint8_t> ReadSpan(size_t length)
    {
        return ReadSpan(mOffset, length);
    }
    Span<const uint8_t> ReadSpan(size_t offset, size_t length)
    {
        if (offset + length <= mSize)
        {
            Seek(offset);
            Span<const uint8_t> data{
                mData + mOffset,
                length
            };

            mOffset += length;
            return data;
        }
        return {};
    }

    
private:
    const uint8_t* mData{ nullptr };
    size_t mSize{ 0 };
    size_t mOffset{ 0 };
};

class MemoryWriter final
{
public:
    MemoryWriter(void* data, size_t size)
        : mData(static_cast<uint8_t*>(data))
        , mSize(size)
    { }

    ~MemoryWriter() = default;

    size_t Offset() const { return mOffset; }

    size_t Remaining() const { return mSize - mOffset; }

    void Reset() { Seek(0); }

    void Seek(size_t offset)
    {
        assert(offset < mSize);
        mOffset = offset;
    }

    void Skip(size_t count)
    {
        Seek(mOffset + count);
    }

    size_t Size() const { return mSize; }

    Span<const uint8_t> GetSpan() const
    {
        return { mData + mOffset, Remaining() };
    }

public:
    void Put(
        const void* data,
        size_t size)
    {
        Put(mOffset, data, size);
    }
    void Put(
        size_t offset,
        const void* data,
        size_t size)
    {
        if (offset + size <= mSize)
        {
            Seek(offset);

            memcpy(mData + mOffset, data, size);
            mOffset += size;
        }
    }

    void Put16_BE(uint16_t value)
    {
        Put16_BE(mOffset, value);
    }
    void Put16_BE(size_t offset, uint16_t value)
    {
        uint8_t values[2] = {
            uint8_t(value >> 8),
            uint8_t(value)
        };

        Put(offset, values, sizeof(values));
    }

    void Put32_BE(uint32_t value)
    {
        Put32_BE(mOffset, value);
    }
    void Put32_BE(size_t offset, uint32_t value)
    {
        uint8_t values[4] = {
            uint8_t(value >> 24),
            uint8_t(value >> 16),
            uint8_t(value >> 8),
            uint8_t(value)
        };

        Put(offset, values, sizeof(values));
    }

    void Put64_BE(uint64_t value)
    {
        Put64_BE(mOffset, value);
    }
    void Put64_BE(size_t offset, uint64_t value)
    {
        uint8_t values[8] = {
            uint8_t(value >> 56),
            uint8_t(value >> 48),
            uint8_t(value >> 40),
            uint8_t(value >> 32),
            uint8_t(value >> 24),
            uint8_t(value >> 16),
            uint8_t(value >> 8),
            uint8_t(value)
        };

        Put(offset, values, sizeof(values));
    }

    void PutZero(size_t count)
    {
        PutZero(mOffset, count);
    }
    void PutZero(size_t offset, size_t count)
    {
        if (offset + count <= mSize)
        {
            Seek(offset);

            memset(mData + mOffset, 0, count);
            mOffset += count;
        }
    }

private:
    uint8_t* mData{ nullptr };
    size_t mSize{ 0 };
    size_t mOffset{ 0 };
};

bool DeserializeHeader(
    Span<const uint8_t> data,
    MessageHeader& header)
{
    assert(data.size >= kMessageHeaderSize);
    MemoryReader reader(data.data, data.size);

    // Read the two magic bytes
    header.magic[0] = reader.Read();
    header.magic[1] = reader.Read();
    // Read the hash value
    header.hash = reader.Read64_BE();
    // Read the payload size
    header.payloadSize = reader.Read32_BE();

    assert(reader.Remaining() == 0);

    return header.magic[0] == MessageHeader::kMagicBytes[0]
        && header.magic[1] == MessageHeader::kMagicBytes[1]
        && header.hash > 0
        && header.payloadSize > 0;
}

void SerializeHeader(
    MessageHeader& header,
    Span<uint8_t> buffer,
    Span<const uint8_t> data)
{
    assert(buffer.size >= kMessageHeaderSize);
    MemoryWriter writer(buffer.data, kMessageHeaderSize);

    // Set the magic bytes (byte-0 and byte-1)
    header.magic[0] = MessageHeader::kMagicBytes[0];
    header.magic[1] = MessageHeader::kMagicBytes[1];
    {
        writer.Put(header.magic, sizeof(header.magic));
    }
    // Set the message FNV1A-64 hash
    header.hash = FNV1A_64(data.data, data.size);
    {
        writer.Put64_BE(header.hash);
    }
    // set the payload size
    header.payloadSize = data.size;
    {
        writer.Put32_BE(header.payloadSize);
    }
}
template<>
static std::optional<Message> Serializer<Message>::Deserialize(
    Span<const uint8_t> input)
{
    if (input.size < kMessageHeaderSize)
    {
        return std::nullopt;
    }

    Message message;
    MessageHeader& header = message.header;
    Span<const uint8_t> headerData = input.Subspan(0, kMessageHeaderSize);

    if (!DeserializeHeader(headerData, header))
    {
        return std::nullopt;
    }
    if (header.payloadSize > kNetworkBufferSize
        || header.payloadSize > input.size)
    {
        return std::nullopt;
    }

    Span<const uint8_t> messageData = input.Subspan(kMessageHeaderSize);
        //kMessageHeaderSize + header.payloadSize);
    MemoryReader reader(messageData.data, messageData.size);

    if (reader.Size() < 8)
    {
        return std::nullopt;
    }

    message.action = Action(reader.Read32_BE());
    message.messageId = reader.Read32_BE();
    // message.data = reader.GetSpan();

    return message;
}

template<>
static size_t Serializer<Message>::Serialize(
    Message& message,
    Span<uint8_t> output)
{
    if (output.size < kMessageSize)
    {
        return 0;
    }

    Span<uint8_t> messageData = output.Subspan(kMessageHeaderSize);

    MemoryWriter writer(messageData.data, messageData.size);
    writer.Put32_BE(uint32_t(message.action));
    writer.Put32_BE(message.messageId);

    return kMessageHeaderSize + writer.Offset();
}

template class Serializer<Message>;

template<>
static std::optional<LoginMessage>
Serializer<LoginMessage>::Deserialize(
    Span<const uint8_t> input)
{
    std::optional<Message> result = Serializer<Message>::Deserialize(input);

    if (!result || result->action != Action::Login)
    {
        return std::nullopt;
    }

    LoginMessage login;
    {
        login.message = std::move(*result);
    }
    Message& message = login.message;
    MessageHeader& header = message.header;

    Span<const uint8_t> data = input.Subspan(kMessageSize);
    MemoryReader reader(data.data, data.size);

    if (reader.Size() < kLoginMessagePayload)
    {
        return {};
    }

    // Session ID
    login.session = reader.Read32_BE();
    // Target IP Address for the return
    login.address[0] = reader.Read();
    login.address[1] = reader.Read();
    login.address[2] = reader.Read();
    login.address[3] = reader.Read();
    // Target Port for the return
    login.port = reader.Read16_BE();

    return login;
}

template<>
static size_t Serializer<LoginMessage>::Serialize(
    LoginMessage& login,
    Span<uint8_t> output)
{
    if (output.size < kLoginMessageSize)
    {
        return 0;
    }

    // Pointer to and counter for the amount of data we can store
    // in the buffer and still have enough room for the message
    // header.
    Span<uint8_t> payloadData = output.Subspan(kMessageSize);
    MemoryWriter writer(payloadData.data, payloadData.size);

    {
        // Write the entries for the login message
        // Session ID
        writer.Put32_BE(login.session);
        // IP Address to reply too
        writer.Put(login.address, sizeof(login.address));
        // Port to connect too
        writer.Put16_BE(login.port);
    }

    // Now we serialize the entries for the Message member
    Message& message = login.message;
    Span<uint8_t> messageData = output.Subspan(0, kMessageSize);
    size_t messageSize = Serializer<Message>::Serialize(message, messageData);
    size_t payloadSize = messageSize + writer.Offset();

    // Lastly we writte the hashed and final header value
    Span<uint8_t> headerData = messageData.Subspan(0, kMessageHeaderSize);
    Span<const uint8_t> payload{
        output.data + kMessageHeaderSize,
        payloadSize
    };
    SerializeHeader(message.header, headerData, payload);

    // We return the number of bytes written to the buffer
    assert(messageSize == kMessageSize);
    return messageSize + writer.Offset();
}

template class Serializer<LoginMessage>;

template<>
static std::optional<PingMessage>
Serializer<PingMessage>::Deserialize(
    Span<const uint8_t> input)
{
    std::optional<Message> result = Serializer<Message>::Deserialize(input);

    if (!result || result->action != Action::Ping)
    {
        return std::nullopt;
    }

    PingMessage ping;
    {
        ping.message = std::move(*result);
    }
    Message& message = ping.message;
    MessageHeader& header = message.header;

    Span<const uint8_t> data = input.Subspan(kMessageSize);
    MemoryReader reader(data.data, data.size);

    if (reader.Size() < kPingMessageSize - kMessageSize)
    {
        return {};
    }

    ping.playerId = reader.Read32_BE();
    ping.messageId = reader.Read64_BE();

    return ping;
}

template<>
static size_t Serializer<PingMessage>::Serialize(
    PingMessage& ping,
    Span<uint8_t> output)
{
    if (output.size < kPingMessageSize)
    {
        return 0;
    }

    // Pointer to and counter for the amount of data we can store
    // in the buffer and still have enough room for the message
    // header.
    Span<uint8_t> payloadData = output.Subspan(kMessageSize);
    MemoryWriter writer(payloadData.data, payloadData.size);

    // Write the entries for the ping message
    writer.Put32_BE(ping.playerId);
    writer.Put64_BE(ping.messageId);

    // Now we serialize the entries for the Message member
    Span<uint8_t> messageData = output.Subspan(0, kMessageSize);
    Message& message = ping.message;
    size_t messageSize = Serializer<Message>::Serialize(message, messageData);
    size_t payloadSize = messageSize + writer.Offset();

    assert(messageSize == kMessageSize);

    // Lastly we writte the hashed and final header value
    Span<uint8_t> headerData = output.Subspan(0, kMessageHeaderSize);
    Span<const uint8_t> payload{
        output.data + kMessageHeaderSize,
        payloadSize
    };
    SerializeHeader(message.header, headerData, payload);

    // We return the number of bytes written to the buffer
    return kMessageSize + writer.Offset();
}

template class Serializer<PingMessage>;

template<>
static std::optional<AcknowledgeMessage>
Serializer<AcknowledgeMessage>::Deserialize(
    Span<const uint8_t> input)
{
    std::optional<Message> result = Serializer<Message>::Deserialize(input);

    if (!result || result->action != Action::Acknowledge)
    {
        return std::nullopt;
    }

    AcknowledgeMessage ack;
    {
        ack.message = std::move(*result);
    }
    Message& message = ack.message;
    MessageHeader& header = message.header;

    Span<const uint8_t> data = input.Subspan(kMessageSize);
    MemoryReader reader(data.data, data.size);

    if (reader.Size() < kAckMessageSize - kMessageSize)
    {
        return {};
    }

    ack.messageId = reader.Read64_BE();

    return ack;
}

template<>
static size_t Serializer<AcknowledgeMessage>::Serialize(
    AcknowledgeMessage& ack,
    Span<uint8_t> output)
{
    if (output.size < kAckMessageSize)
    {
        return 0;
    }

    // Pointer to and counter for the amount of data we can store
    // in the buffer and still have enough room for the message
    // header.
    Span<uint8_t> payloadData = output.Subspan(kMessageSize);
    MemoryWriter writer(payloadData.data, payloadData.size);

    // Write the entries for the login message
    writer.Put64_BE(ack.messageId);

    // Now we serialize the entries for the Message member
    Span<uint8_t> messageData = output.Subspan(0, kMessageSize);
    Message& message = ack.message;
    size_t messageSize = Serializer<Message>::Serialize(message, messageData);
    size_t payloadSize = messageSize + writer.Offset();

    assert(messageSize == kMessageSize);

    // Lastly we writte the hashed and final header value
    Span<uint8_t> headerData = output.Subspan(0, kMessageHeaderSize);
    Span<const uint8_t> payload{
        output.data + kMessageHeaderSize,
        payloadSize
    };
    SerializeHeader(message.header, headerData, payload);

    // We return the number of bytes written to the buffer
    return kMessageSize + writer.Offset();
}

template class Serializer<AcknowledgeMessage>;
}
