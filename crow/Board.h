#pragma once

#include "FEN.h"
#include "Piece.h"
#include <vector>

namespace board {
	class Field {
	public:
		Field() = default;
		Field(int x, int y);
		bool isFieldEmpty;
		void setPiece(pieces::Piece piece);
		pieces::Piece getPiece();
		int x;
		int y;
	private:
		pieces::Piece piece;
	};

	class Board {
	public:
		Board() = default;
		Board(FEN::FEN fen);
		std::vector<Field> fields;
		bool isFieldEmpty(int x, int y);
		bool isFieldOccupiedByOpponentsPiece(int x, int y);
		FEN::FEN fen;
	private:
		void generateFields();
		bool isFieldValid(int x, int y);
		board::Field getField(int x, int y);
	};
}