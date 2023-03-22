#include "Board.h"
#include "Helpers.h"
#include "Piece.h"
#include "Move.h"
#include<fstream>
#include "Engine.h"

board::Board::Board(FEN::FEN fen) {
	this->fen = fen;

	this->generateFields();
	this->colorOnMove = fen.getColor();
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

	return true;
	//return piece.isReal && (this->colorOnMove == FEN::FEN::COLOR_WHITE && !piece.isWhite) || (this->colorOnMove == FEN::FEN::COLOR_BLACK && piece.isWhite);
}

bool board::Board::canCaptureOnField(int x, int y) {
	if (this->isFieldEmpty(x, y)) {
		return false;
	}

	return this->isFieldOccupiedByOpponentsPiece(x, y);
}

void board::Board::makeMove(move::Move* move) {
	this->fields[(8 - move->yTo) * 8 + (move->xTo - 1)].setPiece(this->fields[(8 - move->yFrom) * 8 + (move->xFrom - 1)].getPiece());
	this->fields[(8 - move->yFrom) * 8 + (move->xFrom - 1)].setPiece(pieces::NoPiece('-'));

	this->colorOnMove = (this->colorOnMove == FEN::FEN::COLOR_WHITE ? FEN::FEN::COLOR_BLACK : FEN::FEN::COLOR_WHITE);
}

double board::Board::evaluate() {
	double evaluation = 0.0;
	
	for (int i = 0; i < this->fields.size(); i++) {

		char p = this->fields[i].pieceName;
;
		if (p == 'p') {
			evaluation -= engine::Evaluator::PAWN_BASIC_VALUE;
		}
		else if (p == 'b') {
			evaluation -= engine::Evaluator::BISHOP_BASIC_VALUE;
		}
		else if (p == 'n') {
			evaluation -= engine::Evaluator::KNIGH_BASIC_VALUE;
		}
		else if (p == 'r') {
			evaluation -= engine::Evaluator::ROOK_BASIC_VALUE;
		}
		else if (p == 'q') {
			evaluation -= engine::Evaluator::QUEEN_BASIC_VALUE;
		} 
		else if (p == 'k') {
			evaluation -= 10000;
		}
		else if (p == 'P') {
			evaluation += engine::Evaluator::PAWN_BASIC_VALUE;
		}
		else if (p == 'B') {
			evaluation += engine::Evaluator::BISHOP_BASIC_VALUE;
		}
		else if (p == 'N') {
			evaluation += engine::Evaluator::KNIGH_BASIC_VALUE;
		}
		else if (p == 'R') {
			evaluation += engine::Evaluator::ROOK_BASIC_VALUE;
		}
		else if (p == 'Q') {
			evaluation += engine::Evaluator::QUEEN_BASIC_VALUE;
		}
		else if (p == 'K') {
			evaluation += 10000;
		}
	}

	return evaluation;
}