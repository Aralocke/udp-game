#pragma once

#include <cassert>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>

namespace Common
{
// Common Forward Declarations for Types
class Game;
class Player;

// Common enum corresponding to a single movement
enum class Movement : uint8_t
{
    Left,
    Right,
    Up,
    Down
};

// Represents a single position on a 2D grid
struct Position
{
    uint32_t x{ UINT32_MAX };
    uint32_t y{ UINT32_MAX };

    Position() = default;
    Position(uint32_t x, uint32_t y);
};

constexpr size_t StringLength(const char* s, size_t length)
{
    const char* start = s;
    const char* end = s + length;

    while (start < end && *start)
    {
        ++start;
    }

    return size_t(start - s);
}

constexpr size_t StringLength(const char* s)
{
    return StringLength(s, SIZE_MAX);
}

// define a span class since we don't have one with C++17
template<typename T>
struct Span
{
    T* data{ nullptr };
    size_t size{ 0 };

    Span() = default;
    Span(T* data, size_t size)
        : data(data)
        , size(size)
    { }

    Span<T> Subspan(size_t offset, size_t end = size_t(~0))
    {
        assert(offset <= size);

        if (end != size_t(~0))
        {
            assert(end <= size);
            assert(end >= offset);
        }
        else
        {
            end = size;
        }

        return Span<T>(data + offset, end - offset);
    }
};

// define a RAII type to call a function when the destructor is called
template<typename T>
class ScopeGuard final
{
public:
    template<typename Fn>
    friend ScopeGuard<std::decay_t<Fn>> CreateScopeGuard(Fn fn);

    ~ScopeGuard() { m_obj(); }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) = delete;

private:
    explicit ScopeGuard(T o) : m_obj(std::move(o)) { }

private:
    T m_obj;
};

//
//
//
template<typename Fn>
static ScopeGuard<std::decay_t<Fn>> CreateScopeGuard(Fn fn)
{
    return ScopeGuard<std::decay_t<Fn>>(std::move(fn));
}

#define _COMMON_JOIN(A, B) A ## B
#define COMMON_JOIN(A, B) _COMMON_JOIN(A, B)
#define COMMON_ANONYMOUS_SYMBOL(A) COMMON_JOIN(A, __COUNTER__)
#define SCOPE_GUARD(...) \
    auto COMMON_ANONYMOUS_SYMBOL(_anonymous_) = CreateScopeGuard(__VA_ARGS__);

// handle windows error codes and convert them to strings
std::string ErrorToString(int err);

// convert a wstring to a utf-8 encoded string
std::string WideToUtf8(std::wstring_view str);
}
