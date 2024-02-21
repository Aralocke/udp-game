// Common Includes
#include "Game.h"
#include "Message.h"
#include "Network.h"
#include "Server.h"
// Server Includes
#include "GameLoop.h"
#include "Tests.h"
// Other Includes
#include <iostream>

#include <Windows.h>

namespace
{
using ShutdownFn = std::function<void()>;
static ShutdownFn shutdownFn;
}

int main(int argc, char** argv)
{
    using namespace std::chrono_literals;

    using namespace Common;

    auto CtrlHandler = [](DWORD ev) -> BOOL
    {
        switch (ev)
        {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
            if (shutdownFn)
            {
                shutdownFn();
            }
            return TRUE;
        default:
            return FALSE;
        }
    };

    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE))
    {
        std::cout << "Failed to set console handler\n";
        return 1;
    }

    Tests::RunAllTests();

    WinSock winsock;

    Common::Game::Params params;
    params.playerTimeout = 10s;  // 2000ms
    Server::GameLoop game(params);

    std::string address = "127.0.0.1";
    uint32_t port = 8088;

    UdpServer server(address, port);
    {
        if (!server.Initialize())
        {
            std::cout << "Failed to initialize server on '" << address << ":"
                << port << "'\n";
            return 1;
        }

        server.OnRecv(
            [&game](Common::NetworkMessage& msg)
            {
                Span<const uint8_t> data(msg.buffer.Data(), msg.buffer.Size());
                std::optional<Message> result = Serializer<Message>::Deserialize(data);

                if (!result)
                {
                    std::cout << "invalid message received from '" << msg.address << "'";
                }
                else
                {
                    std::cout << "received message type '" << uint32_t(result->action) << "' from '"
                        << msg.address << ':' << msg.port << "' (payload="
                        << result->header.payloadSize << ", hash=" << result->header.hash
                        << ")" << '\n';

                    game.OnMessage(result->action, msg);
                }
            });

        shutdownFn = [&server] { server.Shutdown(); };
    }

    server.Run(30ms, [&game]() -> bool { return game.Tick(); });
    return 0;
}
