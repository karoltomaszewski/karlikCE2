#include "FEN.h"
#include <vector>

FEN::FEN::FEN(std::string fen)
{
	this->rawFen = fen;
}

const char FEN::FEN::PAWN_WHITE = 'P';
const char FEN::FEN::KNIGHT_WHITE = 'N';
const char FEN::FEN::BISHOP_WHITE = 'B';
const char FEN::FEN::ROOK_WHITE = 'R';
const char FEN::FEN::QUEEN_WHITE = 'Q';
const char FEN::FEN::KING_WHITE = 'K';

const char FEN::FEN::PAWN_BLACK = 'p';
const char FEN::FEN::KNIGHT_BLACK = 'n';
const char FEN::FEN::BISHOP_BLACK = 'b';
const char FEN::FEN::ROOK_BLACK = 'r';
const char FEN::FEN::QUEEN_BLACK = 'q';
const char FEN::FEN::KING_BLACK = 'k';

const std::string FEN::FEN::COLOR_WHITE = "w";
const std::string FEN::FEN::COLOR_BLACK = "b";


std::string FEN::FEN::getRawFen()
{
	return rawFen;
}

std::string FEN::FEN::getPosition()
{
	return this->getNthSegment(1);
}

std::string FEN::FEN::getColor()
{
	return this->getNthSegment(2);
}


// private

std::string FEN::FEN::getNthSegment(int n)
{
	std::string s = rawFen;
	std::string segment = s;
	for (int i = 0; i < n; i++)
	{
		int delimeterPos = s.find(" ");
		segment = s.substr(0, delimeterPos);
		s = s.substr(delimeterPos + 1);
	}

	return segment;
}