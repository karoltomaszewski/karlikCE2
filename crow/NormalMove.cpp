#include "Move.h"
#include "Board.h"
#include "Engine.h"

move::NormalMove::NormalMove(int xFrom, int yFrom, int xTo, int yTo) {

	this->xFrom = xFrom;
	this->yFrom = yFrom;
	this->xTo = xTo;
	this->yTo = yTo;
}

std::string move::NormalMove::getMovePgn() {
	std::string alpha[8] = { "a", "b", "c", "d", "e", "f", "g", "h" };

	return std::to_string(this->xFrom) + std::to_string(this->yFrom);
}