#include "Engine.h"
#include "Helpers.h"

engine::Evaluator::Evaluator(std::string fen)
{
	this->fen = FEN::FEN(fen);
}

const double engine::Evaluator::PAWN_BASIC_VALUE = 1;
const double engine::Evaluator::KNIGH_BASIC_VALUE = 3;
const double engine::Evaluator::BISHOP_BASIC_VALUE = 3.15;
const double engine::Evaluator::ROOK_BASIC_VALUE = 15;
const double engine::Evaluator::QUEEN_BASIC_VALUE = 9;

double engine::Evaluator::getBasicEvaluation()
{
	int whiteEvaluation = helpers::String::countOccurances(FEN::FEN::PAWN_WHITE, this->fen.getPosition()) * this->PAWN_BASIC_VALUE
		+ helpers::String::countOccurances(FEN::FEN::KNIGHT_WHITE, this->fen.getPosition()) * this->KNIGH_BASIC_VALUE
		+ helpers::String::countOccurances(FEN::FEN::BISHOP_WHITE, this->fen.getPosition()) * this->BISHOP_BASIC_VALUE
		+ helpers::String::countOccurances(FEN::FEN::ROOK_WHITE, this->fen.getPosition()) * this->ROOK_BASIC_VALUE
		+ helpers::String::countOccurances(FEN::FEN::QUEEN_WHITE, this->fen.getPosition()) * this->QUEEN_BASIC_VALUE
		- helpers::String::countOccurances(FEN::FEN::PAWN_BLACK, this->fen.getPosition()) * this->PAWN_BASIC_VALUE
		- helpers::String::countOccurances(FEN::FEN::KNIGHT_BLACK, this->fen.getPosition()) * this->KNIGH_BASIC_VALUE
		- helpers::String::countOccurances(FEN::FEN::BISHOP_BLACK, this->fen.getPosition()) * this->BISHOP_BASIC_VALUE
		- helpers::String::countOccurances(FEN::FEN::ROOK_BLACK, this->fen.getPosition()) * this->ROOK_BASIC_VALUE
		- helpers::String::countOccurances(FEN::FEN::QUEEN_BLACK, this->fen.getPosition()) * this->QUEEN_BASIC_VALUE;

	return whiteEvaluation * (fen.getColor() == FEN::FEN::COLOR_WHITE ? 1 : -1);
}