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
	/*
	std::ofstream outfile;

	outfile.open("dane.txt", std::ios_base::app); // append instead of overwrite

	std::string tabs = "";
	for (int t = 0; t < (tempDepth-1); t++) {
		tabs += "  ";
	}

	outfile << "\n" + tabs + move->getMoveICCF() + " D" + std::to_string(tempDepth);
	outfile.close();
	*/
	
	double evaluation;
	double minEvaluation = 10000000.0;
	double maxEvaluation = -10000000.0;

	board::Board tb = board::Board(tempBoard);
	tempBoard.makeMove(move);

	if (tempDepth < 4) {
		tempBoard.colorOnMove = (tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE);

		std::vector<move::Move*> legalMoves = this->findAllLegalMovesOfPosition();
		tempDepth++;

		for (int i = 0; i < legalMoves.size(); i++) {
			evaluation = calculateMove(legalMoves[i]);

			if (tempDepth % 2 == 0) {
				if (evaluation < minEvaluation) {
					minEvaluation = evaluation;
				}
			}
			else {
				if (evaluation > maxEvaluation) {
					maxEvaluation = evaluation;
				}
			}
			
		}

		tempBoard = tb;
		tempDepth--;
		return tempDepth % 2 == 0 ? maxEvaluation : minEvaluation; // linijke wyzej odejmmuje tempDepth!!!
	}
	else {
		//outfile.open("dane.txt", std::ios_base::app); // append instead of overwrite

		double e = tempBoard.evaluate(this->originalColor) * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1.0 : -1.0);

		/*
		std::string tabs = "";
		outfile << " " + std::to_string(e);
		outfile.close();
		*/

		tempBoard = tb;
		return e;
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
							if (tempBoard.isFieldEmpty(field.x, field.y + (piece.isWhite ? 2 : -2))) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 2 : -2)));
							}
						}
						//boardAfterMove =  
					}

					// captures
					
					if (tempBoard.canCaptureOnField(field.x - 1, field.y + (piece.isWhite ? 1 : -1))) {
						legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x - 1, field.y + (piece.isWhite ? 1 : -1)));
					}

					if (tempBoard.canCaptureOnField(field.x + 1, field.y + (piece.isWhite ? 1 : -1))) {
						legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x + 1, field.y + (piece.isWhite ? 1 : -1)));
					}
					
					
					// to do enpassant and promotion
					continue;
				}
				
				// pion i poziom - wie¿a i hetman
				if (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK || piece.pieceName == FEN::FEN::QUEEN_BLACK || piece.pieceName == FEN::FEN::QUEEN_WHITE) {
					// w pionie do góry
					for (int y = (field.y + 1); y<=8; y++) {
						bool isEmpty = tempBoard.isFieldEmpty(field.x, y);

						if (isEmpty) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(field.x, y)) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
							}

							break;
						}
					}

					// w pionie w dó³
					for (int y = (field.y - 1); y >= 1; y--) {
						bool isEmpty = tempBoard.isFieldEmpty(field.x, y);

						if (isEmpty) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(field.x, y)) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
							}

							break;
						}
					}

					// w poziomie w lewo
					for (int x = (field.x - 1); x >= 1; x--) {
						bool isEmpty = tempBoard.isFieldEmpty(x, field.y);

						if (isEmpty) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, field.y)) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
							}

							break;
						}
					}

					// w poziomie w prawo
					for (int x = (field.x + 1); x <= 8; x++) {
						bool isEmpty = tempBoard.isFieldEmpty(x, field.y);

						if (isEmpty) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, field.y)) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
							}

							break;
						}
					}

					if (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK) {
						continue;
					}
				}

				
				// skos - goniec i hetman
				if (piece.pieceName == FEN::FEN::BISHOP_BLACK || piece.pieceName == FEN::FEN::BISHOP_WHITE || piece.pieceName == FEN::FEN::QUEEN_BLACK || piece.pieceName == FEN::FEN::QUEEN_WHITE) {
					// w lewy górny
					for (int x = (field.x - 1), y = (field.y + 1); (x >= 1 && y <= 8);) {
						bool isEmpty = tempBoard.isFieldEmpty(x, y);

						if (isEmpty) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
							}

							break;
						}

						x--;
						y++;
					}

					// w prawy górny
					for (int x = (field.x + 1), y = (field.y + 1); (x <= 8 && y <= 8);) {
						bool isEmpty = tempBoard.isFieldEmpty(x, y);
						if (isEmpty) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
							}

							break;
						}


						x++;
						y++;
					}

					// w lewy dolny
					for (int x = (field.x - 1), y = (field.y - 1); (x >= 1 && y >= 1);) {
						bool isEmpty = tempBoard.isFieldEmpty(x, y);
						if (isEmpty) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
							}

							break;
						}


						x--;
						y--;
					}

					// w prawy dolny
					for (int x = (field.x + 1), y = (field.y - 1); (x <=8 && y >= 1);) {
						bool isEmpty = tempBoard.isFieldEmpty(x, y);
						if (isEmpty) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
							}

							break;
						}


						x++;
						y--;
					}

					continue;
				}

				// skoczek

				if (piece.pieceName == FEN::FEN::KNIGHT_BLACK || piece.pieceName == FEN::FEN::KNIGHT_WHITE) {
					for (int km = 0; km < this->knightMoves.size(); km++) {
						int x = field.x + this->knightMoves[km][0];
						int y = field.y + this->knightMoves[km][1];
						if (tempBoard.isFieldEmpty(x, y) || tempBoard.canCaptureOnField(x, y)) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
					}
				}

				// król

				if (piece.pieceName == FEN::FEN::KING_BLACK || piece.pieceName == FEN::FEN::KING_WHITE) {
					for (int km = 0; km < this->kingMoves.size(); km++) {
						int x = field.x + this->kingMoves[km][0];
						int y = field.y + this->kingMoves[km][1];
						if (tempBoard.isFieldEmpty(x, y) || tempBoard.canCaptureOnField(x, y)) {
							legalMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
					}
				}
			}
		}
	}

	return legalMoves;
}