#include "Move.h"
#include "Engine.h"

move::CaptureMove::CaptureMove(int xFrom, int yFrom, int xTo, int yTo, char pieceName) {

	this->xFrom = xFrom;
	this->yFrom = yFrom;
	this->xTo = xTo;
	this->yTo = yTo;
	this->pieceName = pieceName;
}

std::string move::CaptureMove::getMoveICCF() {

	return std::to_string(this->xFrom) + std::to_string(this->yFrom) + std::to_string(this->xTo) + std::to_string(this->yTo);
}
