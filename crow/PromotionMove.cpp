#include "Move.h"
#include "Engine.h"

move::PromotionMove::PromotionMove(int xFrom, int yFrom, int xTo, int yTo, int promotionCode) {

	this->xFrom = xFrom;
	this->yFrom = yFrom;
	this->xTo = xTo;
	this->yTo = yTo;
	this->promotionCode = promotionCode;
}

std::string move::PromotionMove::getMoveICCF() {

	return std::to_string(this->xFrom) + std::to_string(this->yFrom) + std::to_string(this->xTo) + std::to_string(this->yTo) + std::to_string(this->promotionCode);
}
