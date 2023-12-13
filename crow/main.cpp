#include "FEN.h"
#include "Engine.h"
#include <iostream>
/*int main()
{

    crow::SimpleApp app; //define your crow application

    //define your endpoint at the root directory
    CROW_ROUTE(app, "/")([]() {
        return std::to_string(7*7);
    });

    CROW_ROUTE(app, "/post").methods("POST"_method)([&app](const crow::request& req) {
        std::cout << req.body;
        auto x = crow::json::load(req.body);

        if (!x)
            return crow::response(crow::status::BAD_REQUEST);

        std::string f = std::string(x["fen"]);

        engine::Engine engine(f);

        engine::Engine::bestMoveStructure s = engine.findBestMove();

        return crow::response(s.notation + " : " + std::to_string(s.evaluation));
    });

    //set the port, set the app to run on multiple threads, and run the app
    app.port(2000).multithreaded().run();
}
*/
int main(int argc, char* argv[])
{
    //std::string fen = "rn1qkb1r/1pp1ppp1/p2p1Bbp/3P4/4P1PP/2N2P2/PPP5/R2QKBNR b KQkq - 0 9";
    std::string fen = argv[1];
    std::string drawPositions = argv[2];
    //std::string drawPositions = ""

    engine::Engine engine(fen, drawPositions);

    engine::Engine::bestMoveStructure s = engine.findBestMove();

    std::cout << s.notation << std::endl;
    std::cout << std::to_string(s.evaluation) << std::endl;

    return 0;
}
