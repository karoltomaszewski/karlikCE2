#pragma once
#include "Board.h"

namespace move {
	class Move {
	public:
		Move() = default;
		virtual std::string getMovePgn() = 0;
		int xFrom;
		int yFrom;
		int xTo;
		int yTo;
	};

	class NormalMove : public Move {
	public:
		NormalMove() = default;
		NormalMove(int xFrom, int yFrom, int xTo, int yTo);
		virtual std::string getMovePgn();
		float evaluation;
	};
}