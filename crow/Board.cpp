#include "Board.h"
#include "Helpers.h"
#include "Piece.h"
#include "Move.h"
#include<fstream>
#include<string>
#include "Engine.h"

board::Board::Board(FEN::FEN fen) {
	this->generateFields(fen);
	this->colorOnMove = fen.getColor();

	this->canWhiteKingCastle = fen.getCastleInfo().find("K") != std::string::npos;
	this->canWhiteQueenCastle = fen.getCastleInfo().find("Q") != std::string::npos;
	this->canBlackKingCastle = fen.getCastleInfo().find("k") != std::string::npos;
	this->canBlackQueenCastle = fen.getCastleInfo().find("q") != std::string::npos;

	std::string enPassantInfo = fen.getEnPassantInfo();
	
	std::string files = "abcdefgh";

	if (enPassantInfo != "-") {

		this->canEnPassant = true;
		this->enPassantX = files.find(enPassantInfo[0], 0) + 1;
		this->enPassantY = enPassantInfo[1] - '0';

	}

	for (int i = 0; i < 64; i++) {
		if (this->fields[i].getPiece().pieceName == FEN::FEN::KING_WHITE) {
			this->whiteKingX = this->fields[i].x;
			this->whiteKingY = this->fields[i].y;
		} else if (this->fields[i].getPiece().pieceName == FEN::FEN::KING_BLACK) {
			this->blackKingX = this->fields[i].x;
			this->blackKingY = this->fields[i].y;
		}
	}
}

void board::Board::generateFields(FEN::FEN fen) {
	std::string currentPosition = fen.getPosition();

	int row = 8;
	int columnNum = 1;

	int len = currentPosition.length();
	for (int i = 0; i < len; i++) {
		if (currentPosition[i] == '/') {
			row--;
			columnNum = 1;
		}
		else if (helpers::Char::isNumeric(currentPosition[i])) {
			int len2 = helpers::Char::castToRealInt(currentPosition[i]);
			for (int j = 0; j < len2; j++) {
				board::Field newField = board::Field(columnNum, row);
				newField.setPiece(pieces::PieceFactory::create('-'));
				this->fields.push_back(newField);
				columnNum++;
			}
		}
		else {
			board::Field newField = board::Field(columnNum, row);
			newField.setPiece(pieces::PieceFactory::create(currentPosition[i]));

			this->fields.push_back(newField);
			columnNum++;
		}
	}
}

bool board::Board::isFieldValid(int x, int y) {
	return !(x < 1 || x > 8 || y < 1 || y > 8);
}

board::Field board::Board::getField(int x, int y) {
	return this->fields[(8 - y) * 8 + (x - 1)];
}

bool board::Board::isFieldEmpty(int x, int y) {
	if (!this->isFieldValid(x, y)) {
		return false;
	}

	return this->getField(x, y).isFieldEmpty;
}

bool board::Board::isFieldOccupiedByOpponentsPiece(int x, int y) {
	if (this->isFieldEmpty(x, y)) {
		return false;
	}

	pieces::Piece piece = this->fields[(8 - y) * 8 + (x - 1)].getPiece();

	return piece.isReal && (this->colorOnMove == FEN::FEN::COLOR_WHITE && !piece.isWhite) || (this->colorOnMove == FEN::FEN::COLOR_BLACK && piece.isWhite);
}

bool board::Board::canCaptureOnField(int x, int y) {
	if (!this->isFieldValid(x, y)) {
		return false;
	}

	if (this->isFieldEmpty(x, y)) {
		return false;
	}

	return this->isFieldOccupiedByOpponentsPiece(x, y);
}

int board::Board::calculateIndex(int x, int y) {
	return (8 - y) * 8 + (x - 1);
}

void board::Board::makeMove(move::Move* move) {
	pieces::Piece movedPiece = this->fields[(8 - move->yFrom) * 8 + (move->xFrom - 1)].getPiece();

	this->canEnPassant = false;

	if (move->getType() == "normal") {
		if (movedPiece.pieceName == FEN::FEN::KING_WHITE) {
			this->canWhiteKingCastle = false;
			this->canWhiteQueenCastle = false;

			this->whiteKingX = move->xTo;
			this->whiteKingY = move->yTo;
		}
		else if (movedPiece.pieceName == FEN::FEN::KING_BLACK) {
			this->canBlackKingCastle = false;
			this->canBlackQueenCastle = false;

			this->blackKingX = move->xTo;
			this->blackKingY = move->yTo;
		}
		else if (movedPiece.pieceName == FEN::FEN::ROOK_WHITE) {
			if (move->yFrom == 1 && move->xFrom == 1) {
				this->canWhiteQueenCastle = false;
			}
			else if (move->yFrom == 1 && move->xFrom == 8) {
				this->canWhiteKingCastle = false;
			} 
			else if (move->yFrom == 8 && move->xFrom == 1) {
				this->canBlackQueenCastle = false;
			}
			else if (move->yFrom == 8 && move->xFrom == 8) {
				this->canBlackKingCastle = false;
			}
		}


		if (abs(move->yTo - move->yFrom) == 2) {
			if (this->fields[(8 - move->yFrom) * 8 + (move->xFrom - 1)].getPiece().power == 1) {
				this->canEnPassant = true;
				this->enPassantX = move->xFrom;
				this->enPassantY = (move->yFrom + move->yTo) / 2;
			}
		}

		this->fields[(8 - move->yTo) * 8 + (move->xTo - 1)].setPiece(movedPiece);
this->fields[(8 - move->yFrom) * 8 + (move->xFrom - 1)].setPiece(pieces::NoPiece('-'));
	}
	else if (move->getType() == "promotion") {
	this->fields[(8 - move->yTo) * 8 + (move->xTo - 1)].setPiece(pieces::PieceFactory::create(move->promotionCode, this->colorOnMove));
	this->fields[(8 - move->yFrom) * 8 + (move->xFrom - 1)].setPiece(pieces::NoPiece('-'));
	}
	else if (move->getType() == "castle") {
	if (colorOnMove == FEN::FEN::COLOR_WHITE) {
		this->canWhiteKingCastle = false;
		this->canWhiteQueenCastle = false;

		this->whiteKingX = move->xTo;
		this->whiteKingY = move->yTo;
	}
	else {
		this->canBlackKingCastle = false;
		this->canBlackQueenCastle = false;

		this->blackKingX = move->xTo;
		this->blackKingY = move->yTo;
	}

	// ustawienie króla w dobrym miejscu
	this->fields[(8 - move->yTo) * 8 + (move->xTo - 1)].setPiece(movedPiece);

	// wyczyszczenie pola na którym sta³ król
	this->fields[(8 - move->yFrom) * 8 + (move->xFrom - 1)].setPiece(pieces::NoPiece('-'));

	// ustawienie wie¿y w dobrym miejscu
	this->fields[(8 - move->yTo) * 8 + (((move->xTo + move->xFrom) / 2) - 1)].setPiece(pieces::Rook(colorOnMove == FEN::FEN::COLOR_WHITE ? 'R' : 'r'));

	// wyczyszczenie pola na którym sta³a wie¿a
	if (move->xTo == 3) {
		this->fields[(8 - move->yFrom) * 8].setPiece(pieces::NoPiece('-'));
	}
	else {
		this->fields[(8 - move->yFrom) * 8 + 7].setPiece(pieces::NoPiece('-'));
	}
	}
	else if (move->getType() == "enPassant") {
	this->fields[(8 - move->yTo) * 8 + (move->xTo - 1)].setPiece(movedPiece);
	this->fields[(8 - move->yFrom) * 8 + (move->xFrom - 1)].setPiece(pieces::NoPiece('-'));

	// bia³y bije w przelocie 
	if (move->yTo == 6) {
		this->fields[24 + (move->xTo - 1)].setPiece(pieces::NoPiece('-'));
	}
	else {
		// czarny bije w przelocie
		this->fields[32 + (move->xTo - 1)].setPiece(pieces::NoPiece('-'));
	}
	}
}

double board::Board::evaluate() {
	double evaluation = 0.0;
	double earlyEvaluation = 0.0;
	double egEvaluation = 0.0;
	int phase = 0;
	int x = 0;
	int y = 8;

	bool whiteQueenOnStartingSquare = false;
	bool blackQueenOnStartingSquare = false;

	double pawnTable[64] = {
		0,   0,   0,   0,   0,   0,  0,   0,
		0.98, 1.34,  0.61,  0.95,  0.68, 1.26, 0.34, -0.11,
		-0.06,   0.07,  0.26,  0.31,  0.65,  0.56, 0.25, -0.2,
	-0.14,  0.13,   0.06,  0.21,  0.33,  0.12, 0.17, -0.23,
	-0.27,  -0.02,  -0.05,  0.22,  0.27,   0.06, 0.1, -0.25,
	-0.26,  -0.04,  -0.04, -0.1,   0,   0.03, 0.33, -0.12,
	-0.35,  -0.01, -0.20, -0.43, -0.35,  0.24, 0.38, -0.22,
		0,   0,   0,   0,   0,   0,  0,   0,
	};

	double egPawnTable[64] = {
		0,   0,   0,   0,   0,   0,   0,   0,
		1.78, 1.73, 1.58, 1.34, 1.47, 1.32, 1.65, 1.87,
		0.94, 1,  0.85,  0.67,  0.56,  0.53,  0.82,  0.84,
		0.32,  0.24,  0.13,  0.05,  -0.02,   0.04,  0.17,  0.17,
		0.13,   0.09,  -0.03,  -0.07,  -0.07,  -0.08,   0.03,  -0.01,
		0.04,   0.07,  -0.06,   0.01,   0,  -0.05,  -0.01,  -0.08,
		0.13,   0.08,   0.08,  0.1,  0.13,   0,   0.02,  -0.07,
		0,   0,   0,   0,   0,   0,   0,   0,
	};

	double bishopTable[64] = {
		-0.29, 0.04, -0.82, -0.37, -0.25, -0.42,   0.07,  -0.08,
		-0.26,  0.16, -0.18, -0.13,  0.3,  0.59,  0.18, -0.47,
		-0.16,  0.37,  0.43,  0.4,  0.35,  0.5,  0.37,  -0.02,
		-0.04,   0.05,  0.19,  0.5,  0.37,  0.37,   0.07,  -0.02,
		-0.06,  0.13,  0.13,  0.26,  0.34,  0.12,  0.10,   0.04,
		0,  0.15,  0.15,  -0.15,  -0.14,  0.27,  0.18,  0.10,
		0.04,  0.15,  0.16,   0,   0.07,  0.21,  0.33,   0.01,
		-0.33,  -0.03, -0.14, -0.21, -0.13, -0.12, -0.39, -0.21,
	};

	double egBishopTable[64] = {
		-0.14, -0.21, -0.11, -0.08, -0.07, -0.09, -0.17, -0.24,
		-0.08, -0.04, 0.07, -0.12, -0.03, -0.13, -0.04, -0.14,
		0.02, -0.08, 0, -0.01, -0.02, 0.06, 0, 0.04,
		-0.03, 0.09, 0.12, 0.09, 0.14, 0.1, 0.03, 0.02,
		-0.06, 0.03, 0.13, 0.19, 0.07, 0.1, -0.03, -0.09,
		-0.12, -0.03, 0.08, 0.1, 0.13, 0.03, -0.07, -0.15,
		-0.14, -0.18, -0.07, -0.01, 0.04, -0.09, -0.15, -0.27,
		-0.23, -0.09, -0.23, -0.05, -0.09, -0.16, -0.05, -0.17,
	};

	double rookTable[64] = {
		0.32,  0.42,  0.32,  0.51, 0.63,  0.09,  0.31,  0.43,
		 0.27,  0.32,  0.58,  0.62, 0.80, 0.67,  0.26, 0.44,
		 -0.05,  0.19,  0.26,  0.36, 0.17, 0.45,  0.61,  0.16,
		-0.24, -0.11,   0.07,  0.26, 0.24, 0.35,  -0.08, -0.2,
		-0.36, -0.26, -0.12,  -0.01,  0.09, -0.07,   0.06, -0.23,
		-0.45, -0.25, -0.16, -0.17,  0.03,  0,  -0.05, -0.33,
		-0.44, -0.16, -0.20,  -0.09, -0.01, 0.11,  -0.06, -0.71,
		-0.19, -0.13,   0.01,  0.17, 0.16,  0.07, -0.37, -0.26,
	};

	double egRookTable[64] = {
		0.13, 0.1, 0.18, 0.15, 0.12,  0.12,   0.08,   0.05,
		0.11, 0.13, 0.13, 0.11, -0.03,   0.03,   0.08,   0.03,
			0.07, 0.07,  0.07,  0.05,  0.04,  -0.03,  -0.05,  -0.03,
			0.04,  0.03, 0.13,  0.01,  0.02,   0.01,  -0.01,   0.02,
			0.03,  0.05,  0.08,  0.04, -0.05,  -0.06,  -0.08, -0.11,
		-0.04,  0, -0.05, -0.01, -0.07, -0.12,  -0.08, -0.16,
		-0.06, -0.06,  0,  0.02, -0.09,  -0.09, -0.11,  -0.03,
		-0.09,  0.02,  0.03, -0.01, -0.05, -0.13,   0.04, -0.2,
	};

	double kingTable[64] = {
		-0.65,  0.23,  0.16, -0.15, -0.56, -0.34,   0.02,  0.13,
		 0.29,  -0.01, -0.2,  -0.07,  -0.08,  -0.04, -0.38, -0.29,
		 -0.09,  0.24,   0.02, -0.16, -0.2,   0.06,  0.22, -0.22,
		-0.17, -0.2, -0.12, -0.27, -0.3, -0.25, -0.14, -0.36,
		-0.49,  -0.01, -0.27, -0.39, -0.46, -0.44, -0.33, -0.51,
		-0.14, -0.14, -0.22, -0.46, -0.44, -0.3, -0.15, -0.27,
		  0.01,   0.07,  -0.08, -0.64, -0.43, -0.16,   0.09,   0.08,
		-0.15,  0.36,  0.12, -0.54,  0.08, -0.28,  0.24,  0.14,
	};

	double egKingTable[64] = {
		-0.74, -0.35, -0.18, -0.18, -0.11,  0.15,   0.04, -0.17,
		-0.12,  0.17,  0.14,  0.17,  0.17,  0.38,  0.23, 0.11,
			0.1,  0.17,  0.23,  0.15,  0.20,  0.45,  0.44,  0.13,
			-0.08,  0.22,  0.24,  0.27,  0.26,  0.33,  0.26,  0.03,
		-0.18,  -0.04,  0.21,  0.24,  0.27,  0.23,   0.09, -0.11,
		-0.19,  -0.03,  0.11,  0.21,  0.23,  0.16,   0.07,  -0.09,
		-0.27, -0.11,   0.04,  0.13,  0.14,   0.04,  -0.05, -0.17,
		-0.53, -0.34, -0.21, -0.11, -0.28, -0.14, -0.24, -0.43
	};

	double knightTable[64] = {
		-1.67, -0.89, -0.34, -0.49,  0.61, -0.97, -0.15, -1.07,
		 -0.73, -0.41,  0.72,  0.36,  0.23,  0.62,   0.07,  -0.17,
		 -0.47,  0.60,  0.37,  0.65,  0.84, 1.29,  0.73,   0.44,
		  -0.09,  -0.17,  0.19,  -0.23,  0.37,  0.69,  0.18,   0.22,
		 -0.13,   0.04,  0.16,  0.23,  -0.28,  0.19,  0.21,   -0.08,
		 -0.23,  -0.09,  0.22,  0.10,  0.19,  0.37,  0.25,  -0.16,
		 -0.29, -0.53, -0.12,  -0.03,  -0.01,  0.18, -0.14,  -0.19,
		-1.05, -0.11, -0.58, -0.33, -0.17, -0.28, -0.29,  -0.23,
	};

	double egKnightTable[64] = {
		-0.58, -0.38, -0.13, -0.28, -0.31, -0.27, -0.63, -0.99,
		-0.25,  -0.08, -0.25,  -0.02,  -0.09, -0.25, -0.24, -0.52,
		-0.24, -0.2,  0.1,   0.09,  -0.01,  -0.09, -0.19, -0.41,
		-0.17,   0.03,  0.22,  0.22,  0.22,  0.11,   0.08, -0.18,
		-0.18,  -0.06,  0.16,  0.25,  0.16,  0.17,   0.04, -0.18,
		-0.23,  -0.03,  -0.01,  0.15,  0.1,  -0.03, -0.2, -0.22,
		-0.42, -0.2, -0.1,  -0.05,  -0.02, -0.2, -0.23, -0.44,
		-0.29, -0.51, -0.23, -0.15, -0.22, -0.18, -0.5, -0.64,
	};

	double queenTable[64] = {
		-0.28, 0, 0.29, 0.12, 0.59, 0.44, 0.43, 0.45,
		-0.24, -0.39, -0.05, 0.01, -0.16, 0.57, 0.28, 0.54,
		-0.13, -0.17, 0.07, 0.08, 0.29, 0.56, 0.47, 0.57,
		-0.27, -0.27, -0.16, -0.16, -0.01, 0.17, -0.02, -0.11,
		-0.09, -0.26, -0.09, -0.1, -0.02, -0.04, 0.03, -0.03,
		-0.14, 0.02, -0.11, -0.02, -0.05, 0.02, 0.14, 0.05,
		-0.35, -0.08, 0.11, 0.02, 0.08, 0.15, -0.03, 0.01,
		-0.01, -0.18, -0.09, 0.35, -0.15, -0.25, -0.31, -0.5,
	};

	double egQueenTable[64] = {
		-0.09,  0.22, 0.22,  0.27,  0.27,  0.19,  0.1,  0.2,
		-0.17,  0.2,  0.32,  0.41,  0.58,  0.25,  0.3,   0,
		-0.2,   0.06,   0.09,  0.49,  0.47,  0.35,  0.19,   0.09,
		  0.03,  0.22,  0.24,  0.45,  0.57,  0.4,  0.57,  0.36,
		-0.18,  0.28,  0.19,  0.47,  0.31,  0.34,  0.39,  0.23,
		-0.16, -0.27,  0.15,   0.06,   0.09,  0.17,  0.1,   0.05,
		-0.22, -0.23, -0.30, -0.16, -0.16, -0.23, -0.36, -0.32,
		-0.33, -0.28, -0.22, -0.43,  -0.05, -0.32, -0.2, -0.41,
	};

	for (int i = 0; i < 64; i++) {
		x = (i % 8) + 1;
		y = 8 - (i / 8);

		char p = this->fields[i].pieceName;
		
		if (p == 'p') {
			evaluation -= engine::Evaluator::PAWN_BASIC_VALUE;

			earlyEvaluation -= pawnTable[i ^ 56];
			egEvaluation -= egPawnTable[i ^ 56];
		}
		else if (p == 'b') {
			evaluation -= engine::Evaluator::BISHOP_BASIC_VALUE;

			earlyEvaluation -= bishopTable[i ^ 56];
			egEvaluation -= egBishopTable[i ^ 56];
			phase++;
		}
		else if (p == 'n') {
			evaluation -= engine::Evaluator::KNIGH_BASIC_VALUE;

			earlyEvaluation -= knightTable[i ^ 56];
			egEvaluation -= egKnightTable[i ^ 56];
			phase++;
			
		}
		else if (p == 'r') {
			evaluation -= engine::Evaluator::ROOK_BASIC_VALUE;

			earlyEvaluation -= rookTable[i ^ 56];
			egEvaluation -= egRookTable[i ^ 56];
			phase += 2;
		}
		else if (p == 'q') {
			evaluation -= engine::Evaluator::QUEEN_BASIC_VALUE;

			if (x == 4 && y == 8) {
				blackQueenOnStartingSquare = true;
			}

			earlyEvaluation -= queenTable[i ^ 56];
			egEvaluation -= egQueenTable[i ^ 56];
			phase += 4;
		} 
		else if (p == 'k') {
			evaluation -= 10000;
			earlyEvaluation -= kingTable[i ^ 56];
			egEvaluation -= egKingTable[i ^ 56];
		}
		else if (p == 'P') {
			evaluation += engine::Evaluator::PAWN_BASIC_VALUE;

			earlyEvaluation += pawnTable[i];
			egEvaluation += egPawnTable[i];
		}
		else if (p == 'B') {
			evaluation += engine::Evaluator::BISHOP_BASIC_VALUE;

			earlyEvaluation += bishopTable[i];
			egEvaluation += egBishopTable[i];
			phase++;
		}
		else if (p == 'N') {
			evaluation += engine::Evaluator::KNIGH_BASIC_VALUE;

			earlyEvaluation += knightTable[i];
			egEvaluation += egKnightTable[i];

			phase++;
		}
		else if (p == 'R') {
			evaluation += engine::Evaluator::ROOK_BASIC_VALUE;

			earlyEvaluation += rookTable[i];
			egEvaluation += egRookTable[i];
			phase += 2;
		}
		else if (p == 'Q') {
			evaluation += engine::Evaluator::QUEEN_BASIC_VALUE;
			earlyEvaluation += queenTable[i];
			egEvaluation += egQueenTable[i];

			if (x == 4 && y == 1) {
				blackQueenOnStartingSquare = false;
			}

			phase += 4;
		}
		else if (p == 'K') {
			evaluation += 10000;
			earlyEvaluation += kingTable[i];
			egEvaluation += egKingTable[i];
		}
	}

	if (phase >= 24) {
		phase = 24;

		if (blackQueenOnStartingSquare) {
			evaluation -= 0.5;
		}

		if (whiteQueenOnStartingSquare) {
			evaluation += 0.5;
		}
	}

	return evaluation + ((earlyEvaluation * phase + egEvaluation * (24 - phase)) / 24);
}

std::string board::Board::getPosition() {
	int empties = 0;
	std::string fen = "";
	
	for (int i = 0; i < 64; i++) {
		if (i % 8 == 0 && i != 0) {
			if (empties != 0) {
				fen += std::to_string(empties);
				empties = 0;
			}
			fen += "/";
		}

		if (this->fields[i].isFieldEmpty) {
			empties++;
		}
		else {
			if (empties != 0) {
				fen += std::to_string(empties);
				empties = 0;
			}

			fen += fields[i].getPiece().pieceName;
		}
	}

	return fen;
}

double board::Board::calculateMoveExtraBonus(move::Move* lastMove)
{
	if (this->getField(lastMove->xFrom, lastMove->yFrom).pieceName != 'q' && this->getField(lastMove->xFrom, lastMove->yFrom).pieceName != 'Q') {
		return 0;
	}

	int blackDevelopedPieces = 0;
	int whiteDevelopedPieces = 0;
	double evaluation = 0;

	if (this->getField(2, 8).pieceName != 'n') {
		blackDevelopedPieces++;
	}

	if (this->getField(3, 8).pieceName != 'b') {
		blackDevelopedPieces++;
	}

	if (this->getField(6, 8).pieceName != 'b') {
		blackDevelopedPieces++;
	}

	if (this->getField(7, 8).pieceName != 'n') {
		blackDevelopedPieces++;
	}

	if (this->getField(2, 1).pieceName != 'N') {
		whiteDevelopedPieces++;
	}

	if (this->getField(3, 1).pieceName != 'B') {
		whiteDevelopedPieces++;
	}

	if (this->getField(6, 1).pieceName != 'B') {
		whiteDevelopedPieces++;
	}

	if (this->getField(7, 1).pieceName != 'N') {
		whiteDevelopedPieces++;
	}

	if (this->getField(lastMove->xTo, lastMove->yTo).pieceName == 'q') {
		evaluation += 0.3 * (4 - blackDevelopedPieces);
	}
	else if (this->getField(lastMove->xTo, lastMove->yTo).pieceName == 'Q') {
		evaluation -= 0.3 * (4 - whiteDevelopedPieces);
	}

	return evaluation;
}