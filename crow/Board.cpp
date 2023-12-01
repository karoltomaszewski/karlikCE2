#include "Board.h"
#include "Helpers.h"
#include "Piece.h"
#include "Move.h"
#include<fstream>
#include<string>
#include "Engine.h"

board::Board::Board(FEN::FEN fen) {
	this->fen = fen;

	this->generateFields();
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

void board::Board::generateFields() {
	std::string currentPosition = this->fen.getPosition();

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

double board::Board::evaluate(std::string originalColor, move::Move* lastMove) {
	double evaluation = 0.0;
	double earlyGameEvaluation = 0.0;
	double endGameEvaluation = 0.0;
	int x = 0;
	int y = 8;

	int gamePhase = 0;

	for (int i = 0; i < 64; i++) {
		if (this->fields[i].isFieldEmpty) {
			continue;
		}

		x = (i % 8) + 1;
		y = 8 - (i / 8);

		char p = this->fields[i].pieceName;
		
		if (p == 'p') {
			earlyGameEvaluation -= 0.82;
			endGameEvaluation -= 0.94;

			earlyGameEvaluation -= this->pawnTable[i ^ 56];
			endGameEvaluation -= this->egPawnTable[i ^ 56];
		}
		else if (p == 'b') {
			earlyGameEvaluation -= 3.65;
			endGameEvaluation -= 2.97;

			earlyGameEvaluation -= this->bishopTable[i ^ 56];
			endGameEvaluation -= this->egBishopTable[i ^ 56];

			gamePhase++;
		}
		else if (p == 'n') {
			earlyGameEvaluation -= 3.37;
			endGameEvaluation -= 2.81;

			earlyGameEvaluation -= this->knightTable[i ^ 56];
			endGameEvaluation -= this->egKnightTable[i ^ 56];

			gamePhase++;
		}
		else if (p == 'r') {
			earlyGameEvaluation -= 4.77;
			endGameEvaluation -= 5.12;

			earlyGameEvaluation -= this->rookTable[i ^ 56];
			endGameEvaluation -= this->egRookTable[i ^ 56];

			gamePhase += 2;
		}
		else if (p == 'q') {
			earlyGameEvaluation -= 10.25;
			endGameEvaluation -= 9.36;

			earlyGameEvaluation -= this->queenTable[i ^ 56];
			endGameEvaluation -= this->egQueenTable[i ^ 56];

			gamePhase += 4;
		} 
		else if (p == 'k') {
			evaluation -= 10000;
			earlyGameEvaluation -= this->kingTable[i ^ 56];
			endGameEvaluation -= this->egKingTable[i ^ 56];
		}
		else if (p == 'P') {
			earlyGameEvaluation += 0.82;
			endGameEvaluation += 0.94;

			earlyGameEvaluation += this->pawnTable[i];
			endGameEvaluation += this->egPawnTable[i];
		}
		else if (p == 'B') {
			earlyGameEvaluation += 3.65;
			endGameEvaluation += 2.97;

			earlyGameEvaluation += this->bishopTable[i];
			endGameEvaluation += this->egBishopTable[i];

			gamePhase++;
		}
		else if (p == 'N') {
			earlyGameEvaluation += 3.37;
			endGameEvaluation += 2.81;

			earlyGameEvaluation += this->knightTable[i];
			endGameEvaluation += this->egKnightTable[i];

			gamePhase++;
		}
		else if (p == 'R') {
			earlyGameEvaluation += 4.77;
			endGameEvaluation += 5.12;

			earlyGameEvaluation += this->rookTable[i];
			endGameEvaluation += this->egRookTable[i];

			gamePhase += 2;
		}
		else if (p == 'Q') {
			earlyGameEvaluation += 10.25;
			endGameEvaluation += 9.36;

			earlyGameEvaluation += this->queenTable[i];
			endGameEvaluation += this->egQueenTable[i];

			gamePhase += 4;
		}
		else if (p == 'K') {
			evaluation += 10000;
			earlyGameEvaluation += this->kingTable[i];
			endGameEvaluation += this->egKingTable[i];
		}
	}

	if (gamePhase > 24) {
		gamePhase = 24;
	}

	return ((earlyGameEvaluation * gamePhase) + (endGameEvaluation * (24 - gamePhase))) / 24;
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

	if (this->getField(lastMove->xFrom, lastMove->yFrom).pieceName == 'q' && this->colorOnMove == FEN::FEN::COLOR_BLACK) {
		evaluation += 0.3 * (4 - blackDevelopedPieces);
	}
	else if (this->getField(lastMove->xFrom, lastMove->yFrom).pieceName == 'Q' && this->colorOnMove == FEN::FEN::COLOR_WHITE) {
		evaluation -= 0.3 * (4 - whiteDevelopedPieces);
	}

	return evaluation;
}