#pragma once

#include "FEN.h"
#include "Piece.h"
#include <vector>
#include "Move.h"

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
		char pieceName;
	private:
		pieces::Piece piece;
	};

	class Board {
	public:
		Board() = default;
		Board(FEN::FEN fen);
		Board(const board::Board& board) {
			fields = board.fields;
			fen = board.fen;
		}
		std::vector<Field> fields;
		bool isFieldEmpty(int x, int y);
		bool isFieldOccupiedByOpponentsPiece(int x, int y);
		bool canCaptureOnField(int x, int y);
		FEN::FEN fen;
		void makeMove(move::Move*);
		double evaluate();
		bool isFieldValid(int x, int y);
		std::string colorOnMove;
	private:
		void generateFields();
		board::Field getField(int x, int y);
	};
}