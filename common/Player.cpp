#include "Player.h"

#include "Game.h"

#include <cassert>

namespace Common
{
Player::Player(uint32_t id, Game& game)
    : mPos(0, 0)
    , mId(id)
    , mGame(&game)
{ }

Player::~Player()
{ }

Player::operator bool() const
{
    return IsValid();
}

const Game& Player::GetGame() const
{
    assert(mGame);
    return *mGame;
}

Game& Player::GetGame()
{
    assert(mGame);
    return *mGame;
}

uint32_t Player::GetId() const
{
    return mId;
}

uint32_t Player::GetX() const
{
    return mPos.x;
}

uint32_t Player::GetY() const
{
    return mPos.y;
}

bool Player::IsValid() const
{
    return mId != kInvalidPlayerId;
}
}
