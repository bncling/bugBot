#ifndef BITBOARDS_H
#define BITBOARDS_H

using ulong = unsigned long long int;
using ushort = unsigned short int;

// bunch of functions copied from Sebastian Lague's challenge
// https://seblague.github.io/chess-coding-challenge/documentation/

void SetSquare(ulong& bitboard, int sqInd);

void ClearSquare(ulong& bitboard, int sqInd);

void ToggleSquare(ulong& bitboard, int sqInd);

bool SquareIsSet(ulong& bitboard, int sqInd);

int ClearAndGetLSB(ulong& bitboard);

int NumSetBits(ulong bitboard);

#endif 