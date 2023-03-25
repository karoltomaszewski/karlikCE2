#pragma once

#include <string>
namespace FEN
{
	class FEN
	{
	public:
		FEN() = default;
		FEN(std::string fen);
		std::string getRawFen();
		std::string getPosition();
		std::string getColor();
		std::string getCastleInfo();
		std::string rawFen;

		static const char PAWN_WHITE;
		static const char KNIGHT_WHITE;
		static const char BISHOP_WHITE;
		static const char ROOK_WHITE;
		static const char QUEEN_WHITE;
		static const char KING_WHITE;

		static const char PAWN_BLACK;
		static const char KNIGHT_BLACK;
		static const char BISHOP_BLACK;
		static const char ROOK_BLACK;
		static const char QUEEN_BLACK;
		static const char KING_BLACK;

		static const std::string COLOR_WHITE;
		static const std::string COLOR_BLACK;
	private:
		std::string getNthSegment(int n);
	};
}