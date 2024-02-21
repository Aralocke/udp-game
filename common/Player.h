#pragma once

#include "Common.h"

namespace Common
{
constexpr uint32_t kInvalidPlayerId{ UINT32_MAX };

class Player final
{
public:
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

public:
    Player() = default;
    Player(uint32_t id, Game& game);
    ~Player();

    operator bool() const;
    bool IsValid() const;

    uint32_t GetId() const;
    uint32_t GetX() const;
    uint32_t GetY() const;

    const Game& GetGame() const;
    Game& GetGame();

private:
    Position mPos;
    uint32_t mId{ kInvalidPlayerId };
    Game* mGame{ nullptr };
};
}
