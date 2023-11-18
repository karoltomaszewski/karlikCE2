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
			colorOnMove = board.colorOnMove;
			canWhiteKingCastle = board.canWhiteKingCastle;
			canWhiteQueenCastle = board.canWhiteQueenCastle;
			canBlackKingCastle = board.canBlackKingCastle;
			canBlackQueenCastle = board.canBlackQueenCastle;
			canEnPassant = board.canEnPassant;
			enPassantX = board.enPassantX;
			enPassantY = board.enPassantY;

			whiteKingX = board.whiteKingX;
			whiteKingY = board.whiteKingY;
			blackKingX = board.blackKingX;
			blackKingY = board.blackKingY;
		}
		std::vector<Field> fields;
		bool isFieldEmpty(int x, int y);
		bool isFieldOccupiedByOpponentsPiece(int x, int y);
		bool canCaptureOnField(int x, int y);
		FEN::FEN fen;
		void makeMove(move::Move*);
		double evaluate(std::string originalColor, move::Move* lastMove);
		double calculateMoveExtraBonus( move::Move* lastMove);
		bool isFieldValid(int x, int y);
		std::string colorOnMove;
		board::Field getField(int x, int y);
		std::string getPosition();

		static int calculateIndex(int x, int y);

		bool canWhiteKingCastle;
		bool canWhiteQueenCastle;
		bool canBlackKingCastle;
		bool canBlackQueenCastle;

		int whiteKingX;
		int whiteKingY;
		int blackKingX;
		int blackKingY;

		bool canEnPassant;
		int enPassantX;
		int enPassantY;
	private:
		void generateFields();
		int getNumberOfAttackedFieldsInLines(int x, int y);
		int getNumberOfAttackedFieldsInDiagonal(int x, int y);
	};
}