#include "Engine.h"
#include "Helpers.h"
#include "Board.h"
#include "FEN.h"
#include<fstream>
#include<vector>
#include<ctime>
#include<algorithm>

engine::Engine::Engine(std::string fen)
{
	this->evaluator = Evaluator(fen);
	this->originalFen = FEN::FEN(fen);
	this->originalColor = evaluator.fen.getColor();
	this->timeStart = std::time(nullptr);
}


double engine::Engine::calculateMove(move::Move* move, double alpha, double beta, int tempDepth, int maxDepth)
{
	
	bool isCheck = move->makesCheck; // czy jest szach na przeciwniku

	std::ofstream outfile;

	/*outfile.open("dane.txt", std::ios_base::app); // append instead of overwrite

	std::string tabs = "";
	for (int t = 0; t < (tempDepth - 1); t++) {
		tabs += "  ";
	}

	outfile << "\n" + tabs + move->getMoveICCF() + " D" + std::to_string(tempDepth) + " " + (isCheck ? "CHECK" : "");
	*/
	board::Board tb = board::Board(tempBoard);
	tempBoard.makeMove(move);
	tempBoard.colorOnMove = tempBoard.colorOnMove == FEN::FEN::COLOR_BLACK ? FEN::FEN::COLOR_WHITE : FEN::FEN::COLOR_BLACK;

	if (!isCheck && tempDepth == maxDepth) {
		double e = tempBoard.evaluate(this->originalColor) * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1.0 : -1.0);
		tempBoard = tb;
		return e;
	}

	std::vector<move::Move*> legalMoves = {};

	std::vector<std::vector<move::Move*>> allMoves = this->findAllMovesOfPosition();

	board::Board tb2 = tempBoard;

	int LMR = {};
	int LMR2 = {};

	legalMoves = {};
	int len = allMoves.size();
	for (int level = 0; level < len; level++) {
		if (level == 4) { // ruchy daj¹ce szacha dostaj¹ bonus (d=8), reszta nie (d=6)
			LMR = legalMoves.size();
		}
		else if (level == allMoves.size() - 2) {
			LMR2 = legalMoves.size();
		}

		int len2 = allMoves[level].size();
		for (int i = 0; i < len2; i++) {
			tempBoard.makeMove(allMoves[level][i]);

			if (!this->isCheck(tempBoard.colorOnMove)) {
				legalMoves.push_back(allMoves[level][i]);
			}

			tempBoard = tb2;
		}
	}

	//outfile << " " + std::to_string(legalMoves.size());
	//outfile.close();

	if (legalMoves.size() == 0) {
		tempBoard = tb;

		if (isCheck) {
			int e = 1000000 - (tempDepth - 1);
			if (tempDepth % 2 == 0) {
				e *= -1;
			}

			return e;
		}
		else {
			return 0;
		}
	}

	if (tempDepth == maxDepth) { // isCheck
		double e = tempBoard.evaluate(this->originalColor) * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1.0 : -1.0);
		tempBoard = tb;
		return e;
	}

	if (tempDepth % 2 == 1) {
		// przeciwnik

		double value = INFINITY;
		int len = legalMoves.size();
		for (int i = 0; i < len; i++) {
			maxDepth = std::min(LMR < i ? 7 : (LMR2<i ? 6: 5), maxDepth);
			value = std::min(value, calculateMove(legalMoves[i], alpha, beta, tempDepth + 1, maxDepth));

			if (value == -1000000) {
				tempBoard = tb;
				return -1000000;
			}

			if (value < alpha) {
				break;
			}

			beta = std::min(beta, value);
		}
		tempBoard = tb;
		return value;
	} else {
		double value = -1 * INFINITY;
		int len = legalMoves.size();
		for (int i = 0; i < len; i++) {
			maxDepth = std::min(LMR < i ? 7 : (LMR2 < i ? 6 : 5), maxDepth);

			value = std::max(value, calculateMove(legalMoves[i], alpha, beta, tempDepth + 1, maxDepth));

			if (value == 1000000) {
				tempBoard = tb;
				return 1000000;
			}

			if (value > beta) {
				break;
			}

			alpha = std::max(alpha, value);
		}
		tempBoard = tb;
		return value;
	}	
}

engine::Engine::bestMoveStructure engine::Engine::findBestMove()
{
	tempBoard = board::Board(this->originalFen);
	

	std::vector<std::vector<move::Move*>> allMoves = this->findAllMovesOfPosition();


	double alpha = -1 * INFINITY;
	double beta = INFINITY;
	double value = -1 * INFINITY;

	std::vector<move::Move*> legalMoves = {};

	board::Board tb = tempBoard;
	int LMR = {};
	int LMR2 = {};

	int len = allMoves.size();
	for (int level = 0; level < len; level++) {
		if (level == 4) { // ruchy daj¹ce szacha dostaj¹ bonus (d=8), reszta nie (d=6)
			LMR = legalMoves.size();
		}
		else if (level == allMoves.size() - 2) {
			LMR2 = legalMoves.size();
		}

		int len2 = allMoves[level].size();
		for (int i = 0; i < len2; i++) {
			tempBoard.makeMove(allMoves[level][i]);

			if (!this->isCheck(tempBoard.colorOnMove)) {
				legalMoves.push_back(allMoves[level][i]);
			}

			tempBoard = tb;
		}
	}

	engine::Engine::bestMoveStructure res;
	std::string bestMove;
	double maxEval = -1 * INFINITY;

	if (legalMoves.size() == 0) {
		res.evaluation = 0;
		res.notation = "-";
	}

	len = legalMoves.size();
	for (int j = 0; j < len; j++) { // depth 1


		std::ofstream outfile;

		int maxDepth = (j < LMR) ? 7 : ((j < LMR2) ? 6 : 5);
		double ev = calculateMove(legalMoves[j], alpha, beta, 1, maxDepth);

		//outfile.open("dane.txt", std::ios_base::app); // append instead of overwrite
		//outfile << "\n" + legalMoves[j]->getMoveICCF() + " " + std::to_string(ev);

		value = std::max(value, ev);
		alpha = std::max(alpha, value);

		if (value > maxEval) {
			maxEval = value;
			bestMove = legalMoves[j]->getMoveICCF();

			if (value == 1000000) {
				break;
			}
		}

		tempBoard = tb;

		if (std::time(nullptr) - 30 > this->timeStart) {
			break;
		}

	}

	res.evaluation = maxEval;
	res.notation = bestMove;

	return res;
}

std::vector<std::vector<move::Move*>> engine::Engine::findAllMovesOfPosition() {
	std::vector<std::vector<move::Move*>> legalMovesDividedByLevel = {};
	std::vector<move::Move*> legalMoves = {};

	std::vector<move::Move*> promotionMoves = {};
	std::vector<move::Move*> castlesMoves = {};
	std::vector<move::Move*> advantageCaptureMoves = {};
	std::vector<move::Move*> equalCaptureMoves = {};
	std::vector<move::Move*> disadvantageCaptureMoves = {};
	std::vector<move::Move*> checkMoves = {};
	std::vector<move::Move*> otherMoves = {};

	std::vector<move::Move*> candidatesMoves = {};
	std::vector<move::Move*> possibleMoves = {};
	std::vector<move::Move*> initiallyRejectedMoves = {};


	std::string color = tempBoard.colorOnMove;

	bool canCastle = false;

	std::vector<int> fieldsDefendetByOpponentFrequency = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	std::vector<int> fieldsDefendetByOpponentMin = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	std::vector<int> fieldsAttackedFrequency = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	for (int i = 0; i < 64; i++) {
		board::Field field = tempBoard.fields[i];

		if (field.isFieldEmpty) {
			continue;
		}

		pieces::Piece piece = field.getPiece();

		if (((piece.isWhite && color == FEN::FEN::COLOR_WHITE) || (!piece.isWhite && color == FEN::FEN::COLOR_BLACK))) {
			// my
			if (piece.pieceName == FEN::FEN::PAWN_WHITE || piece.pieceName == FEN::FEN::PAWN_BLACK) {
				if (tempBoard.isFieldValid(field.x - 1, field.y - (piece.isWhite ? 1 : -1))) {
					fieldsAttackedFrequency[board::Board::calculateIndex(field.x - 1, field.y + (piece.isWhite ? 1 : -1))]++;
				}

				if (tempBoard.isFieldValid(field.x + 1, field.y - (piece.isWhite ? 1 : -1))) {
					fieldsAttackedFrequency[board::Board::calculateIndex(field.x + 1, field.y + (piece.isWhite ? 1 : -1))]++;
				}

				continue;
			}


			if (piece.pieceName == FEN::FEN::KNIGHT_WHITE || piece.pieceName == FEN::FEN::KNIGHT_BLACK) {
				for (int km = 0; km < 8; km++) {
					int x = field.x + this->knightMoves[km][0];
					int y = field.y + this->knightMoves[km][1];

					if (tempBoard.isFieldValid(x, y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]++;
					}
				}

				continue;
			}


			if (piece.pieceName == FEN::FEN::BISHOP_WHITE || piece.pieceName == FEN::FEN::BISHOP_BLACK || piece.pieceName == FEN::FEN::QUEEN_WHITE || piece.pieceName == FEN::FEN::QUEEN_BLACK) {
				int power = (piece.pieceName == FEN::FEN::BISHOP_WHITE || piece.pieceName == FEN::FEN::BISHOP_BLACK) ? 3 : 9;

				for (int x = (field.x - 1), y = (field.y + 1); (x >= 1 && y <= 8);) {
					if (tempBoard.isFieldValid(x, y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]++;
					}

					if (!tempBoard.isFieldEmpty(x, y)) {
						break;
					}

					x--;
					y++;
				}

				for (int x = (field.x - 1), y = (field.y - 1); (x >= 1 && y >= 1);) {
					if (tempBoard.isFieldValid(x, y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]++;
					}

					if (!tempBoard.isFieldEmpty(x, y)) {
						break;
					}

					x--;
					y--;
				}

				for (int x = (field.x + 1), y = (field.y - 1); (x <= 8 && y >= 1);) {
					if (tempBoard.isFieldValid(x, y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]++;
					}

					if (!tempBoard.isFieldEmpty(x, y)) {
						break;
					}

					x++;
					y--;
				}

				for (int x = (field.x + 1), y = (field.y + 1); (x <= 8 && y <= 8);) {
					if (tempBoard.isFieldValid(x, y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]++;
					}

					if (!tempBoard.isFieldEmpty(x, y)) {
						break;
					}

					x++;
					y++;
				}


				if (piece.pieceName == FEN::FEN::BISHOP_BLACK || piece.pieceName == FEN::FEN::BISHOP_WHITE) {
					continue;
				}
			}

			if (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK || piece.pieceName == FEN::FEN::QUEEN_WHITE || piece.pieceName == FEN::FEN::QUEEN_BLACK) {
				int power = (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK) ? 5 : 9;

				for (int x = (field.x - 1); x >= 1; x--) {
					if (tempBoard.isFieldValid(x, field.y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(x, field.y)]++;
					}

					if (!tempBoard.isFieldEmpty(x, field.y)) {
						break;
					}
				}

				for (int x = (field.x + 1); x <= 8; x++) {
					if (tempBoard.isFieldValid(x, field.y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(x, field.y)]++;
					}

					if (!tempBoard.isFieldEmpty(x, field.y)) {
						break;
					}
				}

				for (int y = (field.y - 1); y >= 1; y--) {
					if (tempBoard.isFieldValid(field.x, y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(field.x, y)]++;
					}

					if (!tempBoard.isFieldEmpty(field.x, y)) {
						break;
					}
				}

				for (int y = (field.y + 1); y <= 8; y++) {
					if (tempBoard.isFieldValid(field.x, y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(field.x, y)]++;
					}

					if (!tempBoard.isFieldEmpty(field.x, y)) {
						break;
					}

					continue;
				}
			}

			if (piece.pieceName == FEN::FEN::KING_BLACK || piece.pieceName == FEN::FEN::KING_WHITE) {
				for (int km = 0; km < 8; km++) {
					int x = field.x + this->kingMoves[km][0];
					int y = field.y + this->kingMoves[km][1];

					if (tempBoard.isFieldValid(x, y)) {
						fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]++;
					}


				}
			}

		}
		else {
			if (piece.pieceName == FEN::FEN::PAWN_WHITE || piece.pieceName == FEN::FEN::PAWN_BLACK) {
				if (tempBoard.isFieldValid(field.x - 1, field.y - (piece.isWhite ? 1 : -1))) {
					fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x - 1, field.y + (piece.isWhite ? 1 : -1))]++;
					fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x - 1, field.y + (piece.isWhite ? 1 : -1))] = 1;
				}

				if (tempBoard.isFieldValid(field.x + 1, field.y - (piece.isWhite ? 1 : -1))) {
					fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x + 1, field.y + (piece.isWhite ? 1 : -1))]++;
					fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x + 1, field.y + (piece.isWhite ? 1 : -1))] = 1;
				}

				continue;
			}


			if (piece.pieceName == FEN::FEN::KNIGHT_WHITE || piece.pieceName == FEN::FEN::KNIGHT_BLACK) {
				for (int km = 0; km < 8; km++) {
					int x = field.x + this->knightMoves[km][0];
					int y = field.y + this->knightMoves[km][1];

					if (tempBoard.isFieldValid(x, y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] > 3 || fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] = 3;
						}
					}
				}

				continue;
			}


			if (piece.pieceName == FEN::FEN::BISHOP_WHITE || piece.pieceName == FEN::FEN::BISHOP_BLACK || piece.pieceName == FEN::FEN::QUEEN_WHITE || piece.pieceName == FEN::FEN::QUEEN_BLACK) {
				int power = (piece.pieceName == FEN::FEN::BISHOP_WHITE || piece.pieceName == FEN::FEN::BISHOP_BLACK) ? 3 : 9;
				
				for (int x = (field.x - 1), y = (field.y + 1); (x >= 1 && y <= 8);) {
					if (tempBoard.isFieldValid(x, y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] > power || fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] = power;
						}
					}

					if (!tempBoard.isFieldEmpty(x, y)) {
						break;
					}

					x--;
					y++;
				}

				for (int x = (field.x - 1), y = (field.y - 1); (x >= 1 && y >= 1);) {
					if (tempBoard.isFieldValid(x, y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] > power || fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] = power;
						}
					}

					if (!tempBoard.isFieldEmpty(x, y)) {
						break;
					}

					x--;
					y--;
				}

				for (int x = (field.x + 1), y = (field.y - 1); (x <= 8 && y >= 1);) {
					if (tempBoard.isFieldValid(x, y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] > power || fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] = power;
						}
					}

					if (!tempBoard.isFieldEmpty(x, y)) {
						break;
					}

					x++;
					y--;
				}

				for (int x = (field.x + 1), y = (field.y + 1); (x <= 8 && y <= 8);) {
					if (tempBoard.isFieldValid(x, y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] > power || fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] = power;
						}
					}

					if (!tempBoard.isFieldEmpty(x, y)) {
						break;
					}

					x++;
					y++;
				}


				if (piece.pieceName == FEN::FEN::BISHOP_BLACK || piece.pieceName == FEN::FEN::BISHOP_WHITE) {
					continue;
				}
			}
		
			if (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK || piece.pieceName == FEN::FEN::QUEEN_WHITE || piece.pieceName == FEN::FEN::QUEEN_BLACK) {
				int power = (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK) ? 5 : 9;

				for (int x = (field.x - 1); x >= 1; x--) {
					if (tempBoard.isFieldValid(x, field.y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, field.y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] > power || fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] = power;
						}
					}

					if (!tempBoard.isFieldEmpty(x, field.y)) {
						break;
					}
				}

				for (int x = (field.x + 1); x <= 8; x++) {
					if (tempBoard.isFieldValid(x, field.y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, field.y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] > power || fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] = power;
						}
					}

					if (!tempBoard.isFieldEmpty(x, field.y)) {
						break;
					}
				}

				for (int y = (field.y - 1); y >= 1; y--) {
					if (tempBoard.isFieldValid(field.x, y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x, y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] > power || fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] = power;
						}
					}

					if (!tempBoard.isFieldEmpty(field.x, y)) {
						break;
					}
				}

				for (int y = (field.y + 1); y <= 8; y++) {
					if (tempBoard.isFieldValid(field.x, y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x, y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] > power || fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] = power;
						}
					}

					if (!tempBoard.isFieldEmpty(field.x, y)) {
						break;
					}

					continue;
				}
			}
		
			if (piece.pieceName == FEN::FEN::KING_BLACK || piece.pieceName == FEN::FEN::KING_WHITE) {
				for (int km = 0; km < 8; km++) {
					int x = field.x + this->kingMoves[km][0];
					int y = field.y + this->kingMoves[km][1];
					
					if (tempBoard.isFieldValid(x, y)) {
						fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, field.y)]++;

						if (fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] == 0) {
							fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] = 10000;
						}
					}

				}
			}
		}
	}

	// roszady 
	if (
		((tempBoard.canWhiteKingCastle || tempBoard.canWhiteQueenCastle) && color == FEN::FEN::COLOR_WHITE) ||
		((tempBoard.canBlackKingCastle || tempBoard.canBlackQueenCastle) && color == FEN::FEN::COLOR_BLACK)
	) {
		bool isKingChecked;
		if (color == FEN::FEN::COLOR_WHITE) {
			isKingChecked = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(tempBoard.whiteKingX, tempBoard.whiteKingY)] > 0;
		}
		else {
			isKingChecked = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(tempBoard.blackKingX, tempBoard.blackKingY)] > 0;
		}

		if (!isKingChecked) { // król nie jest szachowany
			if (color == FEN::FEN::COLOR_WHITE) { 
				if (tempBoard.canWhiteKingCastle) {
					if (!tempBoard.getField(6, 1).getPiece().isReal && !tempBoard.getField(7, 1).getPiece().isReal) { // pola miêdzy królem, a wie¿¹ musz¹ byæ puste						
						if (fieldsDefendetByOpponentFrequency[61] == 0 && fieldsDefendetByOpponentFrequency[60] == 0) { // sprawdzanie czy pole przez które musi przejœæ król nie jest szachowane i czy docelowe jest szachowane
							castlesMoves.push_back(new move::CastleMove(5, 1, 7, 1)); // od razu do legalMoves
							canCastle = true;
						}
					}
				}

				if (tempBoard.canWhiteQueenCastle) {
					if (!tempBoard.getField(4, 1).getPiece().isReal && !tempBoard.getField(3, 1).getPiece().isReal && !tempBoard.getField(2, 1).getPiece().isReal) { // pola miêdzy królem, a wie¿¹ musz¹ byæ puste
						if (fieldsDefendetByOpponentFrequency[59] == 0 && fieldsDefendetByOpponentFrequency[60] == 0) {  // sprawdzanie czy pole przez które musi przejœæ król nie jest szachowane i czy docelowe jest szachowane
							castlesMoves.push_back(new move::CastleMove(5, 1, 3, 1));  // od razu do legalMoves
							canCastle = true;
						}
					}
				}
			}
			else {
				if (tempBoard.canBlackKingCastle) {
					if (!tempBoard.getField(6, 8).getPiece().isReal && !tempBoard.getField(7, 8).getPiece().isReal) { // pola miêdzy królem, a wie¿¹ musz¹ byæ puste
						if (fieldsDefendetByOpponentFrequency[5] == 0 && fieldsDefendetByOpponentFrequency[4] == 0) { // sprawdza czy król po ruchu nie bêdzie szachowany
							castlesMoves.push_back(new move::CastleMove(5, 8, 7, 8));
							canCastle = true;
						}
					}
				}

				if (tempBoard.canBlackQueenCastle) {
					if (!tempBoard.getField(4, 8).getPiece().isReal && !tempBoard.getField(3, 8).getPiece().isReal && !tempBoard.getField(2, 8).getPiece().isReal) { // pola miêdzy królem, a wie¿¹ musz¹ byæ puste
						if (fieldsDefendetByOpponentFrequency[3] == 0 && fieldsDefendetByOpponentFrequency[4] == 0) { // sprawdza czy król po ruchu nie bêdzie szachowany
							castlesMoves.push_back(new move::CastleMove(5, 8, 3, 8));
							canCastle = true;
						}
					}
				}	
			}
		}
	}

	for (int i = 0; i < 64; i++) {
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

									if (field.x == 4 || field.x == 5) {
										bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x, field.y + (piece.isWhite ? 2 : -2))] >= fieldsAttackedFrequency[board::Board::calculateIndex(field.x, field.y + (piece.isWhite ? 2 : -2))];

										if (isFieldProtected) {
											possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 2 : -2)));
										}
										else {
											candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 2 : -2)));
										}
									}
									else {
										if (
											(tempBoard.isFieldValid(field.x - 1, field.y + (piece.isWhite ? 3 : -3)) && !tempBoard.isFieldEmpty(field.x - 1, field.y + (piece.isWhite ? 3 : -3))) ||
											(tempBoard.isFieldValid(field.x + 1, field.y + (piece.isWhite ? 3 : -3)) && !tempBoard.isFieldEmpty(field.x + 1, field.y + (piece.isWhite ? 3 : -3)))
										) {
											bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x, field.y + (piece.isWhite ? 2 : -2))] > fieldsAttackedFrequency[board::Board::calculateIndex(field.x, field.y + (piece.isWhite ? 2 : -2))];

											if (isFieldProtected) {
												initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 2 : -2)));
											}
											else {
												candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 2 : -2)));
											}
										}
										else {
											possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 2 : -2)));
										}
									}
								} 

								// jak jest na linii startowej to rozwa¿amy ruch o 1 do przodu tylko w przypadku gdy po nim bêdzie atakowaæ / broniæ 
								if (
									(tempBoard.isFieldValid(field.x - 1, field.y + (piece.isWhite ? 2 : -2)) && !tempBoard.isFieldEmpty(field.x - 1, field.y + (piece.isWhite ? 2 : -2))) ||
									(tempBoard.isFieldValid(field.x + 1, field.y + (piece.isWhite ? 2 : -2)) && !tempBoard.isFieldEmpty(field.x + 1, field.y + (piece.isWhite ? 2 : -2)))
								) {
									bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x, field.y + (piece.isWhite ? 1 : -1))] > fieldsAttackedFrequency[board::Board::calculateIndex(field.x, field.y + (piece.isWhite ? 1 : -1))];

									if (isFieldProtected) {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1)));
									}
									else {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1)));
									}
								}
								else {
									initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1)));
								}
							}
							else {
								if (
									(tempBoard.isFieldValid(field.x - 1, field.y + (piece.isWhite ? 2 : -2)) && !tempBoard.isFieldEmpty(field.x - 1, field.y + (piece.isWhite ? 2 : -2))) ||
									(tempBoard.isFieldValid(field.x + 1, field.y + (piece.isWhite ? 2 : -2)) && !tempBoard.isFieldEmpty(field.x + 1, field.y + (piece.isWhite ? 2 : -2)))
								) {
									bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x, field.y + (piece.isWhite ? 1 : -1))] > fieldsAttackedFrequency[board::Board::calculateIndex(field.x, field.y + (piece.isWhite ? 1 : -1))];

									if (isFieldProtected) {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1)));
									}
									else {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1)));
									}
								}
								else {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1)));
								}
							}
						}
						else {
							// promotion

							// QUEEN
							promotionMoves.push_back(new move::PromotionMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1), 1));

							// KNIGHT
							if (piece.isWhite) {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 2, 7) && tempBoard.getField(field.x - 2, 7).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 2, 7) && tempBoard.getField(field.x + 2, 7).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 1, 6) && tempBoard.getField(field.x - 1, 6).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 1, 6) && tempBoard.getField(field.x + 1, 6).getPiece().isReal ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									promotionMoves.push_back(new move::PromotionMove(field.x, field.y, field.x, 8, 4));
								}
							}
							else {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 2, 2) && tempBoard.getField(field.x - 2, 2).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 2, 2) && tempBoard.getField(field.x + 2, 2).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 1, 3) && tempBoard.getField(field.x - 1, 3).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 1, 3) && tempBoard.getField(field.x + 1, 3).getPiece().isReal ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									promotionMoves.push_back(new move::PromotionMove(field.x, field.y, field.x, 1, 4));
								}
							}
							
						}
					}

					// captures

					bool canPawnEnPassant = (tempBoard.canEnPassant && (field.x - 1) == tempBoard.enPassantX && (piece.isWhite ? 6 : 3) == tempBoard.enPassantY && (piece.isWhite ? 5 : 4) == field.y);
					if (
						tempBoard.canCaptureOnField(field.x - 1, field.y + (piece.isWhite ? 1 : -1)) ||
						canPawnEnPassant
					) {

						if ((piece.isWhite && field.y == 7) || (!piece.isWhite && field.y == 2)) {
							promotionMoves.push_back(new move::PromotionMove(field.x, field.y, field.x - 1, field.y + (piece.isWhite ? 1 : -1), 1));

							// KNIGHT
							if (piece.isWhite) {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 3, 7) && tempBoard.getField(field.x - 3, 7).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 1, 7) && tempBoard.getField(field.x + 1, 7).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 2, 6) && tempBoard.getField(field.x - 2, 6).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x, 6) && tempBoard.getField(field.x, 6).getPiece().isReal ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									promotionMoves.push_back(new move::PromotionMove(field.x, field.y, field.x - 1, 8, 4));
								}
							}
							else {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 3, 2) && tempBoard.getField(field.x - 3, 2).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 1, 2) && tempBoard.getField(field.x + 1, 2).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 2, 3) && tempBoard.getField(field.x - 2, 3).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x, 3) && tempBoard.getField(field.x, 3).getPiece().isReal ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									promotionMoves.push_back(new move::PromotionMove(field.x, field.y, field.x - 1, 1, 4));
								}
							}
						}
						else {
							if (canPawnEnPassant) {
								equalCaptureMoves.push_back(new move::EnPassantMove(field.x, field.y, field.x - 1, field.y + (piece.isWhite ? 1 : -1)));
							}
							else {
								if (tempBoard.getField(field.x - 1, field.y + (piece.isWhite ? 1 : -1)).getPiece().power > 1) {
									advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x - 1, field.y + (piece.isWhite ? 1 : -1)));
								}
								else {
									equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x - 1, field.y + (piece.isWhite ? 1 : -1)));
								}
							}
						}
					}

					canPawnEnPassant = (tempBoard.canEnPassant && (field.x + 1) == tempBoard.enPassantX && (piece.isWhite ? 6 : 3) == tempBoard.enPassantY && (piece.isWhite ? 5 : 4) == field.y);
					if (
						tempBoard.canCaptureOnField(field.x + 1, field.y + (piece.isWhite ? 1 : -1)) ||
						canPawnEnPassant
					) {
						if ((piece.isWhite && field.y == 7) || (!piece.isWhite && field.y == 2)) {
							promotionMoves.push_back(new move::PromotionMove(field.x, field.y, field.x + 1, field.y + (piece.isWhite ? 1 : -1), 1));

							// KNIGHT
							if (piece.isWhite) {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x - 1, 7) && tempBoard.getField(field.x - 1, 7).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x + 3, 7) && tempBoard.getField(field.x + 3, 7).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x, 6) && tempBoard.getField(field.x, 6).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x + 2, 6) && tempBoard.getField(field.x + 2, 6).getPiece().isReal) ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									promotionMoves.push_back(new move::PromotionMove(field.x, field.y, field.x + 1, 8, 4));
								}
							}
							else {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x - 1, 2) && tempBoard.getField(field.x - 1, 2).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x + 3, 2) && tempBoard.getField(field.x + 3, 2).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x, 3) && tempBoard.getField(field.x, 3).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x + 2, 3) && tempBoard.getField(field.x + 2, 3).getPiece().isReal) ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									promotionMoves.push_back(new move::PromotionMove(field.x, field.y, field.x + 1, 1, 4));
								}
							}
						}
						else {
							if (canPawnEnPassant) {
								equalCaptureMoves.push_back(new move::EnPassantMove(field.x, field.y, field.x + 1, field.y + (piece.isWhite ? 1 : -1)));
							}
							else {
								if (tempBoard.getField(field.x + 1, field.y + (piece.isWhite ? 1 : -1)).getPiece().power > 1) {
									advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x + 1, field.y + (piece.isWhite ? 1 : -1)));
								}
								else {
									equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x + 1, field.y + (piece.isWhite ? 1 : -1)));
								}
							}
						}
					}
					
					continue;
				}
				
				// pion i poziom - wie¿a i hetman
				if (piece.pieceName == FEN::FEN::ROOK_WHITE || piece.pieceName == FEN::FEN::ROOK_BLACK || piece.pieceName == FEN::FEN::QUEEN_BLACK || piece.pieceName == FEN::FEN::QUEEN_WHITE) {
					int power = (piece.pieceName == FEN::FEN::ROOK_BLACK || piece.pieceName == FEN::FEN::ROOK_WHITE) ? 5 : 9;

					bool isAttackedByWeaker = fieldsDefendetByOpponentMin[i] < power && fieldsDefendetByOpponentMin[i] != 0;

					// w pionie do góry

					for (int y = (field.y + 1); y<=8; y++) {
						bool isEmpty = tempBoard.isFieldEmpty(field.x, y);

						if (isEmpty) {
							bool isFieldProtectedByWeaker = fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] < power && fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] != 0;

							// starting positions
							if (
								(field.y == 1 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_WHITE) ||
								(field.y == 8 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_BLACK) ||
								isAttackedByWeaker
							) {
								if (isFieldProtectedByWeaker) {
									if (isAttackedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
									}
									else {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
									}
								}
								else {
									if (isAttackedByWeaker) {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
									}
									else {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
									}
								}
							}
							else {
								if (isFieldProtectedByWeaker) {
									initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
								else {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
							}
						}
						else {
							if (tempBoard.canCaptureOnField(field.x, y)) {
								if (tempBoard.getField(field.x, y).getPiece().power > power) {
									advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
								else if (tempBoard.getField(field.x, y).getPiece().power == power) {
									equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
								else {
									disadvantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
							}

							break;
						}
					}

					// w pionie w dó³
					for (int y = (field.y - 1); y >= 1; y--) {

						bool isEmpty = tempBoard.isFieldEmpty(field.x, y);

						if (isEmpty) {
							bool isFieldProtectedByWeaker = fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] < power && fieldsDefendetByOpponentMin[board::Board::calculateIndex(field.x, y)] != 0;

							// starting positions
							if (
								(field.y == 1 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_WHITE) ||
								(field.y == 8 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_BLACK) ||
								isAttackedByWeaker
							) {
								if (isFieldProtectedByWeaker) {
									if (isAttackedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
									}
									else {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
									}
								}
								else {
									if (isAttackedByWeaker) {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
									}
									else {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
									}
								}
							}
							else {
								if (isFieldProtectedByWeaker) {
									initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
								else {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
							}
						}
						else {
							if (tempBoard.canCaptureOnField(field.x, y)) {
								if (tempBoard.getField(field.x, y).getPiece().power > power) {
									advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
								else if (tempBoard.getField(field.x, y).getPiece().power == power) {
									equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
								else {
									disadvantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
							}

							break;
						}
					}

					// w poziomie w lewo
					for (int x = (field.x - 1); x >= 1; x--) {
						bool isEmpty = tempBoard.isFieldEmpty(x, field.y);

						if (isEmpty) {
							bool isFieldProtectedByWeaker = fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] < power && fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] != 0;

							// starting positions
							if (
								(field.y == 1 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_WHITE) ||
								(field.y == 8 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_BLACK) ||
								isAttackedByWeaker
							) {
								if (isFieldProtectedByWeaker) {
									if (isAttackedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
									}
									else {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
									}
								}
								else {
									if (isAttackedByWeaker) {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y));
									}
									else {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y));
									}
								}
							}
							else {
								if (isFieldProtectedByWeaker) {
									initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
								}
								else {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
								}
							}
						}
						else {
							if (tempBoard.canCaptureOnField(x, field.y)) {
								if (tempBoard.getField(x, field.y).getPiece().power > power) {
									advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
								}
								else if (tempBoard.getField(x, field.y).getPiece().power == power) {
									equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
								}
								else {
									disadvantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
								}
							}

							break;
						}
					}

					// w poziomie w prawo
					for (int x = (field.x + 1); x <= 8; x++) {
						bool isEmpty = tempBoard.isFieldEmpty(x, field.y);

						if (isEmpty) {
							bool isFieldProtectedByWeaker = fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] < power && fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, field.y)] != 0;

							// starting positions
							if (
								(field.y == 1 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_WHITE) ||
								(field.y == 8 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_BLACK) ||
								isAttackedByWeaker
							) {
								if (isFieldProtectedByWeaker) {
									if (isAttackedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
									}
									else {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
									}
								}
								else {
									if (isAttackedByWeaker) {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y));
									}
									else {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, field.y));
									}
								}
							}
							else {
								if (isFieldProtectedByWeaker) {
									initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
								}
								else {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
								}
							}
						}
						else {
							if (tempBoard.canCaptureOnField(x, field.y)) {
								if (tempBoard.canCaptureOnField(x, field.y)) {
									if (tempBoard.getField(x, field.y).getPiece().power > power) {
										advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
									}
									else if (tempBoard.getField(x, field.y).getPiece().power == power) {
										equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
									}
									else {
										disadvantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
									}
								}
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

					int power = (piece.pieceName == FEN::FEN::BISHOP_BLACK || piece.pieceName == FEN::FEN::BISHOP_WHITE) ? 3 : 9;

					bool isAttackedByWeaker = fieldsDefendetByOpponentMin[i] < power && fieldsDefendetByOpponentMin[i] != 0;

					for (int x = (field.x - 1), y = (field.y + 1); (x >= 1 && y <= 8);) {
						bool isEmpty = tempBoard.isFieldEmpty(x, y);

						if (isEmpty) {
							bool isFieldProtectedByWeaker = fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] < power && fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] != 0;

							// starting positions
							if (
								(field.y == 1 && (field.x == 3 || field.x == 6) && piece.pieceName == FEN::FEN::BISHOP_WHITE) ||
								(field.y == 8 && (field.x == 3 || field.x == 6) && piece.pieceName == FEN::FEN::BISHOP_BLACK) ||
								(field.y == 1 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_WHITE) ||
								(field.y == 8 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_BLACK)
							) {
								if (isFieldProtectedByWeaker) {
									if (isAttackedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}							
							}
							else {
								if (isAttackedByWeaker) {
									if (isFieldProtectedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
								else {
									if (isFieldProtectedByWeaker) {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
							}
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								if (tempBoard.canCaptureOnField(x, y)) {
									if (tempBoard.getField(x, y).getPiece().power > power) {
										advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else if (tempBoard.getField(x, y).getPiece().power == power) {
										equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										disadvantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
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
							bool isFieldProtectedByWeaker = fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] < power && fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] != 0;

							// starting positions
							if (
								(field.y == 1 && (field.x == 3 || field.x == 6) && piece.pieceName == FEN::FEN::BISHOP_WHITE) ||
								(field.y == 8 && (field.x == 3 || field.x == 6) && piece.pieceName == FEN::FEN::BISHOP_BLACK) ||
								(field.y == 1 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_WHITE) ||
								(field.y == 8 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_BLACK)
								) {
								if (isFieldProtectedByWeaker) {
									if (isAttackedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}
							}
							else {
								if (isAttackedByWeaker) {
									if (isFieldProtectedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
								else {
									if (isFieldProtectedByWeaker) {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
							}
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								if (tempBoard.canCaptureOnField(x, y)) {
									if (tempBoard.getField(x, y).getPiece().power > power) {
										advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else if (tempBoard.getField(x, y).getPiece().power == power) {
										equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										disadvantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
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
							bool isFieldProtectedByWeaker = fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] < power && fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] != 0;

							// starting positions
							if (
								(field.y == 1 && (field.x == 3 || field.x == 6) && piece.pieceName == FEN::FEN::BISHOP_WHITE) ||
								(field.y == 8 && (field.x == 3 || field.x == 6) && piece.pieceName == FEN::FEN::BISHOP_BLACK) ||
								(field.y == 1 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_WHITE) ||
								(field.y == 8 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_BLACK)
								) {
								if (isFieldProtectedByWeaker) {
									if (isAttackedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}
							}
							else {
								if (isAttackedByWeaker) {
									if (isFieldProtectedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
								else {
									if (isFieldProtectedByWeaker) {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
							}
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								if (tempBoard.canCaptureOnField(x, y)) {
									if (tempBoard.getField(x, y).getPiece().power > power) {
										advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else if (tempBoard.getField(x, y).getPiece().power == power) {
										equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										disadvantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
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
							bool isFieldProtectedByWeaker = fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] < power && fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] != 0;

							// starting positions
							if (
								(field.y == 1 && (field.x == 3 || field.x == 6) && piece.pieceName == FEN::FEN::BISHOP_WHITE) ||
								(field.y == 8 && (field.x == 3 || field.x == 6) && piece.pieceName == FEN::FEN::BISHOP_BLACK) ||
								(field.y == 1 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_WHITE) ||
								(field.y == 8 && field.x == 4 && piece.pieceName == FEN::FEN::QUEEN_BLACK)
								) {
								if (isFieldProtectedByWeaker) {
									if (isAttackedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}
							}
							else {
								if (isAttackedByWeaker) {
									if (isFieldProtectedByWeaker) {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
								else {
									if (isFieldProtectedByWeaker) {
										initiallyRejectedMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
							}
						}
						else {
							if (tempBoard.canCaptureOnField(x, y)) {
								if (tempBoard.canCaptureOnField(x, y)) {
									if (tempBoard.getField(x, y).getPiece().power > power) {
										advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else if (tempBoard.getField(x, y).getPiece().power == power) {
										equalCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
									else {
										disadvantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
									}
								}
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
					bool isAttackedByWeaker = fieldsDefendetByOpponentMin[i] == 1;

					for (int km = 0; km < 8; km++) {
						int x = field.x + this->knightMoves[km][0];
						int y = field.y + this->knightMoves[km][1];
						if (tempBoard.isFieldEmpty(x, y) || tempBoard.canCaptureOnField(x, y)) {
							move::NormalMove* move = new move::NormalMove(field.x, field.y, x, y);
							std::string addTo = "candidatesMoves";

							if (
								tempBoard.canCaptureOnField(x, y) ||
								(field.y == 1 && (field.x == 2 || field.x == 7) && piece.pieceName == FEN::FEN::KNIGHT_WHITE) ||
								(field.y == 8 && (field.x == 2 || field.x == 7) && piece.pieceName == FEN::FEN::KNIGHT_BLACK) ||
								isAttackedByWeaker
							) {
								if ((tempBoard.canCaptureOnField(x, y))) {
									if (tempBoard.getField(x, y).getPiece().power > 3) {
										advantageCaptureMoves.push_back(move);
									}
									else if (tempBoard.getField(x, y).getPiece().power == 3) {
										equalCaptureMoves.push_back(move);
									}
									else {
										disadvantageCaptureMoves.push_back(move);
									}
								}
								else {
									bool isFieldProtectedByPawn = fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] == 1;

									if (isAttackedByWeaker) {
										if (isFieldProtectedByPawn) {
											addTo = "possibleMoves";
										}
									}

									if (x == 1 || x == 8 || y == 1 || y == 8) {
										addTo = "possibleMoves";
									}
								}
							}
							else {
								if (x == 1 || x == 8 || y == 1 || y == 8) {
									addTo = "initiallyRejectedMoves";
								}
								else {
									addTo = "possibleMoves";
								}
							}

							if (addTo == "candidatesMoves") {
								candidatesMoves.push_back(move);
							}
							else if (addTo == "possibleMoves") {
								possibleMoves.push_back(move);
							}
							else if (addTo == "initiallyRejectedMoves") {
								initiallyRejectedMoves.push_back(move);
							}
						}
					}
				}

				// król

				if (piece.pieceName == FEN::FEN::KING_BLACK || piece.pieceName == FEN::FEN::KING_WHITE) {
					for (int km = 0; km < 8; km++) {
						int x = field.x + this->kingMoves[km][0];
						int y = field.y + this->kingMoves[km][1];
						if (tempBoard.isFieldEmpty(x, y) || tempBoard.canCaptureOnField(x, y)) {
							if (fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)] == 0) { // inaczej wchodzi pod szacha, bez sensu to liczyæ
								if (tempBoard.canCaptureOnField(x, y)) {
									advantageCaptureMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}
								else {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}
							}
						}
					}
				}
			}
		}
	}

	board::Board tb = tempBoard;
	
	std::vector<move::Move*> promotionMovesWithoutCheck = {};
	std::vector<move::Move*> advantageCapturesWithoutCheck = {};
	std::vector<move::Move*> equalCapturesWithoutCheck = {};
	std::vector<move::Move*> disadvantageCapturesWithoutCheck = {};
	std::vector<move::Move*> castleWithoutCheck = {};
	std::vector<move::Move*> initiallyRejectedMovesWithoutCheck = {};

	int len = initiallyRejectedMoves.size();
	for (int i = 0; i < len; i++) {
		tempBoard.makeMove(initiallyRejectedMoves[i]);

		if (this->isCheck(tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE)) {
			initiallyRejectedMoves[i]->setMakesCheck();
			checkMoves.insert(checkMoves.begin(), initiallyRejectedMoves[i]);
		}
		else {
			initiallyRejectedMovesWithoutCheck.push_back(initiallyRejectedMoves[i]);
		}

		tempBoard = tb;
	}

	len = possibleMoves.size();
	for (int i = 0; i < len; i++) {
		tempBoard.makeMove(possibleMoves[i]);

		if (this->isCheck(tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE)) {
			possibleMoves[i]->setMakesCheck();
			checkMoves.insert(checkMoves.begin(), possibleMoves[i]);
		}
		else {
			otherMoves.push_back(possibleMoves[i]);
		}

		tempBoard = tb;
	}

	len = candidatesMoves.size();
	for (int i = 0; i < len; i++) {
		tempBoard.makeMove(candidatesMoves[i]);

		if (this->isCheck(tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE)) {
			candidatesMoves[i]->setMakesCheck();
			checkMoves.insert(checkMoves.begin(), candidatesMoves[i]);
		}
		else {
			otherMoves.push_back(candidatesMoves[i]);
		}

		tempBoard = tb;
	}

	len = castlesMoves.size();
	for (int i = 0; i < len; i++) {
		tempBoard.makeMove(castlesMoves[i]);

		if (this->isCheck(tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE)) {
			castlesMoves[i]->setMakesCheck();
			checkMoves.insert(checkMoves.begin(), castlesMoves[i]);
		}
		else {
			castleWithoutCheck.push_back(castlesMoves[i]);
		}

		tempBoard = tb;
	}

	len = disadvantageCaptureMoves.size();
	for (int i = 0; i < len; i++) {
		tempBoard.makeMove(disadvantageCaptureMoves[i]);

		if (this->isCheck(tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE)) {
			disadvantageCaptureMoves[i]->setMakesCheck();
			checkMoves.insert(checkMoves.begin(), disadvantageCaptureMoves[i]);
		}
		else {
			disadvantageCapturesWithoutCheck.push_back(disadvantageCaptureMoves[i]);
		}

		tempBoard = tb;
	}

	len = equalCaptureMoves.size();
	for (int i = 0; i < len; i++) {
		tempBoard.makeMove(equalCaptureMoves[i]);

		if (this->isCheck(tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE)) {
			equalCaptureMoves[i]->setMakesCheck();
			checkMoves.insert(checkMoves.begin(), equalCaptureMoves[i]);
		}
		else {
			equalCapturesWithoutCheck.push_back(equalCaptureMoves[i]);
		}

		tempBoard = tb;
	}

	len = advantageCaptureMoves.size();
	for (int i = 0; i < len; i++) {
		tempBoard.makeMove(advantageCaptureMoves[i]);

		if (this->isCheck(tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE)) {
			advantageCaptureMoves[i]->setMakesCheck();
			checkMoves.insert(checkMoves.begin(), advantageCaptureMoves[i]);
		}
		else {
			advantageCapturesWithoutCheck.push_back(advantageCaptureMoves[i]);
		}

		tempBoard = tb;
	}

	len = promotionMoves.size();
	for (int i = 0; i < len; i++) {
		tempBoard.makeMove(promotionMoves[i]);

		if (this->isCheck(tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE)) {
			promotionMoves[i]->setMakesCheck();
			checkMoves.insert(checkMoves.begin(), promotionMoves[i]);
		}
		else {
			promotionMovesWithoutCheck.push_back(promotionMoves[i]);
		}

		tempBoard = tb;
	}

	return {
		checkMoves,
		promotionMovesWithoutCheck,
		advantageCapturesWithoutCheck,
		equalCapturesWithoutCheck,
		disadvantageCapturesWithoutCheck,
		castleWithoutCheck,
		otherMoves,
		initiallyRejectedMovesWithoutCheck
	};
}

bool engine::Engine::isCheck(std::string onColor) {
	int kingX, kingY;

	if (onColor == FEN::FEN::COLOR_BLACK) {
		kingX = tempBoard.blackKingX;
		kingY = tempBoard.blackKingY;
	}
	else {
		kingX = tempBoard.whiteKingX;
		kingY = tempBoard.whiteKingY;
	}

	// czy jest szachowany przez pionka
	if (onColor == FEN::FEN::COLOR_WHITE) {

		if (
			(tempBoard.isFieldValid(kingX - 1, kingY + 1) && tempBoard.getField(kingX - 1, kingY + 1).getPiece().pieceName == FEN::FEN::PAWN_BLACK) ||
			(tempBoard.isFieldValid(kingX + 1, kingY + 1) && tempBoard.getField(kingX + 1, kingY + 1).getPiece().pieceName == FEN::FEN::PAWN_BLACK)
		) {
			return true;
		}
	}
	else {
		if (
			(tempBoard.isFieldValid(kingX - 1, kingY - 1) && tempBoard.getField(kingX - 1, kingY - 1).getPiece().pieceName == FEN::FEN::PAWN_WHITE) ||
			(tempBoard.isFieldValid(kingX + 1, kingY - 1) && tempBoard.getField(kingX + 1, kingY - 1).getPiece().pieceName == FEN::FEN::PAWN_WHITE)
		) {
			return true;
		}
	}

	// czy jest szachowany przez hetmana lub wie¿e
	
	// poziom - lewo
	for (int x = (kingX - 1); x > 0; x--) {
		board::Field field = tempBoard.getField(x, kingY);

		if (!field.isFieldEmpty) {
			if (
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::ROOK_BLACK : FEN::FEN::ROOK_WHITE) ||
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::QUEEN_BLACK : FEN::FEN::QUEEN_WHITE)
			) {
				return true;
			}

			break;
		}
	}

	// poziom - prawo
	for (int x = (kingX + 1); x < 9; x++) {
		board::Field field = tempBoard.getField(x, kingY);

		if (!field.isFieldEmpty) {
			if (
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::ROOK_BLACK : FEN::FEN::ROOK_WHITE) ||
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::QUEEN_BLACK : FEN::FEN::QUEEN_WHITE)
			) {
				return true;
			}

			break;
		}
	}

	// pion - góra
	for (int y = (kingY + 1); y < 9; y++) {
		board::Field field = tempBoard.getField(kingX, y);

		if (!field.isFieldEmpty) {
			if (
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::ROOK_BLACK : FEN::FEN::ROOK_WHITE) ||
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::QUEEN_BLACK : FEN::FEN::QUEEN_WHITE)
			) {
				return true;
			}

			break;
		}
	}

	// pion - dó³
	for (int y = (kingY - 1); y > 0; y--) {
		board::Field field = tempBoard.getField(kingX, y);

		if (!field.isFieldEmpty) {
			if (
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::ROOK_BLACK : FEN::FEN::ROOK_WHITE) ||
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::QUEEN_BLACK : FEN::FEN::QUEEN_WHITE)
			) {
				return true;
			}

			break;
		}
	}

	// skos 
	// lewy górny

	for (int x = (kingX - 1), y = (kingY + 1); (x > 0 && y < 9);) {
		board::Field field = tempBoard.getField(x, y);

		if (!field.isFieldEmpty) {
			if (
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::BISHOP_BLACK : FEN::FEN::BISHOP_WHITE) ||
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::QUEEN_BLACK : FEN::FEN::QUEEN_WHITE)
			) {
				return true;
			}

			break;
		}

		x--;
		y++;
	}

	// prawy górny

	for (int x = (kingX + 1), y = (kingY + 1); (x < 9 && y < 9);) {
		board::Field field = tempBoard.getField(x, y);

		if (!field.isFieldEmpty) {
			if (
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::BISHOP_BLACK : FEN::FEN::BISHOP_WHITE) ||
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::QUEEN_BLACK : FEN::FEN::QUEEN_WHITE)
			) {
				return true;
			}

			break;
		}

		x++;
		y++;
	}

	// lewy dolny

	for (int x = (kingX - 1), y = (kingY - 1); (x > 0 && y > 0);) {
		board::Field field = tempBoard.getField(x, y);

		if (!field.isFieldEmpty) {
			if (
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::BISHOP_BLACK : FEN::FEN::BISHOP_WHITE) ||
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::QUEEN_BLACK : FEN::FEN::QUEEN_WHITE)
			) {
				return true;
			}

			break;
		}

		x--;
		y--;
	}

	// prawy dolny

	for (int x = (kingX + 1), y = (kingY - 1); (x < 9 && y > 0);) {
		board::Field field = tempBoard.getField(x, y);

		if (!field.isFieldEmpty) {
			if (
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::BISHOP_BLACK : FEN::FEN::BISHOP_WHITE) ||
				field.getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::QUEEN_BLACK : FEN::FEN::QUEEN_WHITE)
			) {
				return true;
			}

			break;
		}

		x++;
		y--;
	}

	// skoczek

	for (int km = 0; km < 8; km++) {
		int x = kingX + this->knightMoves[km][0];
		int y = kingY + this->knightMoves[km][1];

		if (
			tempBoard.isFieldValid(x, y) &&
			tempBoard.getField(x, y).getPiece().pieceName == (onColor == FEN::FEN::COLOR_WHITE ? FEN::FEN::KNIGHT_BLACK : FEN::FEN::KNIGHT_WHITE)
		) {
			return true;
		}
	}

	// król

	if (
		abs(tempBoard.whiteKingX - tempBoard.blackKingX) <= 1 &&
		abs(tempBoard.whiteKingY - tempBoard.blackKingY) <= 1
	) {
		return true;
	}

	return false;
}

bool engine::Engine::doesMoveMakeCheck(move::Move* move) {
	if (move->getType() == "normal" || move->getType() == "enPassant") {

	}

	return false;
}