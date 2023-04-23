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
	else if (pieceName == '-') {
		return pieces::NoPiece(pieceName);
	}

	return pieces::King(pieceName);
}

pieces::Piece pieces::PieceFactory::create(int promotionCode, std::string color)
{
	if (promotionCode == 1) {
		return color == FEN::FEN::COLOR_WHITE ? pieces::Queen('Q') : pieces::Queen('q');
	}
	else if (promotionCode == 2) {
		return color == FEN::FEN::COLOR_WHITE ? pieces::Rook('R') : pieces::Rook('r');
	}
	else if (promotionCode == 3) {
		return color == FEN::FEN::COLOR_WHITE ? pieces::Bishop('B') : pieces::Bishop('b');
	}

	return color == FEN::FEN::COLOR_WHITE ? pieces::Knight('N') : pieces::Knight('n');
}

pieces::Pawn::Pawn(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::PAWN_WHITE;
	this->pieceName = pieceName;
	this->power = 1;
}

pieces::Knight::Knight(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::KNIGHT_WHITE;
	this->pieceName = pieceName;
	this->power = 3;
}

pieces::Bishop::Bishop(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::BISHOP_WHITE;
	this->pieceName = pieceName;
	this->power = 3;
}

pieces::Rook::Rook(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::ROOK_WHITE;
	this->pieceName = pieceName;
	this->power = 5;
}

pieces::Queen::Queen(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::QUEEN_WHITE;
	this->pieceName = pieceName;
	this->power = 9;
}

pieces::King::King(char pieceName) {
	this->isWhite = pieceName == FEN::FEN::KING_WHITE;
	this->pieceName = pieceName;
	this->power = 10000;
}

pieces::NoPiece::NoPiece(char pieceName) {
	this->isWhite = false;
	this->pieceName = pieceName;
	this->isReal = false;
	this->power = 0;
}