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
	this->tempColor = evaluator.fen.getColor();
}

double engine::Engine::calculateMove(move::Move* move)
{
	std::ofstream outfile;

	outfile.open("dane.txt", std::ios_base::app); // append instead of overwrite
	outfile << move->getMoveICCF() + " D" + std::to_string(tempDepth) + "\n";
	outfile.close();

	double evaluation;
	double minEvaluation = 10000000;

	tempBoard.makeMove(move);

	if (tempDepth < 25) {
		board::Board tb = board::Board(tempBoard);
		tempBoard.colorOnMove = (tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE);

		std::vector<move::Move*> legalMoves = this->findAllLegalMovesOfPosition();
		tempDepth++;

		for (int i = 0; i < legalMoves.size(); i++) {
			evaluation = calculateMove(legalMoves[i]);

			if (evaluation < minEvaluation) {
				minEvaluation = evaluation;
			}
		}

		tempBoard = tb;
		tempDepth--;
		return minEvaluation;
	}
	else {
		return tempBoard.evaluate() * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1 : -1);
	}
	
}

std::string engine::Engine::findBestMove()
{
	tempBoard = board::Board(this->originalFen);

	std::vector<move::Move*> legalMoves = this->findAllLegalMovesOfPosition(); // wszystkie depth 1  b
	tempDepth++;

	double maxEvaluation = -100000000;
	std::string bestMove = "-";

	for (int i = 0; i < legalMoves.size(); i++) {
		double eval = calculateMove(legalMoves[i]);

		if (eval > maxEvaluation) {
			maxEvaluation = eval;
			bestMove = legalMoves[i]->getMoveICCF();
		}
	}

	return bestMove;
}

std::vector<move::Move*> engine::Engine::findAllLegalMovesOfPosition() {
	std::vector<move::Move*> legalMoves = {};
	std::string color = tempBoard.colorOnMove;

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
					/*
					if (tempBoard.canCaptureOnField(field.x - 1, field.y + (piece.isWhite ? 1 : -1))) {
						legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x - 1, field.y + (1 * piece.isWhite ? 1 : -1)));
					}

					if (tempBoard.canCaptureOnField(field.x + 1, field.y + (piece.isWhite ? 1 : -1))) {
						legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x + 1, field.y + (1 * piece.isWhite ? 1 : -1)));
					}
					*/
					
					// to do enpassant and promotion
				}
				/*else if (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK) {
					// w pionie do góry
					for (int y = (field.y + 1); y<=8; y++) {
						if (tempBoard.isFieldEmpty(field.x, y) || tempBoard.isFieldOccupiedByOpponentsPiece(field.x, y)) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
						}
					}

					// w poziomie
				}*/
			}
		}
	}

	return legalMoves;
}