#include "Board.h"
#include "Helpers.h"
#include "Piece.h"
#include "Move.h"
#include<fstream>

board::Board::Board(FEN::FEN fen) {
	this->fen = fen;

	this->generateFields();
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
				this->fields.push_back(board::Field(columnNum, row));
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
	std::ofstream zapis("dane.txt");
	zapis << this->fields.size();
	return this->fields[(8 - y) * 8 + (x - 1)];
}

bool board::Board::isFieldEmpty(int x, int y) {
	if (!this->isFieldValid(x, y)) {
		return false;
	}

	return this->getField(x, y).isFieldEmpty;
}

bool board::Board::isFieldOccupiedByOpponentsPiece(int x, int y) {
	if (this->isFieldValid(x, y)) {
		return false;
	}

	if (this->isFieldEmpty(x, y)) {
		return false;
	}

	pieces::Piece piece = this->fields[(y - 8) * 8 + (x - 1)].getPiece();

	return (this->fen.getColor() == FEN::FEN::COLOR_WHITE && piece.isWhite) || (this->fen.getColor() == FEN::FEN::COLOR_BLACK && !piece.isWhite);
}
