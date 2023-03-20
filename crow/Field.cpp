#include "Board.h"

board::Field::Field(int x, int y)
{
	this->x = x;
	this->y = y;
	this->isFieldEmpty = true;
}

void board::Field::setPiece(pieces::Piece piece)
{
	this->piece = piece;
	this->isFieldEmpty = false;
}

pieces::Piece board::Field::getPiece()
{
	return this->piece;
}