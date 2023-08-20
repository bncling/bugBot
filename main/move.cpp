#include "move.h"
#include <iostream>
#include <iomanip>

const char PROMOTIONS[4] = { 'n', 'b', 'r', 'q' };
const char FILES[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
const char RANKS[8] = { '1', '2', '3', '4', '5', '6', '7', '8' };

bool Move::Equals(Move otherMove)
{
	return (
		StartSquare == otherMove.StartSquare &&
		TargetSquare == otherMove.TargetSquare && 
		CapturePieceType == otherMove.CapturePieceType &&
		PromotionPieceType == otherMove.PromotionPieceType &&
		MovePieceType == otherMove.MovePieceType
	);
}

void Move::Reset()
{
	StartSquare = 64;
	TargetSquare = 64;

	IsCapture = false;
	IsCastles = false;
	IsPromotion = false;
	IsEnPassant = false;
	IsDoublePawnPush = false;

	MovePieceType = 0;
	CapturePieceType = 0;
	PromotionPieceType = 0;
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

std::ostream& operator<<(std::ostream& os, const Move rhsObj)
{
	int promType = rhsObj.PromotionPieceType;
	os << intToAlg(rhsObj.StartSquare) << intToAlg(rhsObj.TargetSquare);
	if (promType != 0)
	{
		if (promType > 6)
		{
			promType -= 6;
		}

		promType -= 2;
		os << PROMOTIONS[promType];
	}
	else
	{
		os << " ";
	}
	os << " (" << std::setw(2) << rhsObj.MovePieceType + 1 << " " << std::setw(2) << rhsObj.CapturePieceType;
	os << " " << std::setw(2) << rhsObj.PromotionPieceType << " | " << rhsObj.IsCapture;
	os << " " << rhsObj.IsCastles << " " << rhsObj.IsEnPassant << " " << rhsObj.IsDoublePawnPush << " )";

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
