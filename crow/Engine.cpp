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

std::string engine::Engine::findBestMove()
{
	tempBoard = board::Board(this->originalFen);

	int depth = 0;
	std::vector<move::Move*> legalMoves = this->findAllLegalMovesOfPosition(); // wszystkie depth 1  b
	depth = 1;

	double evaluation;
	double maxEvaluation = -10000000;
	std::string bestMove = "-";
	std::string s = "";
	std::ofstream zapis("dane.txt");
	for (int i = 0; i < legalMoves.size(); i++) {
		tempBoard.makeMove(legalMoves[i]); //w
		board::Board tb = board::Board(tempBoard); //w
		tempBoard.colorOnMove = (tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE);

		std::vector<move::Move*> lMoves = this->findAllLegalMovesOfPosition();
		depth = 2;

		s += legalMoves[i]->getMoveICCF() + "(" + std::to_string(tempBoard.evaluate() * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1 : -1)) +"): \n";

		for (int j = 0; j < lMoves.size(); j++) {
		
			tempBoard.makeMove(lMoves[j]); //b
			evaluation = tempBoard.evaluate() * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1 : -1);
			
			s += "\t" + lMoves[j]->getMoveICCF() + " " + std::to_string(evaluation) + " \n";


			if (evaluation > maxEvaluation) {
				maxEvaluation = evaluation;
				bestMove = legalMoves[i]->getMoveICCF();
			}

			tempBoard = tb;
		}
		// revert
		tempBoard = board::Board(this->originalFen);
	}

	zapis << s;
	zapis.close();

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
					
					if (tempBoard.canCaptureOnField(field.x - 1, field.y + (piece.isWhite ? 1 : -1))) {
						legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x - 1, field.y + (1 * piece.isWhite ? 1 : -1)));
					}

					if (tempBoard.canCaptureOnField(field.x + 1, field.y + (piece.isWhite ? 1 : -1))) {
						legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x + 1, field.y + (1 * piece.isWhite ? 1 : -1)));
					}
					
					// to do enpassant and promotion
				}
				else if (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK) {
					// w pionie do góry
					for (int y = (field.y + 1); y<=8; y++) {
						if (tempBoard.isFieldEmpty(field.x, y) || tempBoard.isFieldOccupiedByOpponentsPiece(field.x, y)) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
						}
					}

					// w poziomie
				}
			}
		}
	}

	return legalMoves;
}