#pragma once
#include <string>

namespace pieces {
	class Piece
	{
	public:
		Piece() = default;
		bool isWhite = false;
		bool isReal = true;
		char pieceName;
		int power;
	};
	
	class PieceFactory 
	{
	public:
		static Piece create(char pieceName);
		static Piece create(int promotionCode, std::string color);
	};

	class Pawn : public Piece 
	{
	public:
		Pawn() = default;
		Pawn(char pieceName);
		//virtual void findLegalMoves();
	};

	class Knight : public Piece
	{
	public:
		Knight() = default;
		Knight(char pieceName);
	};

	class Bishop : public Piece
	{
	public:
		Bishop() = default;
		Bishop(char pieceName);
	};

	class Rook : public Piece
	{
	public:
		Rook() = default;
		Rook(char pieceName);
	};

	class Queen : public Piece
	{
	public:
		Queen() = default;
		Queen(char pieceName);
	};

	class King : public Piece
	{
	public:
		King() = default;
		King(char pieceName);
	};

	class NoPiece : public Piece
	{
	public:
		NoPiece() = default;
		NoPiece(char pieceName);
	};
}