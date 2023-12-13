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

double board::Board::evaluate() {
	double evaluation = 0.0;
	int x = 0;
	int y = 8;

	int blackDevelopedPieces = 0;
	int whiteDevelopedPieces = 0;

	double pawnTable[64] = {
			 0,   0,   0,   0,   0,   0,  0,   0,
			 0.98, 1.34,  0.61,  0.95,  0.68, 1.26, 0.34, -0.11,
			 -0.06,   0.07,  0.26,  0.31,  0.65,  0.56, 0.25, -0.2,
			-0.14,  0.13,   0.06,  0.21,  0.33,  0.12, 0.17, -0.23,
			-0.27,  -0.02,  -0.05,  0.22,  0.27,   0.06, 0.1, -0.25,
			-0.26,  -0.04,  -0.04, 0,   0,   0.03, 0.33, -0.12,
			-0.35,  -0.01, -0.20, -0.43, -0.35,  0.24, 0.38, -0.22,
			  0,   0,   0,   0,   0,   0,  0,   0,
	};

	if (this->getField(2, 8).pieceName != 'n') {
		evaluation -= 0.12;
		blackDevelopedPieces++;
	}

	if (this->getField(3, 8).pieceName != 'b') {
		evaluation -= 0.12;
		blackDevelopedPieces++;
	}

	if (this->getField(6, 8).pieceName != 'b') {
		evaluation -= 0.06;
		blackDevelopedPieces++;
	}

	if (this->getField(7, 8).pieceName != 'n') {
		evaluation -= 0.12;
		blackDevelopedPieces++;
	}

	if (this->getField(4, 7).pieceName != 'p') {
		evaluation -= 0.12;
		blackDevelopedPieces++;
	}

	if (this->getField(5, 7).pieceName != 'p') {
		evaluation -= 0.12;
		blackDevelopedPieces++;
	}

	if (this->getField(2, 1).pieceName != 'N') {
		evaluation += 0.06;
		whiteDevelopedPieces++;
	}

	if (this->getField(3, 1).pieceName != 'B') {
		evaluation += 0.12;
		whiteDevelopedPieces++;
	}

	if (this->getField(6, 1).pieceName != 'B') {
		evaluation += 0.12;
		whiteDevelopedPieces++;
	}

	if (this->getField(7, 1).pieceName != 'N') {
		evaluation += 0.12;
		whiteDevelopedPieces++;
	}

	if (this->getField(4, 7).pieceName != 'P') {
		evaluation += 0.12;
		whiteDevelopedPieces++;
	}

	if (this->getField(5, 7).pieceName != 'P') {
		evaluation += 0.12;
		whiteDevelopedPieces++;
	}

	for (int i = 0; i < 64; i++) {
		x = (i % 8) + 1;
		y = 8 - (i / 8);

		char p = this->fields[i].pieceName;
		
		if (p == 'p') {
			evaluation -= engine::Evaluator::PAWN_BASIC_VALUE;

			if (this->isFieldValid(x - 1, y + 2) && this->getField(x - 1, y + 2).pieceName == 'p' && !this->getField(x - 1, y + 1).isFieldEmpty && this->getField(x - 1, y + 1).getPiece().power > 1 && !this->getField(x - 1, y + 1).getPiece().isWhite) {
				evaluation += 0.5;
			}

			if (this->isFieldValid(x + 1, y + 2) && this->getField(x + 1, y + 2).pieceName == 'p' && !this->getField(x - 1, y + 1).isFieldEmpty && this->getField(x + 1, y + 1).getPiece().power > 1 && !this->getField(x + 1, y + 1).getPiece().isWhite) {
				evaluation += 0.5;
			}

			for (int posY = y - 1; posY > 1; posY--) {
				if (this->getField(x, posY).getPiece().pieceName == FEN::FEN::PAWN_BLACK) { // zdublowane pionki
					evaluation += 0.3;
					break;
				}
			}

			bool isSemiPassedPawn = true; // semi passed pawn - pion, który na swojej drodze nie mo¿e zostaæ zablokowany ani zbity przez pionka
			for (int posY = y - 1; posY > 1; posY--) {
				if (
					!this->getField(x, posY).getPiece().isReal ||
					(this->isFieldValid(x - 1, posY - 1) && this->getField(x - 1, posY - 1).getPiece().pieceName == FEN::FEN::PAWN_WHITE) ||
					(this->isFieldValid(x + 1, posY - 1) && this->getField(x + 1, posY - 1).getPiece().pieceName == FEN::FEN::PAWN_WHITE)
					) {
					isSemiPassedPawn = false;
					break;
				}
			}

			evaluation -= pawnTable[i ^ 56];

			if (isSemiPassedPawn) {
				if (y < 6) {
					evaluation -= 0.75 * (7 - y);
				}
			}

			if (x != 1) {
				pieces::Piece pieceOnAttackedField = this->getField(x - 1, y - 1).getPiece();
				if (pieceOnAttackedField.isReal && pieceOnAttackedField.pieceName != FEN::FEN::PAWN_WHITE) {
					// bonus za atakowanie pionkiem figury
					evaluation -= 0.1;
				}
			}

			if (x != 8) {
				pieces::Piece pieceOnAttackedField = this->getField(x + 1, y - 1).getPiece();
				if (pieceOnAttackedField.isReal && pieceOnAttackedField.pieceName != FEN::FEN::PAWN_WHITE) {
					// bonus za atakowanie pionkiem figury
					evaluation -= 0.1;
				}
			}
		}
		else if (p == 'b') {
			evaluation -= engine::Evaluator::BISHOP_BASIC_VALUE;


			int attackedFields = this->getNumberOfAttackedFieldsInDiagonal(x, y);


			evaluation -= attackedFields * 0.03;
		}
		else if (p == 'n') {
			evaluation -= engine::Evaluator::KNIGH_BASIC_VALUE;

			if ((x >= 2 && x <= 7) && y >= 3 && y <= 7) {
				evaluation -= 0.1;
				if ((x >= 3 && x <= 6) && (y >= 3 && y <= 6)) {
					evaluation -= 0.2;
				}
			}
			
		}
		else if (p == 'r') {
			evaluation -= engine::Evaluator::ROOK_BASIC_VALUE;

			int attackedFields = this->getNumberOfAttackedFieldsInLines(x, y);

			if (attackedFields > 10 && y != 1 && y != 8) {
				evaluation -= 0.3;
			}
		}
		else if (p == 'q') {
			evaluation -= engine::Evaluator::QUEEN_BASIC_VALUE;

			if (x != 4 || y != 8) { // not on starting field
				evaluation += 0.3 * (6 - blackDevelopedPieces);
			}
		} 
		else if (p == 'k') {
			if (x == 7 && y == 8) { // pole po roszadzie
				if (
					this->getField(6, 7).getPiece().pieceName == FEN::FEN::PAWN_BLACK &&
					this->getField(7, 7).getPiece().pieceName == FEN::FEN::PAWN_BLACK &&
					(this->getField(8, 7).getPiece().pieceName == FEN::FEN::PAWN_BLACK || this->getField(8, 6).getPiece().pieceName == FEN::FEN::PAWN_BLACK)
				) {
					evaluation -= 1;
				}

				evaluation -= 0.2;
			}

			evaluation -= 10000;
		}
		else if (p == 'P') {
			evaluation += engine::Evaluator::PAWN_BASIC_VALUE;

			if (this->isFieldValid(x - 1, y - 2) && this->getField(x - 1, y - 2).pieceName == 'P' && !this->getField(x - 1, y - 1).isFieldEmpty && this->getField(x - 1, y - 1).getPiece().power > 1 && this->getField(x - 1, y - 1).getPiece().isWhite) {
				evaluation -= 0.5;
			}

			if (this->isFieldValid(x + 1, y - 2) && this->getField(x + 1, y - 2).pieceName == 'P' && !this->getField(x + 1, y - 1).isFieldEmpty && this->getField(x + 1, y - 1).getPiece().power > 1 && this->getField(x + 1, y - 1).getPiece().isWhite) {
				evaluation -= 0.5;
			}

			for (int posY = y + 1; posY < 8; posY++) {
				if (this->getField(x, posY).getPiece().pieceName == FEN::FEN::PAWN_WHITE) { // zdublowane pionki
					evaluation -= 0.3;
					break;
				}
			}

			bool isSemiPassedPawn = true;
			for (int posY = y + 1; posY < 8; posY++) {
				if (
					!this->getField(x, posY).getPiece().isReal ||
					(this->isFieldValid(x - 1, posY + 1) && this->getField(x - 1, posY + 1).getPiece().pieceName == FEN::FEN::PAWN_BLACK) ||
					(this->isFieldValid(x + 1, posY + 1) && this->getField(x + 1, posY + 1).getPiece().pieceName == FEN::FEN::PAWN_BLACK)
				) {
					isSemiPassedPawn = false;
					break;
				}
			}

			if (isSemiPassedPawn) {
				if (y > 3) {
					evaluation += 0.75 * (y - 2);
				}
			}

			evaluation += pawnTable[i];

			if (x != 1) {
				pieces::Piece pieceOnAttackedField = this->getField(x - 1, y + 1).getPiece();
				if (pieceOnAttackedField.isReal && pieceOnAttackedField.pieceName != FEN::FEN::PAWN_BLACK) {
					// bonus za atakowanie pionkiem figury
					evaluation += 0.1;
				}
			}

			if (x != 8) {
				pieces::Piece pieceOnAttackedField = this->getField(x + 1, y + 1).getPiece();
				if (pieceOnAttackedField.isReal && pieceOnAttackedField.pieceName != FEN::FEN::PAWN_BLACK) {
					// bonus za atakowanie pionkiem figury
					evaluation += 0.1;
				}
			}
		}
		else if (p == 'B') {
			evaluation += engine::Evaluator::BISHOP_BASIC_VALUE;

			int attackedFields = this->getNumberOfAttackedFieldsInDiagonal(x, y);

			evaluation += attackedFields * 0.03;
		}
		else if (p == 'N') {
			evaluation += engine::Evaluator::KNIGH_BASIC_VALUE;

			if ((x >= 2 && x <= 7) && y >= 3 && y <= 7) {
				evaluation += 0.1;
				if ((x >= 3 && x <= 6) && (y >= 3 && y <= 6)) {
					evaluation += 0.2;
				}
			}
		}
		else if (p == 'R') {
			evaluation += engine::Evaluator::ROOK_BASIC_VALUE;

			int attackedFields = this->getNumberOfAttackedFieldsInLines(x, y);

			if (attackedFields > 10 && y != 1 && y != 8) {
				evaluation += 0.3;
			}
		}
		else if (p == 'Q') {
			evaluation += engine::Evaluator::QUEEN_BASIC_VALUE;

			if (x != 4 || y != 8) { // not on starting field
				evaluation -= 0.3 * (6 - whiteDevelopedPieces);
			}
		}
		else if (p == 'K') {
			evaluation += 10000;

			if (x == 7 && y == 1) { // pole po roszadzie
				if (
					this->getField(6, 2).getPiece().pieceName == FEN::FEN::PAWN_BLACK &&
					this->getField(7, 2).getPiece().pieceName == FEN::FEN::PAWN_BLACK &&
					(this->getField(8, 2).getPiece().pieceName == FEN::FEN::PAWN_BLACK || this->getField(8, 2).getPiece().pieceName == FEN::FEN::PAWN_BLACK)
					) {
					evaluation += 1;
				}

				evaluation += 0.2;
			}

		}
	}

	return evaluation;
}

int board::Board::getNumberOfAttackedFieldsInLines(int x, int y)
{
	int attackedFields = 0;
	for (int posX = x - 1; posX > 0; posX--) {
		attackedFields++;
		if (!this->getField(posX, y).isFieldEmpty) {
			break;
		}
	}
	for (int posX = x + 1; posX < 9; posX++) {
		attackedFields++;
		if (!this->getField(posX, y).isFieldEmpty) {
			break;
		}
	}
	for (int posY = y - 1; posY > 0; posY--) {
		attackedFields++;
		if (!this->getField(x, posY).isFieldEmpty) {
			break;
		}
	}
	for (int posY = y + 1; posY < 9; posY++) {
		attackedFields++;
		if (!this->getField(x, posY).isFieldEmpty) {
			break;
		}
	}

	return attackedFields;
}

int board::Board::getNumberOfAttackedFieldsInDiagonal(int x, int y)
{
	int attackedFields = 0;
	for (int posX = x - 1, posY = y - 1; (posX > 0 && posY > 0);) {
		attackedFields++;
		if (!this->getField(posX, y).isFieldEmpty) {
			break;
		}

		posX--;
		posY--;
	}
	for (int posX = x + 1, posY = y - 1; (posX < 9 && posY > 0);) {
		attackedFields++;
		if (!this->getField(posX, y).isFieldEmpty) {
			break;
		}

		posX++;
		posY--;
	}
	for (int posX = x - 1, posY = y + 1; (posX > 0 && posY < 9);) {
		attackedFields++;
		if (!this->getField(x, posY).isFieldEmpty) {
			break;
		}

		posX--;
		posY++;
	}
	for (int posX = x + 1, posY = y + 1; (posX < 9 && posY < 9);) {
		attackedFields++;
		if (!this->getField(x, posY).isFieldEmpty) {
			break;
		}

		posX++;
		posY++;
	}

	return attackedFields;
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