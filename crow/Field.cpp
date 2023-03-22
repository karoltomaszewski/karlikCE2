#include "Board.h"

board::Field::Field(int x, int y)
{
	this->x = x;
	this->y = y;
	this->isFieldEmpty = true;
	this->pieceName = '-';
}

void board::Field::setPiece(pieces::Piece piece)
{
	this->piece = piece;
	this->pieceName = piece.pieceName;
	this->isFieldEmpty = !piece.isReal;
}

pieces::Piece board::Field::getPiece()
{
	return this->piece;
}