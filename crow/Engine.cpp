#include "Engine.h"
#include "Helpers.h"
#include "Board.h"
#include "FEN.h"
#include<fstream>

engine::Engine::Engine(std::string fen)
{
	this->evaluator = Evaluator(fen);
	this->originalFen = FEN::FEN(fen);
	this->originalColor = evaluator.fen.getColor();
}

std::string engine::Engine::findBestMove()
{
	tempBoard = board::Board(this->originalFen);

	std::vector<move::Move> legalMoves = this->findAllLegalMovesOfPosition();

	std::string s = "";

	for (int i = 0; i < legalMoves.size(); i++) {
		s += legalMoves[i].getMovePgn() + '\n';
	}

	return s;
}

std::vector<move::Move> engine::Engine::findAllLegalMovesOfPosition() {
	std::vector<move::Move> legalMoves = {};
	std::string color = tempBoard.fen.getColor();

	for (int i = 0; i < tempBoard.fields.size(); i++) {
		board::Field field = tempBoard.fields[i];
		if (!field.isFieldEmpty) {
			pieces::Piece piece = field.getPiece();
			if ((color == FEN::FEN::COLOR_WHITE && piece.isWhite) || (color == FEN::FEN::COLOR_BLACK && !piece.isWhite)) {
				if (piece.pieceName == FEN::FEN::PAWN_WHITE || piece.pieceName == FEN::FEN::PAWN_BLACK) {
					// ruchy do przodu 

					// ruch o 1 pole do przodu
					if (tempBoard.isFieldEmpty(field.x, field.y + (1 * piece.isWhite ? 1 : -1))) {
						if ((field.y == 2 && piece.isWhite) || (field.y == 7 && !piece.isWhite)) { // linia startowa
							legalMoves.push_back(move::NormalMove(this->tempBoard, field.x, field.y, field.x, field.y + (1 * piece.isWhite ? 1 : -1)));
						}
						//boardAfterMove =  
					}

					// captures
					/*
					if (tempBoard.isFieldOccupiedByOpponentsPiece(field.x - 1, field.y + (1 * piece.isWhite ? 1 : -1))) {

					}

					if (tempBoard.isFieldOccupiedByOpponentsPiece(field.x + 1, field.y + (1 * piece.isWhite ? 1 : -1))) {

					}
					*/
				}
			}
		}
	}

	return legalMoves;
}