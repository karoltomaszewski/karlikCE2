#include "Move.h"
#include "Board.h"
#include "Engine.h"

move::NormalMove::NormalMove(board::Board board, int xFrom, int yFrom, int xTo, int yTo) {
}

std::string move::NormalMove::getMovePgn() {
	std::string alpha[8] = { "a", "b", "c", "d", "e", "f", "g", "h" };

	return "abc";
}