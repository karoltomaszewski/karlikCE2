#pragma once

#include <string>
#include "FEN.h"
#include "Move.h"
#include <vector>
#include "Board.h"
#include <map>

namespace engine
{
	class Evaluator
	{
	public:
		Evaluator() = default;
		Evaluator(std::string fen);
		double getBasicEvaluation();

		static const double PAWN_BASIC_VALUE;
		static const double KNIGH_BASIC_VALUE;
		static const double BISHOP_BASIC_VALUE;
		static const double ROOK_BASIC_VALUE;
		static const double QUEEN_BASIC_VALUE;

		static const double CODE_NO_MORE_MOVE;
		static const double CODE_WINS_KING;
		FEN::FEN fen;
	};

	class Engine
	{
	public:
		Engine() = default;
		Engine(std::string fen, std::string drawPositions);
		board::Board tempBoard;
		Evaluator evaluator;
		FEN::FEN originalFen;
		std::vector<std::string> drawPositions = {};

		struct bestMoveStructure {
			std::string notation;
			double evaluation;
		};

		bestMoveStructure findBestMove();
		std::vector<std::vector<move::Move*>> findAllMovesOfPosition();
		bool isCheck(std::string onColor);
		bool doesMoveMakeCheck(move::Move* move);

		int minDepth = 1;

		int timeStart = 0;

		std::map<long long int, double> hashTable;
		int dumpCounter = 0;

		double calculateMove(
			move::Move* move,
			double alpha,
			double beta,
			int tempDepth,
			int maxDepth
		);
	private:
		std::string originalColor;
		std::vector<std::vector<int>> knightMoves = {
			{-1, 2}, {1, 2}, {2, -1}, {2, 1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}
		};
		std::vector<std::vector<int>> kingMoves = {
			{1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}
		};
	};

	
}