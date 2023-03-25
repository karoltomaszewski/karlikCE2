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
	/*std::ofstream outfile;

	outfile.open("dane.txt", std::ios_base::app); // append instead of overwrite

	std::string tabs = "";
	for (int t = 0; t < (tempDepth-1); t++) {
		tabs += "  ";
	}

	outfile << "\n" + tabs + move->getMoveICCF() + " D" + std::to_string(tempDepth);
	outfile.close();*/
	
	
	double evaluation;
	double minEvaluation = 10000000.0;
	double maxEvaluation = -10000000.0;

	board::Board tb = board::Board(tempBoard);
	tempBoard.makeMove(move);

	if (tempDepth < 4) {
		tempBoard.colorOnMove = (tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE);

		double dummyEvalOfPosition = tempBoard.evaluate(this->originalColor) * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1.0 : -1.0);
		if (tempDepth % 2 != 0) { 
			if (dummyEvalOfPosition > 9900 && dummyEvalOfPosition < 10100) { // zbity król przeciwnika
				tempBoard = tb;
				return Evaluator::CODE_WINS_KING;
			}
		}

		std::vector<move::Move*> legalMoves = this->findAllLegalMovesOfPosition();
		tempDepth++;

		for (int i = 0; i < legalMoves.size(); i++) {
			evaluation = calculateMove(legalMoves[i]);

			if (tempDepth % 2 == 0) { // ruch przeciwnika
				if (evaluation == Evaluator::CODE_WINS_KING) {
					return evaluation;
				}

				if (evaluation < minEvaluation) {
					minEvaluation = evaluation;
				}
			}
			else {
				if (evaluation > maxEvaluation) { // w³asny ruch
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

engine::Engine::bestMoveStructure engine::Engine::findBestMove()
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

	engine::Engine::bestMoveStructure res;

	res.evaluation = maxEvaluation;
	res.notation = bestMove;

	return res;
}

std::vector<move::Move*> engine::Engine::findAllLegalMovesOfPosition() {
	std::vector<move::Move*> legalMoves = {};
	std::vector<move::Move*> possibleMoves = {};
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

						if (!((field.y == 2 && !piece.isWhite) || (field.y == 7 && piece.isWhite))) { //promotion conditions
							if ((field.y == 2 && piece.isWhite) || (field.y == 7 && !piece.isWhite)) { // linia startowa
								if (tempBoard.isFieldEmpty(field.x, field.y + (piece.isWhite ? 2 : -2))) {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 2 : -2)));
								}
							}

							possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1)));
						}
						else {
							// promotion

							// QUEEN
							possibleMoves.push_back(new move::PromotionMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1), 1));

							// KNIGHT
							possibleMoves.push_back(new move::PromotionMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1), 4));
						}
					}

					// captures
					
					if (tempBoard.canCaptureOnField(field.x - 1, field.y + (piece.isWhite ? 1 : -1))) {
						possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x - 1, field.y + (piece.isWhite ? 1 : -1)));
					}

					if (tempBoard.canCaptureOnField(field.x + 1, field.y + (piece.isWhite ? 1 : -1))) {
						possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x + 1, field.y + (piece.isWhite ? 1 : -1)));
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
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(field.x, y)) {
								possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
							}

							break;
						}
					}

					// w pionie w dó³
					for (int y = (field.y - 1); y >= 1; y--) {
						bool isEmpty = tempBoard.isFieldEmpty(field.x, y);

						if (isEmpty) {
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(field.x, y)) {
								possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
							}

							break;
						}
					}

					// w poziomie w lewo
					for (int x = (field.x - 1); x >= 1; x--) {
						bool isEmpty = tempBoard.isFieldEmpty(x, field.y);

						if (isEmpty) {
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, field.y)) {
								possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
							}

							break;
						}
					}

					// w poziomie w prawo
					for (int x = (field.x + 1); x <= 8; x++) {
						bool isEmpty = tempBoard.isFieldEmpty(x, field.y);

						if (isEmpty) {
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, field.y)) {
								possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
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
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
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
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
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
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
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
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
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
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
					}
				}

				// król

				if (piece.pieceName == FEN::FEN::KING_BLACK || piece.pieceName == FEN::FEN::KING_WHITE) {
					for (int km = 0; km < this->kingMoves.size(); km++) {
						int x = field.x + this->kingMoves[km][0];
						int y = field.y + this->kingMoves[km][1];
						if (tempBoard.isFieldEmpty(x, y) || tempBoard.canCaptureOnField(x, y)) {
							possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
						}
					}
				}
			}
		}
	}

	// sprawdzanie czy po takim ruchu nie ma szacha na w³asnym królu

	for (int i = 0; i < possibleMoves.size(); i++) {
		board::Board tb = tempBoard;
		tempBoard.makeMove(possibleMoves[i]);
		tempBoard.colorOnMove = (tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE);

		if (!this->isCheck()) {
			legalMoves.push_back(possibleMoves[i]);
		}

		tempBoard = tb;
	}

	return legalMoves;
}

bool engine::Engine::isCheck() {
	for (int i = 0; i < tempBoard.fields.size(); i++) {
		
		board::Field field = tempBoard.fields[i];
		pieces::Piece piece = field.getPiece();


		/*if (
			(piece.pieceName == FEN::FEN::PAWN_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
			(piece.pieceName == FEN::FEN::PAWN_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
		) {
			bool isEmpty = tempBoard.isFieldEmpty(field.x - 1, field.y);

			if (!isEmpty && tempBoard.isFieldValid(field.x - 1, field.y)) {
				char pieceOnField = tempBoard.getField(field.x - 1, field.y).getPiece().pieceName;

				if (
					(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
					(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
				) {
					return true;
				}
			}
			

			isEmpty = tempBoard.isFieldEmpty(field.x + 1, field.y);

			if (!isEmpty && tempBoard.isFieldValid(field.x + 1, field.y)) {
				char pieceOnField = tempBoard.getField(field.x + 1, field.y).getPiece().pieceName;

				if (
					(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
					(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
					) {
					return true;
				}
			}

			continue;
		}
		*/

		if (
			((piece.pieceName == FEN::FEN::QUEEN_WHITE || piece.pieceName == FEN::FEN::ROOK_WHITE) && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
			((piece.pieceName == FEN::FEN::QUEEN_BLACK || piece.pieceName == FEN::FEN::ROOK_BLACK) && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
		) {
			// w pionie do góry
			for (int y = (field.y + 1); y <= 8; y++) {
				bool isEmpty = tempBoard.isFieldEmpty(field.x, y);

				if (!isEmpty) {
					char pieceOnField = tempBoard.getField(field.x, y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
					) {
						return true;
					}

					break;
				}
			}

			// w pionie w dó³
			for (int y = (field.y - 1); y >= 1; y--) {
				bool isEmpty = tempBoard.isFieldEmpty(field.x, y);

				if (!isEmpty) {
					char pieceOnField = tempBoard.getField(field.x, y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
						) {
						return true;
					}

					break;
				}
			}

			// w poziomie w lewo
			for (int x = (field.x - 1); x >= 1; x--) {
				bool isEmpty = tempBoard.isFieldEmpty(x, field.y);

				if (!isEmpty) {
					char pieceOnField = tempBoard.getField(x, field.y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
						) {
						return true;
					}

					break;
				}
			}

			// w poziomie w prawo
			for (int x = (field.x + 1); x <= 8; x++) {
				bool isEmpty = tempBoard.isFieldEmpty(x, field.y);

				if (!isEmpty) {
					char pieceOnField = tempBoard.getField(x, field.y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
						) {
						return true;
					}

					break;
				}
			}

			if (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK) {
				continue;
			}
		}

		
		// skos - goniec i hetman
		if (
			((piece.pieceName == FEN::FEN::QUEEN_WHITE || piece.pieceName == FEN::FEN::BISHOP_WHITE) && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
			((piece.pieceName == FEN::FEN::QUEEN_BLACK || piece.pieceName == FEN::FEN::BISHOP_BLACK) && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
		) {
			// w lewy górny
			for (int x = (field.x - 1), y = (field.y + 1); (x >= 1 && y <= 8);) {
				bool isEmpty = tempBoard.isFieldEmpty(x, y);

				if (!isEmpty && tempBoard.isFieldValid(x, y)) {
					char pieceOnField = tempBoard.getField(x, y).getPiece().pieceName;
					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
					) {
						return true;
					}

					break;
				}

				x--;
				y++;
			}

			// w prawy górny
			for (int x = (field.x + 1), y = (field.y + 1); (x <= 8 && y <= 8);) {
				bool isEmpty = tempBoard.isFieldEmpty(x, y);

				if (!isEmpty && tempBoard.isFieldValid(x, y)) {
					char pieceOnField = tempBoard.getField(x, y).getPiece().pieceName;
					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
						) {
						return true;
					}

					break;
				}

				x++;
				y++;
			}

			// w lewy dolny
			for (int x = (field.x - 1), y = (field.y - 1); (x >= 1 && y >= 1);) {
				bool isEmpty = tempBoard.isFieldEmpty(x, y);

				if (!isEmpty && tempBoard.isFieldValid(x, y)) {
					char pieceOnField = tempBoard.getField(x, y).getPiece().pieceName;
					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
						) {
						return true;
					}

					break;
				}

				x--;
				y--;
			}

			// w prawy dolny
			for (int x = (field.x + 1), y = (field.y - 1); (x <= 8 && y >= 1);) {
				bool isEmpty = tempBoard.isFieldEmpty(x, y);

				if (!isEmpty && tempBoard.isFieldValid(x, y)) {
					char pieceOnField = tempBoard.getField(x, y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
						) 
					{
						return true;
					}

					break;
				}

				x++;
				y--;
			}

			continue;
		}
		
		if (
			(piece.pieceName == FEN::FEN::KNIGHT_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
			(piece.pieceName == FEN::FEN::KNIGHT_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
		) {
			for (int km = 0; km < this->knightMoves.size(); km++) {
				int x = field.x + this->knightMoves[km][0];
				int y = field.y + this->knightMoves[km][1];

				bool isEmpty = tempBoard.isFieldEmpty(x, y);

				if (!isEmpty && tempBoard.isFieldValid(x, y)) {
					char pieceOnField = tempBoard.getField(x, y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
						) {
						return true;
					}
				}
			}

			continue;
		}

		if (
			(piece.pieceName == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
			(piece.pieceName == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
			) {
			for (int km = 0; km < this->kingMoves.size(); km++) {
				int x = field.x + this->kingMoves[km][0];
				int y = field.y + this->kingMoves[km][1];

				bool isEmpty = tempBoard.isFieldEmpty(x, y);

				if (!isEmpty && tempBoard.isFieldValid(x, y)) {
					char pieceOnField = tempBoard.getField(x, y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE) ||
						(pieceOnField == FEN::FEN::KING_WHITE && tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK)
						) {
						return true;
					}
				}
			}
		}
	}

	return false;
}