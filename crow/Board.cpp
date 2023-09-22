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

	for (int i = 0; i < currentPosition.length(); i++) {
		if (currentPosition[i] == '/') {
			row--;
			columnNum = 1;
		}
		else if (helpers::Char::isNumeric(currentPosition[i])) {
			for (int j = 0; j < helpers::Char::castToRealInt(currentPosition[i]); j++) {
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

double board::Board::evaluate(std::string originalColor) {
	double evaluation = 0.0;
	int x = 0;
	int y = 8;

	for (int i = 0; i < this->fields.size(); i++) {
		x = (i % 8) + 1;
		y = 8 - (i / 8);

		char p = this->fields[i].pieceName;
;
		if (p == 'p') {
			evaluation -= engine::Evaluator::PAWN_BASIC_VALUE;

			if ((x >= 3 && x <= 6) && (y >= 3 && y <= 6)) {
				evaluation -= 0.1;
				
				// 4 najbardziej centralne pola na planszy
				if ((x >= 4 && x <= 5) && (y >= 4 && y <= 5)) {
					evaluation -= 0.1;
				}

				evaluation -= 0.05 * (7 - y);

				for (int posY = y - 1; posY > 1; posY--) {
					if (this->getField(x, posY).getPiece().pieceName == FEN::FEN::PAWN_BLACK) { // zdublowane pionki
						evaluation += 0.35;
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

				if (isSemiPassedPawn) {
					evaluation -= 0.75 * (7 - y);
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
		}
		else if (p == 'b') {
			evaluation -= engine::Evaluator::BISHOP_BASIC_VALUE;

			if ((x == 5 || x == 4) && y == 6) {
				if (x == 5) {
					if (this->getField(5, 7).getPiece().pieceName == FEN::FEN::PAWN_BLACK) {
						evaluation += 0.5; // kara
					}
				}
				else {
					if (this->getField(4, 7).getPiece().pieceName == FEN::FEN::PAWN_BLACK) {
						evaluation += 0.5; // kara
					}
				}
			}

			int attackedFields = this->getNumberOfAttackedFieldsInDiagonal(x, y);

			if (attackedFields > 10 && y != 1 && y != 8) {
				evaluation -= 0.15;
			}

			if (x == 4 || x == 5) {
				evaluation -= attackedFields * 0.04;
			}
			else if (x == 3 || x == 6) {
				evaluation -= attackedFields * 0.03;
			}
			else {
				evaluation -= attackedFields * 0.02;
			}
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

			if (x >= 2 && x <= 7 && y >= 2 && y <= 7) {
				evaluation -= 0.1;
				if (x >= 3 && x <= 6 && y >= 3 && y <= 6) {
					evaluation -= 0.1;

					if (x >= 4 && x <= 5 && y >= 4 && y <= 5) {
						evaluation -= 0.1;
					}
				}
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
			}

			evaluation -= 10000;
		}
		else if (p == 'P') {
			evaluation += engine::Evaluator::PAWN_BASIC_VALUE;

			// 4 najbardziej centralne pola na planszy
			if ((x >= 4 && x <= 5) && (y >= 4 && y <= 5)) {
				evaluation += 0.1;
			}

			evaluation += 0.05 * (y - 2);

			for (int posY = y + 1; posY < 8; posY++) {
				if (this->getField(x, posY).getPiece().pieceName == FEN::FEN::PAWN_WHITE) { // zdublowane pionki
					evaluation -= 0.35;
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
				evaluation += 0.75 * (y - 2);
			}

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

			if ((x == 5 || x == 4) && y == 3) {
				if (x == 5) {
					if (this->getField(5, 2).getPiece().pieceName == FEN::FEN::PAWN_WHITE) {
						evaluation -= 0.5; // kara
					}
				}
				else {
					if (this->getField(4, 2).getPiece().pieceName == FEN::FEN::PAWN_WHITE) {
						evaluation -= 0.5; // kara
					}
				}
			}

			int attackedFields = this->getNumberOfAttackedFieldsInDiagonal(x, y);

			if (attackedFields > 10 && y != 1 && y != 8) {
				evaluation += 0.15;
			}

			if (x == 4 || x == 5) {
				evaluation += attackedFields * 0.04;
			}
			else if (x == 3 || x == 6) {
				evaluation += attackedFields * 0.03;
			}
			else {
				evaluation += attackedFields * 0.02;
			}
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

			if (x >= 2 && x <= 7 && y >= 2 && y <= 7) {
				evaluation += 0.1;
				if (x >= 3 && x <= 6 && y >= 3 && y <= 6) {
					evaluation += 0.1;

					if (x >= 4 && x <= 5 && y >= 4 && y <= 5) {
						evaluation += 0.1;
					}
				}
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