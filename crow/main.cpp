#include "crow.h"
#include "FEN.h"
#include "Engine.h"

int main()
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

        return crow::response(s.notation);
    });

    //set the port, set the app to run on multiple threads, and run the app
    app.port(2000).multithreaded().run();
}
/*
int main(int argc, char* argv[])
{
    std::string fen = argv[1];

    engine::Engine engine(fen);

    engine::Engine::bestMoveStructure s = engine.findBestMove();

    std::cout << s.notation << std::endl;

    return 0;
}
*/