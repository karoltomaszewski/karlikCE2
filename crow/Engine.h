#pragma once

#include <string>
#include "FEN.h"
#include "Move.h"

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

	class Engine
	{
	public:
		Engine() = default;
		Engine(std::string fen);
		board::Board tempBoard;
		Evaluator evaluator;
		FEN::FEN originalFen;
		std::string findBestMove();
		std::vector<move::Move*> findAllLegalMovesOfPosition();
	private:
		std::string originalColor;
	};
}