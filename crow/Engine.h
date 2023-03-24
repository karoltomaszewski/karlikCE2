#pragma once

#include <string>
#include "FEN.h"
#include "Move.h"
#include <vector>
#include "Board.h"

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
		FEN::FEN fen;
	};

	class History
	{
	public:
		std::vector<move::Move*> moves;
	};

	class Engine
	{
	public:
		Engine() = default;
		Engine(std::string fen);
		board::Board tempBoard;
		std::string tempColor;
		Evaluator evaluator;
		FEN::FEN originalFen;
		std::string findBestMove();
		std::vector<move::Move*> findAllLegalMovesOfPosition();
		board::Board history;
		int minDepth = 1;
		int tempDepth = 0;

		double getMin();
		double calculateMove(move::Move* move);
	private:
		std::string originalColor;
		std::vector<std::vector<int>> knightMoves = {
			{-1, 2}, {1, 2}, {2, -1}, {2, 1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}
		};
	};

	
}