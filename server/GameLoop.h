#pragma once

#include "Game.h"

namespace Server
{
class GameLoop : public Common::Game
{
public:
    GameLoop(Common::Game::Params params);
    ~GameLoop() override;

public:
    void HandleLogin(LoginEvent* ev) override;
    void HandlePing(PingEvent* ev) override;
    void HandleAcknowledge(AcknowledgeEvent* ev) override;
};
}
