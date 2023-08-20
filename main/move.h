#ifndef MOVE_H
#define MOVE_H
#include <string>

struct Move
{
	int StartSquare;
	int TargetSquare;

	bool IsCapture;
	bool IsCastles;
	bool IsPromotion;
	bool IsEnPassant;
	bool IsDoublePawnPush;

	int MovePieceType;
	int CapturePieceType;
	int PromotionPieceType;

	bool Equals(Move otherMove);

	void Reset();

	friend std::ostream& operator<<(std::ostream& os, const Move rhsObj);
};

#endif