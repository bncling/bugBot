#include "bitboards.h"
#include <bitset>

const ulong MAX_UL = -1;

void SetSquare(ulong& bitboard, int sqInd)
{
	bitboard |= 1ULL << sqInd;
}

void ClearSquare(ulong& bitboard, int sqInd)
{
	bitboard &= ~(1ULL << sqInd);
}

void ToggleSquare(ulong& bitboard, int sqInd)
{
	bitboard ^= 1ULL << sqInd;
}

bool SquareIsSet(ulong& bitboard, int sqInd)
{
	return ((bitboard >> sqInd) & 1) != 0;
}

int ClearAndGetLSB(ulong& bitboard)
{
	int trailing = __builtin_ctzll(bitboard);
	bitboard &= (bitboard - 1);
	return trailing;
}

int NumSetBits(ulong bitboard)
{
	//std::bitset<64> newBoard = bitboard;
	//return newBoard.count();
	return __builtin_popcountll(bitboard);
}