#include "Engine.h"
#include "Helpers.h"
#include "Board.h"
#include "FEN.h"
#include<fstream>
#include<vector>

engine::Engine::Engine(std::string fen)
{
	this->evaluator = Evaluator(fen);
	this->originalFen = FEN::FEN(fen);
	this->originalColor = evaluator.fen.getColor();
}

std::string engine::Engine::findBestMove()
{
	tempBoard = board::Board(this->originalFen);

	int depth = 0;
	std::vector<move::Move*> legalMoves = this->findAllLegalMovesOfPosition();
	depth = 1;

	double evaluation;
	double maxEvaluation = -10000000;
	std::string bestMove = "-";
	std::string s = "";
	std::ofstream zapis("dane.txt");
	for (int i = 0; i < legalMoves.size(); i++) {
		tempBoard.makeMove(legalMoves[i]);
		evaluation = tempBoard.evaluate() * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1 : -1);

		s += legalMoves[i]->getMoveICCF() + ": " + std::to_string(evaluation) + " ";

		if (evaluation > maxEvaluation) {
			maxEvaluation = evaluation;
			bestMove = legalMoves[i]->getMoveICCF();
		}

		// revert
		tempBoard = board::Board(this->originalFen);
	}
	zapis << s << std::endl;
	zapis.close();

	return bestMove;
}

std::vector<move::Move*> engine::Engine::findAllLegalMovesOfPosition() {
	std::vector<move::Move*> legalMoves = {};
	std::string color = tempBoard.fen.getColor();

	for (int i = 0; i < tempBoard.fields.size(); i++) {
		board::Field field = tempBoard.fields[i];
		if (!field.isFieldEmpty) {
			pieces::Piece piece = field.getPiece();
			if ((color == FEN::FEN::COLOR_WHITE && piece.isWhite) || (color == FEN::FEN::COLOR_BLACK && !piece.isWhite)) {
				if (piece.pieceName == FEN::FEN::PAWN_WHITE || piece.pieceName == FEN::FEN::PAWN_BLACK) {
					// ruchy do przodu 

					// ruch o 1 pole do przodu
					if (tempBoard.isFieldEmpty(field.x, field.y + (1 * piece.isWhite ? 1 : -1))) {
						legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1)));

						if ((field.y == 2 && piece.isWhite) || (field.y == 7 && !piece.isWhite)) { // linia startowa
							if (tempBoard.isFieldEmpty(field.x, field.y + (2 * piece.isWhite ? 1 : -1))) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 2 : -2)));
							}
						}
						//boardAfterMove =  
					}

					// captures
					
					if (tempBoard.isFieldValid(field.x - 1, field.y + (1 * piece.isWhite ? 1 : -1)) && tempBoard.isFieldOccupiedByOpponentsPiece(field.x - 1, field.y + (1 * piece.isWhite ? 1 : -1))) {
						legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x - 1, field.y + (1 * piece.isWhite ? 1 : -1)));
					}

					if (tempBoard.isFieldValid(field.x + 1, field.y + (1 * piece.isWhite ? 1 : -1)) && tempBoard.isFieldOccupiedByOpponentsPiece(field.x + 1, field.y + (1 * piece.isWhite ? 1 : -1))) {
						legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x + 1, field.y + (1 * piece.isWhite ? 1 : -1)));
					}
					
				}
			}
		}
	}

	return legalMoves;
}