#include "Move.h"
#include "Engine.h"

move::CastleMove::CastleMove(int xFrom, int yFrom, int xTo, int yTo) {

	this->xFrom = xFrom;
	this->yFrom = yFrom;
	this->xTo = xTo;
	this->yTo = yTo;
}

std::string move::CastleMove::getMoveICCF() {

	return std::to_string(this->xFrom) + std::to_string(this->yFrom) + std::to_string(this->xTo) + std::to_string(this->yTo);
}
