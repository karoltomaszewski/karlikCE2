#pragma once
#include "Board.h"

namespace move {
	class Move {
	public:
		Move() = default;
		std::string getMovePgn() { return "xd"; };
	};

	class NormalMove : public Move {
	public:
		NormalMove() = default;
		NormalMove(board::Board board, int xFrom, int yFrom, int xTo, int yTo);
		std::string getMovePgn();
		board::Board board;
		float evaluation;
	};
}