#include "move.h"
#include <iostream>
#include <iomanip>

const char PROMOTIONS[4] = { 'n', 'b', 'r', 'q' };
const char FILES[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
const char RANKS[8] = { '1', '2', '3', '4', '5', '6', '7', '8' };

bool Move::Equals(Move otherMove)
{
	return moveCode == otherMove.moveCode;
}

void Move::Reset()
{
	moveCode = 0U;

	MovePieceType = 0;
	CapturePieceType = 0;
}

std::string intToAlg(int index)
{
	if (index == 64)
	{
		return "00";
	}

	int file = index % 8;
	int rank = 7 - (index - file) / 8;

	std::string algebraic;
	algebraic += FILES[file];
	algebraic += RANKS[rank];

	return algebraic;
}

std::ostream& operator<<(std::ostream& os, Move rhsObj)
{
	os << intToAlg(rhsObj.getStartSquare()) << intToAlg(rhsObj.getTargetSquare());
	os << " (" << rhsObj.isCapture() << " " << rhsObj.isCastles() << " " << rhsObj.isEnPassant() << " " << rhsObj.isDoublePawnPush() << ")";
	os << " " << std::bitset<16>(rhsObj.moveCode) << " " << rhsObj.MovePieceType << " " << rhsObj.CapturePieceType;

	return os;
}

/*
Move::Move()
{
	StartSquare = 64;
	TargetSquare = 64;
	// All piece types are 0 by default, representing a null piece
	// bools all false by default
	MoveString = "--";
}*/

/*
Move::Move(std::string UCIString, Board& board)
{
	MoveString = UCIString;

	if (MoveString.size() > 4) {
		IsPromotion = true;
		for (int i = 0; i < 4; i++)
		{
			if (MoveString[4] == PROMOTIONS[i])
			{
				PromotionPieceType = i + 2;
			}
		}
	}

	std::cout << MoveString.size() << std::endl;
}*/
