#ifndef MOVE_H
#define MOVE_H
#include <string>
#include "bitboards.h"
#include "constants.h"

struct Move
{
	// moveCode is fffcttttttssssss
	// f = flag
	// c = capture
	// t = target
	// s = start

	ushort moveCode = 0U;

	int MovePieceType;
	int CapturePieceType;

	bool Equals(Move otherMove);

	void Reset();

	int getStartSquare() {return (moveCode & Constants::startMask); }
	int getTargetSquare() {return ((moveCode & Constants::targetMask) >> 6); }

	bool isCapture() { return (((moveCode >> 12) % 2) == 1); }
	bool isEnPassant() { return ((moveCode >> 13) == Constants::EnPassantFlag); }
	bool isCastles() { return ((moveCode >> 13) == Constants::CastleFlag); }
	bool isDoublePawnPush() { return ((moveCode >> 13) == Constants::DoublePawnFlag); }
	bool isPromotion() { return ((moveCode >> 13) >= Constants::KnightPromoteFlag); } 
	int promotionPieceType(bool isWhite) { return ((moveCode >> 13) + 3 - 6*isWhite);  }

	friend std::ostream& operator<<(std::ostream& os, const Move rhsObj);
};

#endif