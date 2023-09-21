#pragma once
#include <string>

namespace move {
	class Move {
	public:
		Move() = default;
		virtual std::string getMoveICCF() = 0;
		int xFrom;
		int yFrom;
		int xTo;
		int yTo;
		int promotionCode;
		virtual std::string getType() = 0;
	};

	class NormalMove : public Move {
	public:
		NormalMove() = default;
		NormalMove(int xFrom, int yFrom, int xTo, int yTo);
		virtual std::string getMoveICCF();
		virtual std::string getType() { return "normal"; };
	};

	class PromotionMove : public Move {
	public:
		PromotionMove() = default;
		PromotionMove(int xFrom, int yFrom, int xTo, int yTo, int promotionCode);
		virtual std::string getMoveICCF();
		virtual std::string getType() { return "promotion"; };
	};

	class CastleMove : public Move {
	public:
		CastleMove() = default;
		CastleMove(int xFrom, int yFrom, int xTo, int yTo);
		virtual std::string getMoveICCF();
		virtual std::string getType() { return "castle"; };
	};

	class EnPassantMove : public Move {
	public:
		EnPassantMove() = default;
		EnPassantMove(int xFrom, int yFrom, int xTo, int yTo);
		virtual std::string getMoveICCF();
		virtual std::string getType() { return "enPassant"; }
	};
}