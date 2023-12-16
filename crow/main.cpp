#include "FEN.h"
#include "Engine.h"
#include <iostream>

int main(int argc, char* argv[])
{
    std::string fen = argv[1];
    std::string drawPositions = argv[2];

    engine::Engine engine(fen, drawPositions);

    engine::Engine::bestMoveStructure s = engine.findBestMove();

    std::cout << s.notation << std::endl;
    std::cout << std::to_string(s.evaluation) << std::endl;

    return 0;
}
