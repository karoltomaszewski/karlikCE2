#include "Piece.h"
#include "FEN.h"
#include <vector>

pieces::Piece pieces::PieceFactory::create(char pieceName)
{
	if (pieceName == FEN::FEN::PAWN_BLACK || pieceName == FEN::FEN::PAWN_WHITE) {
		return pieces::Pawn(pieceName);
	}
	else if (pieceName == FEN::FEN::KNIGHT_BLACK || pieceName == FEN::FEN::KNIGHT_WHITE) {
		return pieces::Knight(pieceName);
	}
	else if (pieceName == FEN::FEN::BISHOP_BLACK || pieceName == FEN::FEN::BISHOP_WHITE) {
		return pieces::Bishop(pieceName);
	}
	else if (pieceName == FEN::FEN::ROOK_BLACK || pieceName == FEN::FEN::ROOK_WHITE) {
		return pieces::Rook(pieceName);
	}
	else if (pieceName == FEN::FEN::QUEEN_BLACK || pieceName == FEN::FEN::QUEEN_WHITE) {
		return pieces::Queen(pieceName);
	}

	return pieces::King(pieceName);
}


pieces::Pawn::Pawn(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::PAWN_WHITE;
	this->pieceName = pieceName;
}

pieces::Knight::Knight(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::KNIGHT_WHITE;
	this->pieceName = pieceName;
}

pieces::Bishop::Bishop(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::BISHOP_WHITE;
	this->pieceName = pieceName;
}

pieces::Rook::Rook(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::ROOK_WHITE;
	this->pieceName = pieceName;
}

pieces::Queen::Queen(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::QUEEN_WHITE;
	this->pieceName = pieceName;
}

pieces::King::King(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::KING_WHITE;
	this->pieceName = pieceName;
}