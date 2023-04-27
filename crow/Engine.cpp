#include "Engine.h"
#include "Helpers.h"
#include "Board.h"
#include "FEN.h"
#include<fstream>
#include<vector>
#include<ctime>

engine::Engine::Engine(std::string fen)
{
	this->evaluator = Evaluator(fen);
	this->originalFen = FEN::FEN(fen);
	this->originalColor = evaluator.fen.getColor();
	this->tempColor = evaluator.fen.getColor();

	this->timeStart = std::time(nullptr);
}

double engine::Engine::calculateMove(move::Move* move)
{
	/*
		std::ofstream outfile;

		outfile.open("dane.txt", std::ios_base::app); // append instead of overwrite

		std::string tabs = "";
		for (int t = 0; t < (tempDepth - 1); t++) {
			tabs += "  ";
		}

		outfile << "\n" + tabs + move->getMoveICCF() + " D" + std::to_string(tempDepth);

		outfile.close();
	*/
	
	double evaluation;
	double minEvaluation = 1000000.0;
	double maxEvaluation = -1000000.0;

	board::Board tb = board::Board(tempBoard);

	tempBoard.makeMove(move);
	tempBoard.colorOnMove = (tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE);

	bool isCheck = this->isCheck(tempBoard.colorOnMove);

	if ((tempDepth < 6)) {

		double startingBasicEval = tempBoard.evaluate(this->originalColor) * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1.0 : -1.0);
		std::string thisNodeMode = "candidates";

		std::vector<move::Move*> legalMoves = this->findAllLegalMovesOfPosition(thisNodeMode);

		if (legalMoves.size() == 0) {
			thisNodeMode = "normal";
			legalMoves = this->findAllLegalMovesOfPosition(thisNodeMode);
		}

		tempDepth++;

		if (legalMoves.size() == 0) {
			if (isCheck) {
				tempBoard = tb;
				tempDepth--;

				return (tempDepth % 2 == 0) ? -1000000 : 1000000;
			}
			else {
				tempBoard = tb;
				tempDepth--;

				return 0; // pat
			}
		}

		for (int i = 0; i < legalMoves.size(); i++) {
			if (std::time(nullptr) - 115 > this->timeStart) {
				engine::Engine::bestMoveStructure res;

				tempBoard = tb;
				tempDepth--;
				return -1000000;
			}

			evaluation = calculateMove(legalMoves[i]);

			if (tempDepth % 2 == 0) { // ruch przeciwnika

				if (evaluation == -1000000) {
					tempBoard = tb;
					tempDepth--;
					return -1000000;
				}

				if (evaluation < minEvaluation) {
					minEvaluation = evaluation;
				}
			}
			else {

				if (evaluation == 1000000) {
					tempBoard = tb;
					tempDepth--;
					return 1000000;
				}

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
	tempColor = this->originalColor;
	tempDepth = 0;

	double startingEval = tempBoard.evaluate(this->originalColor) * (this->originalColor == FEN::FEN::COLOR_WHITE ? 1.0 : -1.0);

	std::vector<move::Move*> legalMoves = this->findAllLegalMovesOfPosition("all");

	tempDepth++;

	double maxEvaluation = -100000000;

	std::vector<std::string> bestMoves = {};

	if (legalMoves.size() == 0) {
		if (!this->isCheck(tempBoard.colorOnMove)) {
			maxEvaluation = 0;
		}
	}

	for (int i = 0; i < legalMoves.size(); i++) {


		double eval = calculateMove(legalMoves[i]);

		if (eval == 1000000) {
			maxEvaluation = eval;
			bestMoves = { legalMoves[i]->getMoveICCF() };
			break;
		}

		if (eval > maxEvaluation) {
			if (eval - 0.06 < maxEvaluation) {
				bestMoves.push_back(legalMoves[i]->getMoveICCF());
			}
			else {
				bestMoves = { legalMoves[i]->getMoveICCF() };
			}

			maxEvaluation = eval;
		}
		else if (eval == maxEvaluation) {
			bestMoves.push_back(legalMoves[i]->getMoveICCF());
		}

		if (std::time(nullptr) - 105 > this->timeStart) {
			engine::Engine::bestMoveStructure res;

			res.evaluation = maxEvaluation;
			res.notation = bestMoves[rand() % bestMoves.size()];

			return res;
		}
	}

	engine::Engine::bestMoveStructure res;

	res.evaluation = maxEvaluation;
	res.notation = bestMoves[rand() % bestMoves.size()];

	return res;
}

std::vector<move::Move*> engine::Engine::findAllLegalMovesOfPosition(std::string mode) {
	std::vector<move::Move*> legalMoves = {};

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
				for (int km = 0; km < this->knightMoves.size(); km++) {
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
				for (int km = 0; km < this->kingMoves.size(); km++) {
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
				for (int km = 0; km < this->knightMoves.size(); km++) {
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
				for (int km = 0; km < this->kingMoves.size(); km++) {
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
							legalMoves.push_back(new move::CastleMove(5, 1, 7, 1)); // od razu do legalMoves
							canCastle = true;
						}
					}
				}

				if (tempBoard.canWhiteQueenCastle) {
					if (!tempBoard.getField(4, 1).getPiece().isReal && !tempBoard.getField(3, 1).getPiece().isReal && !tempBoard.getField(2, 1).getPiece().isReal) { // pola miêdzy królem, a wie¿¹ musz¹ byæ puste
						if (fieldsDefendetByOpponentFrequency[59] == 0 && fieldsDefendetByOpponentFrequency[60] == 0) {  // sprawdzanie czy pole przez które musi przejœæ król nie jest szachowane i czy docelowe jest szachowane
							legalMoves.push_back(new move::CastleMove(5, 1, 3, 1));  // od razu do legalMoves
							canCastle = true;
						}
					}
				}
			}
			else {
				if (tempBoard.canBlackKingCastle) {
					if (!tempBoard.getField(6, 8).getPiece().isReal && !tempBoard.getField(7, 8).getPiece().isReal) { // pola miêdzy królem, a wie¿¹ musz¹ byæ puste
						if (fieldsDefendetByOpponentFrequency[5] == 0 && fieldsDefendetByOpponentFrequency[4] == 0) { // sprawdza czy król po ruchu nie bêdzie szachowany
							legalMoves.push_back(new move::CastleMove(5, 8, 7, 8));
							canCastle = true;
						}
					}
				}

				if (tempBoard.canBlackQueenCastle) {
					if (!tempBoard.getField(4, 8).getPiece().isReal && !tempBoard.getField(3, 8).getPiece().isReal && !tempBoard.getField(2, 8).getPiece().isReal) { // pola miêdzy królem, a wie¿¹ musz¹ byæ puste
						if (fieldsDefendetByOpponentFrequency[3] == 0 && fieldsDefendetByOpponentFrequency[4] == 0) { // sprawdza czy król po ruchu nie bêdzie szachowany
							legalMoves.push_back(new move::CastleMove(5, 8, 3, 8));
							canCastle = true;
						}
					}
				}	
			}
		}
	}

	for (int i = 0; i < 64; i++) {
		board::Field field = tempBoard.fields[i];
		/*
		if (tempDepth == 1) {
			std::ofstream outfile;

			outfile.open("dane.txt", std::ios_base::app); // append instead of overwrite

			outfile << std::to_string(field.x) + " " + std::to_string(field.y) + " " + std::to_string(board::Board::calculateIndex(field.x, field.y)) + "\n";

			outfile.close();

		}
		*/


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
							candidatesMoves.push_back(new move::PromotionMove(field.x, field.y, field.x, field.y + (piece.isWhite ? 1 : -1), 1));

							// KNIGHT
							if (piece.isWhite) {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 2, 7) && tempBoard.getField(field.x - 2, 7).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 2, 7) && tempBoard.getField(field.x + 2, 7).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 1, 6) && tempBoard.getField(field.x - 1, 6).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 1, 6) && tempBoard.getField(field.x + 1, 6).getPiece().isReal ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									candidatesMoves.push_back(new move::PromotionMove(field.x, field.y, field.x, 8, 4));
								}
							}
							else {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 2, 2) && tempBoard.getField(field.x - 2, 2).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 2, 2) && tempBoard.getField(field.x + 2, 2).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 1, 3) && tempBoard.getField(field.x - 1, 3).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 1, 3) && tempBoard.getField(field.x + 1, 3).getPiece().isReal ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									candidatesMoves.push_back(new move::PromotionMove(field.x, field.y, field.x, 1, 4));
								}
							}
							
						}
					}

					// captures
					
					if (tempBoard.canCaptureOnField(field.x - 1, field.y + (piece.isWhite ? 1 : -1))) {
						if ((piece.isWhite && field.y == 7) || (!piece.isWhite && field.y == 2)) {
							candidatesMoves.push_back(new move::PromotionMove(field.x, field.y, field.x - 1, field.y + (piece.isWhite ? 1 : -1), 1));

							// KNIGHT
							if (piece.isWhite) {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 3, 7) && tempBoard.getField(field.x - 3, 7).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 1, 7) && tempBoard.getField(field.x + 1, 7).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 2, 6) && tempBoard.getField(field.x - 2, 6).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x, 6) && tempBoard.getField(field.x, 6).getPiece().isReal ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									candidatesMoves.push_back(new move::PromotionMove(field.x, field.y, field.x - 1, 8, 4));
								}
							}
							else {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 3, 2) && tempBoard.getField(field.x - 3, 2).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x + 1, 2) && tempBoard.getField(field.x + 1, 2).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x - 2, 3) && tempBoard.getField(field.x - 2, 3).getPiece().isReal ? 1 : 0;
								attackedOrDefendedPiecesByKnight += tempBoard.isFieldValid(field.x, 3) && tempBoard.getField(field.x, 3).getPiece().isReal ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									candidatesMoves.push_back(new move::PromotionMove(field.x, field.y, field.x - 1, 1, 4));
								}
							}
						}
						else {
							candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x - 1, field.y + (piece.isWhite ? 1 : -1)));
						}
					}

					if (tempBoard.canCaptureOnField(field.x + 1, field.y + (piece.isWhite ? 1 : -1))) {
						if ((piece.isWhite && field.y == 7) || (!piece.isWhite && field.y == 2)) {
							candidatesMoves.push_back(new move::PromotionMove(field.x, field.y, field.x + 1, field.y + (piece.isWhite ? 1 : -1), 1));

							// KNIGHT
							if (piece.isWhite) {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x - 1, 7) && tempBoard.getField(field.x - 1, 7).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x + 3, 7) && tempBoard.getField(field.x + 3, 7).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x, 6) && tempBoard.getField(field.x, 6).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x + 2, 6) && tempBoard.getField(field.x + 2, 6).getPiece().isReal) ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									candidatesMoves.push_back(new move::PromotionMove(field.x, field.y, field.x + 1, 8, 4));
								}
							}
							else {
								int attackedOrDefendedPiecesByKnight = 0;

								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x - 1, 2) && tempBoard.getField(field.x - 1, 2).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x + 3, 2) && tempBoard.getField(field.x + 3, 2).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x, 3) && tempBoard.getField(field.x, 3).getPiece().isReal) ? 1 : 0;
								attackedOrDefendedPiecesByKnight += (tempBoard.isFieldValid(field.x + 2, 3) && tempBoard.getField(field.x + 2, 3).getPiece().isReal) ? 1 : 0;

								if (attackedOrDefendedPiecesByKnight > 1) {
									candidatesMoves.push_back(new move::PromotionMove(field.x, field.y, field.x + 1, 1, 4));
								}
							}
						}
						else {
							candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x + 1, field.y + (piece.isWhite ? 1 : -1)));
						}
					}
					
					// to do enpassant
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
								bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x, y)] >= fieldsAttackedFrequency[board::Board::calculateIndex(field.x, y)]
									&& tempBoard.getField(field.x, y).getPiece().power < power;

								if (isFieldProtected) {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
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
								bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(field.x, y)] >= fieldsAttackedFrequency[board::Board::calculateIndex(field.x, y)]
									&& tempBoard.getField(field.x, y).getPiece().power < power;

								if (isFieldProtected) {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, field.x, y));
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
								bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, field.y)] >= fieldsAttackedFrequency[board::Board::calculateIndex(x, field.y)]
									&& tempBoard.getField(x, field.y).getPiece().power < power;

								if (isFieldProtected) {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
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
								bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, field.y)] >= fieldsAttackedFrequency[board::Board::calculateIndex(x, field.y)]
									&& tempBoard.getField(x, field.y).getPiece().power < power;

								if (isFieldProtected) {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, field.y));
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
								bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)] >= fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]
									&& tempBoard.getField(x, field.y).getPiece().power < power;

								if (isFieldProtected) {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
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
								bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)] >= fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]
									&& tempBoard.getField(x, field.y).getPiece().power < power;

								if (isFieldProtected) {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
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
								bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)] >= fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]
									&& tempBoard.getField(x, y).getPiece().power < power;

								if (isFieldProtected) {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
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
								bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)] >= fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]
									&& tempBoard.getField(x, field.y).getPiece().power < power;

								if (isFieldProtected) {
									possibleMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
								}
								else {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
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

					for (int km = 0; km < this->knightMoves.size(); km++) {
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
									bool isFieldProtected = fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)] >= fieldsAttackedFrequency[board::Board::calculateIndex(x, y)]
										&& tempBoard.getField(x,y).getPiece().power < 3 && fieldsDefendetByOpponentMin[board::Board::calculateIndex(x, y)] != 10000;

									if (isFieldProtected) {
										addTo = "possibleMoves";
									}
									// else addTo is still candidatesMoves
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
							else {
								initiallyRejectedMoves.push_back(move);
							}
						}
					}
				}

				// król

				if (piece.pieceName == FEN::FEN::KING_BLACK || piece.pieceName == FEN::FEN::KING_WHITE) {
					for (int km = 0; km < this->kingMoves.size(); km++) {
						int x = field.x + this->kingMoves[km][0];
						int y = field.y + this->kingMoves[km][1];
						if (tempBoard.isFieldEmpty(x, y) || tempBoard.canCaptureOnField(x, y)) {
							if (fieldsDefendetByOpponentFrequency[board::Board::calculateIndex(x, y)] == 0) { // inaczej wchodzi pod szacha, bez sensu to liczyæ
								if (tempBoard.canCaptureOnField(x, y)) {
									candidatesMoves.push_back(new move::NormalMove(field.x, field.y, x, y));
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


	
	// sprawdzanie czy po takim ruchu nie ma szacha na w³asnym królu

	for (int i = 0; i < candidatesMoves.size(); i++) {
		board::Board tb = tempBoard;
		tempBoard.makeMove(candidatesMoves[i]);

		if (!this->isCheck(tempBoard.colorOnMove)) {
			legalMoves.push_back(candidatesMoves[i]);
		}

		tempBoard = tb;
	}

	if (legalMoves.size() > 0) {
		for (int i = 0; i < possibleMoves.size(); i++) {
			board::Board tb = tempBoard;
			tempBoard.makeMove(possibleMoves[i]);

			if (this->isCheck(tempBoard.colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE)) {
				if (!this->isCheck(tempBoard.colorOnMove)) {
					legalMoves.push_back(possibleMoves[i]);
				}
			}

			tempBoard = tb;
		}
	}
		
	if (mode == "candidates") {
		return legalMoves;
	}

	// sprawdzanie czy po takim ruchu nie ma szacha na w³asnym królu

	for (int i = 0; i < possibleMoves.size(); i++) {
		board::Board tb = tempBoard;
		tempBoard.makeMove(possibleMoves[i]);

		if (!this->isCheck(tempBoard.colorOnMove)) {
			legalMoves.push_back(possibleMoves[i]);
		}

		tempBoard = tb;
	}

	if (legalMoves.size() == 0 || mode == "all") {
		for (int i = 0; i < initiallyRejectedMoves.size(); i++) {
			board::Board tb = tempBoard;
			tempBoard.makeMove(initiallyRejectedMoves[i]);

			if (!this->isCheck(tempBoard.colorOnMove)) {
				legalMoves.push_back(initiallyRejectedMoves[i]);
			}

			tempBoard = tb;
		}
	}

	return legalMoves;
}

bool engine::Engine::isCheck(std::string onColor) {
	for (int i = 0; i < tempBoard.fields.size(); i++) {
		
		board::Field field = tempBoard.fields[i];
		pieces::Piece piece = field.getPiece();

		if (
			(piece.pieceName == FEN::FEN::PAWN_WHITE && onColor == FEN::FEN::COLOR_BLACK) ||
			(piece.pieceName == FEN::FEN::PAWN_BLACK && onColor == FEN::FEN::COLOR_WHITE)
		) {
			bool isEmpty = tempBoard.isFieldEmpty(field.x - 1, field.y + (piece.isWhite ? 1 : -1));

			if (!isEmpty && tempBoard.isFieldValid(field.x - 1, field.y + (piece.isWhite ? 1 : -1))) {
				char pieceOnField = tempBoard.getField(field.x - 1, field.y + (piece.isWhite ? 1 : -1)).getPiece().pieceName;

				if (
					(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
					(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
				) {
					return true;
				}
			}
			

			isEmpty = tempBoard.isFieldEmpty(field.x + 1, field.y + (piece.isWhite ? 1 : -1));

			if (!isEmpty && tempBoard.isFieldValid(field.x + 1, field.y + (piece.isWhite ? 1 : -1))) {
				char pieceOnField = tempBoard.getField(field.x + 1, field.y + (piece.isWhite ? 1 : -1)).getPiece().pieceName;

				if (
					(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
					(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
					) {
					return true;
				}
			}

			continue;
		}

		if (
			((piece.pieceName == FEN::FEN::QUEEN_WHITE || piece.pieceName == FEN::FEN::ROOK_WHITE) && onColor == FEN::FEN::COLOR_BLACK) ||
			((piece.pieceName == FEN::FEN::QUEEN_BLACK || piece.pieceName == FEN::FEN::ROOK_BLACK) && onColor == FEN::FEN::COLOR_WHITE)
		) {
			// w pionie do góry
			for (int y = (field.y + 1); y <= 8; y++) {
				bool isEmpty = tempBoard.isFieldEmpty(field.x, y);

				if (!isEmpty) {
					char pieceOnField = tempBoard.getField(field.x, y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
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
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
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
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
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
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
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
			((piece.pieceName == FEN::FEN::QUEEN_WHITE || piece.pieceName == FEN::FEN::BISHOP_WHITE) && onColor == FEN::FEN::COLOR_BLACK) ||
			((piece.pieceName == FEN::FEN::QUEEN_BLACK || piece.pieceName == FEN::FEN::BISHOP_BLACK) && onColor == FEN::FEN::COLOR_WHITE)
		) {
			// w lewy górny
			for (int x = (field.x - 1), y = (field.y + 1); (x >= 1 && y <= 8);) {
				bool isEmpty = tempBoard.isFieldEmpty(x, y);

				if (!isEmpty && tempBoard.isFieldValid(x, y)) {
					char pieceOnField = tempBoard.getField(x, y).getPiece().pieceName;
					if (
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
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
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
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
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
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
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
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
			(piece.pieceName == FEN::FEN::KNIGHT_WHITE && onColor == FEN::FEN::COLOR_BLACK) ||
			(piece.pieceName == FEN::FEN::KNIGHT_BLACK && onColor == FEN::FEN::COLOR_WHITE)
		) {
			for (int km = 0; km < this->knightMoves.size(); km++) {
				int x = field.x + this->knightMoves[km][0];
				int y = field.y + this->knightMoves[km][1];

				bool isEmpty = tempBoard.isFieldEmpty(x, y);

				if (!isEmpty && tempBoard.isFieldValid(x, y)) {
					char pieceOnField = tempBoard.getField(x, y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
					) {
						return true;
					}
				}
			}

			continue;
		}

		if (
			(piece.pieceName == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_BLACK) ||
			(piece.pieceName == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_WHITE)
			) {
			for (int km = 0; km < this->kingMoves.size(); km++) {
				int x = field.x + this->kingMoves[km][0];
				int y = field.y + this->kingMoves[km][1];

				bool isEmpty = tempBoard.isFieldEmpty(x, y);

				if (!isEmpty && tempBoard.isFieldValid(x, y)) {
					char pieceOnField = tempBoard.getField(x, y).getPiece().pieceName;

					if (
						(pieceOnField == FEN::FEN::KING_BLACK && onColor == FEN::FEN::COLOR_BLACK) ||
						(pieceOnField == FEN::FEN::KING_WHITE && onColor == FEN::FEN::COLOR_WHITE)
					) {
						return true;
					}
				}
			}
		}
	}

	return false;
}