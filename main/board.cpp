#include "board.h"
#include "bitboards.h"
#include "move.h"
#include "constants.h"
#include <cmath>
#include <string>
#include <vector>
#include <iostream>

const char charList[12] = { 'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k' };
const char FILES[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
const char RANKS[8] = { '1', '2', '3', '4', '5', '6', '7', '8' };

int Board::algToInt(std::string sqName)
{
	int Index = 64;
	int File, Rank;
	if (sqName != "-")
	{
		for (int i = 0; i < 8; i++)
		{
			if (sqName[0] == FILES[i])
			{
				File = i;
				break;
			}
		}

		for (int i = 0; i < 8; i++)
		{
			if (sqName[1] == RANKS[i])
			{
				Rank = i;
				break;
			}
		}

		Index = 8 * (7 - Rank) + File;
	}

	return Index;
}

std::string Board::intToAlg(int index)
{
	if (index == 64)
	{
		return "-";
	}

	int file = index % 8;
	int rank = 7 - (index - file) / 8;

	std::string algebraic;
	algebraic += FILES[file];
	algebraic += RANKS[rank];

	return algebraic;
}

Board::Board()
{
	fenString = "8/8/8/8/8/8/8/8 w - - 0 1";
	whiteToMove = true;

	/*
	whiteKingside = false;
	whiteQueenside = false;
	blackKingside = false;
	blackQueenside = false;*/

	castlingRights.push_back(0U);
	enPassantSquares.push_back(64);
	zobristHistory.push_back(0ULL);
	gameOver = false;
}

Board::Board(std::string fen)
{
	castlingRights.clear();
	enPassantSquares.clear();
	capturedPieces.clear();
	moveHistory.clear();
	zobristHistory.clear();
	gamePhases.clear();
	moveCountsToFifty.clear();

	fenString = fen;

	/*whiteKingside = false;
	whiteQueenside = false;
	blackKingside = false;
	blackQueenside = false;*/

	ushort canCastle = 0;

	std::string enPassantString;

	zobristHistory.push_back(0ULL);

	gameOver = false;

	stalemate = false;
	insufficientMaterial = false;
	threefoldRep = false;
	fiftyMoves = false;

	moveCountsToFifty.push_back(0);

	gamePhases.push_back(0);

	checkmate = 0; // no side has been checkmated

	int index = 0;
	int section = 0;
	for (int i = 0; i < fen.size(); i++)
	{
		if (fen[i] == ' ')
		{
			if (section > 2) break;
			section++;
		} else {
			switch (section)
			{
				// parsing pieces on the board
				case 0:
				{
					int possiblyNumeric = fen[i] - '0';
					if (possiblyNumeric > 0 && possiblyNumeric < 9)
					{
						index += possiblyNumeric;
					} else if (fen[i] != '/') {
						int pieceCode;

						for (int j = 0; j < 12; j++)
						{
							if (fen[i] == charList[j])
							{
								pieceCode = j + 1;
								break;
							}
						}

						forDrawing[index] = pieceCode;

						SetSquare(pieceBitboards[pieceCode - 1], index);
						zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[pieceCode - 1][index];

						gamePhases.back() += Constants::PIECE_PHASES[pieceCode - 1];

						index++;
					}

					break;
				}

				// parsing side to move
				case 1:
				{
					whiteToMove = (fen[i] == 'w');
					if (whiteToMove) zobristHistory.back() ^= Constants::WHITE_ZOBRIST;
					break;
				}

				// parsing castling rights
				case 2:
				{
					switch (fen[i])
					{
						case 'K':
						{
							//whiteKingside = true;
							canCastle |= 1U;
							break;
						}

						case 'Q':
						{
							//whiteQueenside = true;
							canCastle |= 2U;
							break;
						}

						case 'k':
						{
							//blackKingside = true;
							canCastle |= 4U;
							break;
						}

						case 'q':
						{
							//blackQueenside = true;
							canCastle |= 8U;
							break;
						}
					}

					break;
				}

				// parsing en passant moves
				case 3:
				{
					enPassantString += fen[i];
					break;
				}
			}
		}
	}

	for (int i = 0; i < 6; i++)
	{
		//whitePieces |= pieceBitboards[i];
		colorPieces[1] |= pieceBitboards[i];
		//blackPieces |= pieceBitboards[i + 6];
		colorPieces[0] |= pieceBitboards[i + 6];
	}

	allPieces = colorPieces[0] | colorPieces[1];

	castlingRights.push_back(canCastle);
	zobristHistory.back() ^= Constants::CASTLE_ZOBRISTS[castlingRights.back()];

	enPassantSquare = algToInt(enPassantString);
	enPassantSquares.push_back(algToInt(enPassantString));
	if (enPassantSquare != 64) zobristHistory.back() ^= Constants::EN_PASSANT_FILE_ZOBRISTS[enPassantSquare % 8];

	// add this position to the repetition table, using lowest 14 bits as index
	//repetitionTable[zobristHistory.back() & 16383]++;

	//std::cout << zobristHistory.back() << std::endl;

	//std::cout << gamePhases.back() << std::endl;

	updateAttacks();

	getLegalMoves();
}

ulong Board::getAttacks(int attackingPiece)
{
	return attacks[attackingPiece];
}

void Board::updateAttacks()
{
	// white pawns
	ulong iterBitboard = pieceBitboards[0];
	ulong moveMask;
	int squareToCheck, magicIndex, kingMagic;
	kingAttacks[0] = 0;
	kingAttacks[1] = 0;

	for (int i = 0; i < 12; i++)
	{
		attacks[i] = 0ULL;
	}

	// white pawns
	while (iterBitboard != 0)
	{
		attacks[0] |= Constants::WHITE_PAWN_ATTACKS[ClearAndGetLSB(iterBitboard)];
	}

	// white knights
	iterBitboard = pieceBitboards[1];
	while (iterBitboard != 0)
	{
		attacks[1] |= Constants::KNIGHT_ATTACKS[ClearAndGetLSB(iterBitboard)];
	}

	// white bishops + queen
	iterBitboard = pieceBitboards[2] | pieceBitboards[4];
	while (iterBitboard != 0)
	{
		squareToCheck = ClearAndGetLSB(iterBitboard);
		moveMask = Constants::BISHOP_MASKS[squareToCheck];
		magicIndex = (((moveMask & allPieces) * Constants::BISHOP_MAGICS[squareToCheck]) >> 54);
		kingMagic = (((moveMask & (allPieces & ~pieceBitboards[11])) * Constants::BISHOP_MAGICS[squareToCheck]) >> 54);
		attacks[2] |= Constants::BISHOP_ATTACKS[squareToCheck][magicIndex];
		kingAttacks[1] |= Constants::BISHOP_ATTACKS[squareToCheck][kingMagic];
	}

	// white rooks + queen
	iterBitboard = pieceBitboards[3] | pieceBitboards[4];
	while (iterBitboard != 0)
	{
		squareToCheck = ClearAndGetLSB(iterBitboard);
		moveMask = Constants::ROOK_MASKS[squareToCheck];
		magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[squareToCheck]) >> 52);
		kingMagic = (((moveMask & (allPieces & ~pieceBitboards[11])) * Constants::ROOK_MAGICS[squareToCheck]) >> 52);
		attacks[3] |= Constants::ROOK_ATTACKS[squareToCheck][magicIndex];
		kingAttacks[1] |= Constants::ROOK_ATTACKS[squareToCheck][kingMagic];
	}

	// white king
	iterBitboard = pieceBitboards[5];
	while (iterBitboard != 0)
	{
		attacks[5] |= Constants::KING_ATTACKS[ClearAndGetLSB(iterBitboard)];
	}

	// black pawns
	iterBitboard = pieceBitboards[6];
	while (iterBitboard != 0)
	{
		attacks[6] |= Constants::BLACK_PAWN_ATTACKS[ClearAndGetLSB(iterBitboard)];
	}

	// black knights
	iterBitboard = pieceBitboards[7];
	while (iterBitboard != 0)
	{
		attacks[7] |= Constants::KNIGHT_ATTACKS[ClearAndGetLSB(iterBitboard)];
	}

	// black bishops + queen
	iterBitboard = pieceBitboards[8] | pieceBitboards[10];
	while (iterBitboard != 0)
	{
		squareToCheck = ClearAndGetLSB(iterBitboard);
		moveMask = Constants::BISHOP_MASKS[squareToCheck];
		magicIndex = (((moveMask & allPieces) * Constants::BISHOP_MAGICS[squareToCheck]) >> 54);
		kingMagic = (((moveMask & (allPieces & ~pieceBitboards[5])) * Constants::BISHOP_MAGICS[squareToCheck]) >> 54);
		attacks[8] |= Constants::BISHOP_ATTACKS[squareToCheck][magicIndex];
		kingAttacks[0] |= Constants::BISHOP_ATTACKS[squareToCheck][kingMagic];
	}

	// black rooks + queen
	iterBitboard = pieceBitboards[9] | pieceBitboards[10];
	while (iterBitboard != 0)
	{
		squareToCheck = ClearAndGetLSB(iterBitboard);
		moveMask = Constants::ROOK_MASKS[squareToCheck];
		magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[squareToCheck]) >> 52);
		kingMagic = (((moveMask & (allPieces & ~pieceBitboards[5])) * Constants::ROOK_MAGICS[squareToCheck]) >> 52);
		attacks[9] |= Constants::ROOK_ATTACKS[squareToCheck][magicIndex];
		kingAttacks[0] |= Constants::ROOK_ATTACKS[squareToCheck][kingMagic];
	}

	// black king
	iterBitboard = pieceBitboards[11];
	while (iterBitboard != 0)
	{
		attacks[11] |= Constants::KING_ATTACKS[ClearAndGetLSB(iterBitboard)];
	}

	// update full attack boards
	colorAttacks[1] = 0;
	colorAttacks[0] = 0;
	for (int i = 0; i < 6; i++)
	{
		colorAttacks[1] |= attacks[i];
		colorAttacks[0] |= attacks[i + 6];
	}

	for (int i = 0; i < 2; i++)
	{
		kingAttacks[1] |= attacks[i];
		kingAttacks[0] |= attacks[i + 6];
	}

	kingAttacks[1] |= attacks[5];
	kingAttacks[0] |= attacks[11];
}

bool Board::isAttacked(int sqInd, bool white)
{
	return SquareIsSet(colorAttacks[white], sqInd);
}

bool Board::isInCheck()
{
	checkingPieces = 0;
	int bonus = (whiteToMove) * 6;
	ulong kingBoard = pieceBitboards[6 - bonus + 5];
	int kingSquare = ClearAndGetLSB(kingBoard);

	checkingPieces |= ((whiteToMove ? Constants::WHITE_PAWN_ATTACKS[kingSquare] : Constants::BLACK_PAWN_ATTACKS[kingSquare]) & pieceBitboards[bonus]);

	checkingPieces |= (Constants::KNIGHT_ATTACKS[kingSquare] & pieceBitboards[bonus + 1]);

	ulong mask = Constants::BISHOP_MASKS[kingSquare];
	int magicIndex = (((mask & allPieces) * Constants::BISHOP_MAGICS[kingSquare]) >> 54);
	checkingPieces |= (Constants::BISHOP_ATTACKS[kingSquare][magicIndex] & (pieceBitboards[bonus + 2] | pieceBitboards[bonus + 4]));

	mask = Constants::ROOK_MASKS[kingSquare];
	magicIndex = (((mask & allPieces) * Constants::ROOK_MAGICS[kingSquare]) >> 52);
	checkingPieces |= (Constants::ROOK_ATTACKS[kingSquare][magicIndex] & (pieceBitboards[bonus + 3] | pieceBitboards[bonus + 4]));

	numCheckers = NumSetBits(checkingPieces);

	return (numCheckers > 0);
}

// faster in search
bool Board::kingInCheck(bool white)
{
	ulong kingBoard = pieceBitboards[6 * (!white) + 5];
	return ((colorAttacks[!white] & kingBoard));
}

void Board::getPawnMoves(bool capsOnly)
{
	ulong pieceBitboard = pieceBitboards[6 - whiteToMove * 6];
	ulong thisStartSquare, thisTargetSquare, candidateCaptures;
	ulong candidateTargets = 0;
	ulong possibleSquares = pushMask | captureMask;

	toPlay.MovePieceType = 0;

	// deal with en passants first
	int candidatePassant = enPassantSquares.back();
	if ((candidatePassant != 64) && (SquareIsSet(possibleSquares, candidatePassant) || SquareIsSet(possibleSquares, (candidatePassant + (whiteToMove ? 8 : -8)))))
	{
		ulong enPassantBoard = pieceBitboard & (whiteToMove ? Constants::BLACK_PAWN_ATTACKS[candidatePassant] : Constants::WHITE_PAWN_ATTACKS[candidatePassant]);

		while (enPassantBoard != 0)
		{
			//std::cout << "I think there's an en passant move" << std::endl;

			thisStartSquare = ClearAndGetLSB(enPassantBoard);

			ulong pawnRankMask = 255ULL << ((thisStartSquare - (thisStartSquare % 8)));
			bool discoveredCheck = false;
			ulong kingBoard = pieceBitboards[whiteToMove ? 5 : 11];
			int kingSquare = ClearAndGetLSB(kingBoard);
			ulong checkerMask = 0;
			if ((pawnRankMask & enemyRooks) && (SquareIsSet(pawnRankMask, kingSquare)))
			{
				checkerMask = pawnRankMask & enemyRooks;
				//std::cout << std::bitset<64>(checkerMask) << std::endl;
				ulong directCheckMask = -1ULL;

				while (checkerMask != 0)
				{
					int rookSquare = ClearAndGetLSB(checkerMask);
					directCheckMask &= Constants::RAYS[kingSquare][rookSquare];
					//std::cout << std::bitset<64>(directCheckMask) << std::endl;
				}

				//std::cout << std::bitset<64>(directCheckMask) << std::endl;
				//std::cout << std::bitset<64>(allPieces) << std::endl;

				discoveredCheck = (NumSetBits(directCheckMask & allPieces) == 3);
			}

			//bool discoveredCheck = ((pawnRankMask & pieceBitboards[whiteToMove ? 5 : 11]) != 0) && (NumSetBits(pawnRankMask & allPieces) == (NumSetBits(pawnRankMask & enemyRooks) + 3));

			if (!discoveredCheck)
			{
				toPlay.moveCode = 0b0010000000000000;
				toPlay.moveCode |= thisStartSquare;
				toPlay.moveCode |= candidatePassant << 6;

				testMoves.push_back(toPlay);
			}
		}
	}

	while (pieceBitboard != 0)
	{
		bool promotion = false;

		thisStartSquare = ClearAndGetLSB(pieceBitboard);

		if (whiteToMove)
		{
			candidateCaptures = Constants::WHITE_PAWN_ATTACKS[thisStartSquare] & colorPieces[0] & possibleSquares;
			if (!(SquareIsSet(allPieces, thisStartSquare - 8) || capsOnly))
			{
				candidateTargets = Constants::WHITE_PAWN_FORWARDS[thisStartSquare] & ~allPieces & possibleSquares;
			}

			if (thisStartSquare < 16)
			{
				promotion = true;
			}
		}
		else
		{
			//std::cout << intToAlg(thisStartSquare) << std::endl;
			//std::cout << std::bitset<64>(Constants::BLACK_PAWN_ATTACKS[thisStartSquare] & colorPieces[1] & possibleSquares) << std::endl;

			candidateCaptures = Constants::BLACK_PAWN_ATTACKS[thisStartSquare] & colorPieces[1] & possibleSquares;
			if (!(SquareIsSet(allPieces, thisStartSquare + 8) || capsOnly))
			{
				candidateTargets = Constants::BLACK_PAWN_FORWARDS[thisStartSquare] & ~allPieces & possibleSquares;
			}

			if (thisStartSquare > 47)
			{
				promotion = true;
			}
		}

		// captures first
		while (candidateCaptures != 0)
		{
			toPlay.moveCode = thisStartSquare;
			toPlay.moveCode |= 0b0001000000000000;

			thisTargetSquare = ClearAndGetLSB(candidateCaptures);
			toPlay.moveCode |= thisTargetSquare << 6;

			toPlay.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 * whiteToMove) - 1 - 6 * whiteToMove;

			if (promotion)
			{
				for (int i = 7; i > 3; i--)
				{
					toPlay.moveCode &= 0b0001111111111111;
					toPlay.moveCode |= (i << 13);

					testMoves.push_back(toPlay);
				}
			} 
			else
			{
				testMoves.push_back(toPlay);
			}
		}

		// regular pawn moves
		while (candidateTargets != 0)
		{
			toPlay.moveCode = thisStartSquare;

			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toPlay.moveCode |= thisTargetSquare << 6;

			//std::cout << "\t" << intToAlg(thisTargetSquare) << std::endl;

			if ( (2 * whiteToMove - 1) * (thisStartSquare - thisTargetSquare) == 16)
			{
				// double pawn push
				toPlay.moveCode |= 0b0110000000000000;
				testMoves.push_back(toPlay);
			}
			else if (promotion)
			{
				for (int i = 7; i > 3; i--)
				{
					toPlay.moveCode &= 0b0000111111111111;
					toPlay.moveCode |= (i << 13);

					testMoves.push_back(toPlay);
				}
			} 
			else
			{
				testMoves.push_back(toPlay);
			}
		}
	}
}

void Board::getKnightMoves(bool capsOnly)
{
	ulong pieceBitboard = pieceBitboards[7 - whiteToMove * 6];
	ulong thisStartSquare, thisTargetSquare, candidateTargets;					 // = -1ULL when not in check
	ulong possibleSquares = capsOnly ? colorPieces[!whiteToMove] & captureMask : pushMask | captureMask;

	toPlay.MovePieceType = 1;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);

		candidateTargets = Constants::KNIGHT_ATTACKS[thisStartSquare] & ~colorPieces[whiteToMove] & possibleSquares;

		while (candidateTargets != 0)
		{
			toPlay.moveCode = thisStartSquare;

			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toPlay.moveCode |= thisTargetSquare << 6;

			// check if this is a capture
			if (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare))
			{
				toPlay.moveCode |= 0b0001000000000000;
				toPlay.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 * whiteToMove) - 1 - 6 * whiteToMove;
			}
			//toPlay.moveCode |= (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare) << 12);

			testMoves.push_back(toPlay);
		}
	}
}

void Board::getSlidingMoves(bool capsOnly)
{
	ulong pieceBitboard = pieceBitboards[8 - whiteToMove * 6] | pieceBitboards[10 - whiteToMove * 6];
	ulong thisStartSquare, thisTargetSquare, moveMask, magicIndex, candidateTargets;
	ulong possibleSquares = capsOnly ? colorPieces[!whiteToMove] & captureMask : pushMask | captureMask;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);

		moveMask = Constants::BISHOP_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::BISHOP_MAGICS[thisStartSquare]) >> 54);
		candidateTargets = Constants::BISHOP_ATTACKS[thisStartSquare][magicIndex] & ~colorPieces[whiteToMove] & possibleSquares;

		while (candidateTargets != 0)
		{
			toPlay.moveCode = thisStartSquare;

			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toPlay.moveCode |= thisTargetSquare << 6;

			// check if this is a capture
			if (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare))
			{
				toPlay.moveCode |= 0b0001000000000000;
				toPlay.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 * whiteToMove) - 1 - 6 * whiteToMove;
				toPlay.MovePieceType = getPieceTypeAtSquare(thisStartSquare, 6 * (!whiteToMove)) - 1 - 6 * (!whiteToMove);
			}
			//toPlay.moveCode |= (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare) << 12);

			testMoves.push_back(toPlay);
		}
	}

	pieceBitboard = pieceBitboards[9 - whiteToMove * 6] | pieceBitboards[10 - whiteToMove * 6];

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);

		moveMask = Constants::ROOK_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[thisStartSquare]) >> 52);
		candidateTargets = Constants::ROOK_ATTACKS[thisStartSquare][magicIndex] & ~colorPieces[whiteToMove] & possibleSquares;

		while (candidateTargets != 0)
		{
			toPlay.moveCode = thisStartSquare;

			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toPlay.moveCode |= thisTargetSquare << 6;

			// check if this is a capture
			if (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare))
			{
				toPlay.moveCode |= 0b0001000000000000;
				toPlay.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 * whiteToMove) - 1 - 6 * whiteToMove;
				toPlay.MovePieceType = getPieceTypeAtSquare(thisStartSquare, 6 * (!whiteToMove)) - 1 - 6 * (!whiteToMove);
			}
			//toPlay.moveCode |= (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare) << 12);

			testMoves.push_back(toPlay);
		}
	}

}

void Board::getKingMoves(bool capsOnly)
{
	toPlay.MovePieceType = 5;

	// castling moves first
	if (!(isInCheck() || capsOnly))
	{
		ushort rights = (castlingRights.back() & Constants::CASTLE_FLAG_MASKS[whiteToMove]) >> 2 * (!whiteToMove);

		if ((rights % 2) == 1) // checking for a 1 flag (kingside only) or a 3 flag (both)
		{
			ulong mask = Constants::CASTLE_MASKS[2 * (!whiteToMove)] & (allPieces | colorAttacks[!whiteToMove]);
			if (mask == 0)
			{
				toPlay.moveCode = 0b0100000000000000;
				int startingSquare = Constants::CASTLE_STARTS[whiteToMove];
				toPlay.moveCode |= startingSquare;
				toPlay.moveCode |= ((startingSquare + 2) << 6);
				testMoves.push_back(toPlay);
			}
		}

		if (rights > 1) // checking for a 2 flag (queenside only) or a 2 flag (both)
		{
			ulong mask = Constants::CASTLE_MASKS[1 + 2 * (!whiteToMove)];
			ulong maskCopy = mask;
			ClearAndGetLSB(maskCopy); // removes a square the king does not pass through before attack check
			mask &= allPieces;
			mask |= (maskCopy & colorAttacks[!whiteToMove]);
			if (mask == 0)
			{
				toPlay.moveCode = 0b0100000000000000;
				int startingSquare = Constants::CASTLE_STARTS[whiteToMove];
				toPlay.moveCode |= startingSquare;
				toPlay.moveCode |= ((startingSquare - 2) << 6);
				testMoves.push_back(toPlay);
			}
		}
	}

	ulong pieceBitboard = pieceBitboards[11 - whiteToMove * 6];
	ulong thisStartSquare, thisTargetSquare, candidateTargets;
	ulong possibleSquares = capsOnly ? colorPieces[!whiteToMove] : -1ULL;
	possibleSquares &= ~kingAttacks[!whiteToMove];

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		candidateTargets = Constants::KING_ATTACKS[thisStartSquare] & ~colorPieces[whiteToMove] & possibleSquares;

		while (candidateTargets != 0)
		{
			toPlay.moveCode = thisStartSquare;

			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toPlay.moveCode |= thisTargetSquare << 6;

			// check if this is a capture
			if (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare))
			{
				toPlay.moveCode |= 0b0001000000000000;
				toPlay.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 * whiteToMove) - 1 - 6 * whiteToMove;
			}
			//toPlay.moveCode |= (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare) << 12);

			testMoves.push_back(toPlay);
		}
	}
}

void Board::getPinnedMoves(bool capsOnly)
{
	ulong pinRay;
	int bonus = 6 * whiteToMove;
	ulong kingBoard = pieceBitboards[6 - bonus + 5];
	int kingSquare = ClearAndGetLSB(kingBoard);
	ulong sliders[2]; 
	sliders[0] = enemyBishops;
	sliders[1] = enemyRooks;
	ulong possibleSquares = capsOnly ? colorPieces[!whiteToMove] & captureMask : pushMask | captureMask;

	ulong candidateTargets = 0;
	ulong candidateCaptures = 0;
	ulong moveMask = 0;
	int thisStartSquare, thisTargetSquare, magicIndex;

	pinnedPawns = 0;
	pinnedKnights = 0;
	pinnedBishops = 0;
	pinnedRooks = 0;
	pinnedQueens = 0;

	//std::cout << "getting pinned moves!!" << std::endl;

	for (int i = 0; i < 2; i++)
	{
		while (sliders[i] != 0)
		{
			pinRay = Constants::RAYS[kingSquare][ClearAndGetLSB(sliders[i])] & ((i == 0) ? Constants::BISHOP_ATTACKS[kingSquare][0] : Constants::ROOK_ATTACKS[kingSquare][0]);

			ulong potentialPins = pinRay & allPieces;
			//std::cout << std::bitset<64>(potentialPins) << std::endl;
			bool realPin = (NumSetBits(potentialPins) - 1 == 1);
			//std::cout << std::bitset<64>(colorPieces[whiteToMove]) << std::endl;
			potentialPins &= colorPieces[whiteToMove];
			//std::cout << std::bitset<64>(potentialPins) << std::endl;
			bool hasPieces = (potentialPins != 0);
			int pinnedSquare = ClearAndGetLSB(potentialPins);
			thisStartSquare = pinnedSquare;

			//std::cout << thisStartSquare << std::endl;

			if (potentialPins == 0 && hasPieces && realPin)
			{
				//std::cout << "found a pinned piece of type " << getPieceTypeAtSquare(pinnedSquare, 6 - bonus) << " at square " << pinnedSquare << std::endl;
				possibleSquares = capsOnly ? colorPieces[!whiteToMove] & captureMask : pushMask | captureMask;

				// there was only one piece in the ray, it is actually pinned
				switch (getPieceTypeAtSquare(pinnedSquare, 6 - bonus))
				{
					// pawns (complicated)
					case 1:
					case 7:
					{
						toPlay.MovePieceType = 0;

						SetSquare(pinnedPawns, pinnedSquare);

						// annoying en passant things
						int candidatePassant = enPassantSquares.back();
						ulong testBoard = (whiteToMove ? Constants::BLACK_PAWN_ATTACKS[candidatePassant] : Constants::WHITE_PAWN_ATTACKS[candidatePassant]);
						if ((candidatePassant != 64) && (SquareIsSet(testBoard, pinnedSquare)) && (SquareIsSet(possibleSquares, candidatePassant) || SquareIsSet(possibleSquares, (candidatePassant + (whiteToMove ? 8 : -8)))) && SquareIsSet(pinRay, candidatePassant))
						{

							toPlay.moveCode = 0b0010000000000000;
							toPlay.moveCode |= pinnedSquare;
							toPlay.moveCode |= candidatePassant << 6;

							testMoves.push_back(toPlay);
						}

						possibleSquares &= pinRay;


						if (whiteToMove)
						{
							candidateCaptures = Constants::WHITE_PAWN_ATTACKS[thisStartSquare] & colorPieces[0] & possibleSquares;
							if (!(SquareIsSet(allPieces, thisStartSquare - 8) || capsOnly))
							{
								candidateTargets = Constants::WHITE_PAWN_FORWARDS[thisStartSquare] & ~allPieces & possibleSquares;
							}
						}
						else
						{
							candidateCaptures = Constants::BLACK_PAWN_ATTACKS[thisStartSquare] & colorPieces[1] & possibleSquares;
							if (!(SquareIsSet(allPieces, thisStartSquare + 8) || capsOnly))
							{
								candidateTargets = Constants::BLACK_PAWN_FORWARDS[thisStartSquare] & ~allPieces & possibleSquares;
							}
						}


						// only captures when checking against bishop type pins
						if (i == 0)
						{
							while (candidateCaptures != 0)
							{
								toPlay.moveCode = 0b0001000000000000;

								thisTargetSquare = ClearAndGetLSB(candidateCaptures);
								toPlay.moveCode |= thisTargetSquare << 6;
								toPlay.moveCode |= pinnedSquare;

								if (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare))

								toPlay.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 * whiteToMove) - 1 - 6 * whiteToMove;

								testMoves.push_back(toPlay);
							}
						}
						else 
						{
							while (candidateTargets != 0)
							{
								toPlay.moveCode = thisStartSquare;

								thisTargetSquare = ClearAndGetLSB(candidateTargets);
								toPlay.moveCode |= thisTargetSquare << 6;

								if ( (2 * whiteToMove - 1) * (thisStartSquare - thisTargetSquare) == 16)
								{
									// double pawn push
									toPlay.moveCode |= 0b0110000000000000;
									testMoves.push_back(toPlay);
								}
								else
								{
									testMoves.push_back(toPlay);
								}
							}
						}

						break;
					}

					// knights
					case 2:
					case 8:
					{
						SetSquare(pinnedKnights, pinnedSquare);
						break;
					}

					case 3:
					case 9:
					{	
						toPlay.MovePieceType = 2;

						SetSquare(pinnedBishops, pinnedSquare);
						possibleSquares &= pinRay;

						moveMask = Constants::BISHOP_MASKS[thisStartSquare];
						magicIndex = (((moveMask & allPieces) * Constants::BISHOP_MAGICS[thisStartSquare]) >> 54);
						candidateTargets = Constants::BISHOP_ATTACKS[thisStartSquare][magicIndex] & ~colorPieces[whiteToMove] & possibleSquares;

						while (candidateTargets != 0)
						{
							toPlay.moveCode = thisStartSquare;

							thisTargetSquare = ClearAndGetLSB(candidateTargets);
							toPlay.moveCode |= thisTargetSquare << 6;

							// check if this is a capture
							if (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare))
							{
								toPlay.moveCode |= 0b0001000000000000;
								toPlay.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 * whiteToMove) - 1 - 6 * whiteToMove;
							}
							//toPlay.moveCode |= (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare) << 12);

							testMoves.push_back(toPlay);
						}

						break;
					}

					case 4:
					case 10:
					{
						toPlay.MovePieceType = 3;

						SetSquare(pinnedRooks, pinnedSquare);
						possibleSquares &= pinRay;

						moveMask = Constants::ROOK_MASKS[thisStartSquare];
						magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[thisStartSquare]) >> 52);
						candidateTargets = Constants::ROOK_ATTACKS[thisStartSquare][magicIndex] & ~colorPieces[whiteToMove] & possibleSquares;

						while (candidateTargets != 0)
						{
							toPlay.moveCode = thisStartSquare;

							thisTargetSquare = ClearAndGetLSB(candidateTargets);
							toPlay.moveCode |= thisTargetSquare << 6;

							// check if this is a capture
							if (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare))
							{
								toPlay.moveCode |= 0b0001000000000000;
								toPlay.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 * whiteToMove) - 1 - 6 * whiteToMove;
							}
							//toPlay.moveCode |= (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare) << 12);

							testMoves.push_back(toPlay);
						}

						break;
					}

					case 5:
					case 11:
					{
						toPlay.MovePieceType = 4;

						SetSquare(pinnedQueens, pinnedSquare);
						possibleSquares &= pinRay;

						moveMask = Constants::BISHOP_MASKS[thisStartSquare];
						magicIndex = (((moveMask & allPieces) * Constants::BISHOP_MAGICS[thisStartSquare]) >> 54);
						candidateTargets = Constants::BISHOP_ATTACKS[thisStartSquare][magicIndex] & ~colorPieces[whiteToMove] & possibleSquares;

						moveMask = Constants::ROOK_MASKS[thisStartSquare];
						magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[thisStartSquare]) >> 52);
						candidateTargets |= Constants::ROOK_ATTACKS[thisStartSquare][magicIndex] & ~colorPieces[whiteToMove] & possibleSquares;

						while (candidateTargets != 0)
						{
							toPlay.moveCode = thisStartSquare;

							thisTargetSquare = ClearAndGetLSB(candidateTargets);
							toPlay.moveCode |= thisTargetSquare << 6;

							// check if this is a capture
							if (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare))
							{
								toPlay.moveCode |= 0b0001000000000000;
								toPlay.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 * whiteToMove) - 1 - 6 * whiteToMove;
							}
							//toPlay.moveCode |= (SquareIsSet(colorPieces[!whiteToMove], thisTargetSquare) << 12);

							testMoves.push_back(toPlay);
						}

						break;
					}
				}
			}
		}
	}

	//std::cout << pinnedPawns << std::endl;

	pieceBitboards[6 - bonus] &= ~pinnedPawns;
	pieceBitboards[7 - bonus] &= ~pinnedKnights;
	pieceBitboards[8 - bonus] &= ~pinnedBishops;
	pieceBitboards[9 - bonus] &= ~pinnedRooks;
	pieceBitboards[10 - bonus] &= ~pinnedQueens;

	//std::cout << pieceBitboards[6 - bonus] << std::endl;
}

std::vector<Move> Board::getLegalMoves(bool capsOnly)
{
	testMoves.clear();
	getKingMoves(capsOnly);

	if (!(isInCheck() && numCheckers > 1))
	{
		int bonus = 6 * whiteToMove;

		enemyBishops = pieceBitboards[2 + bonus] | pieceBitboards[4 + bonus];
		enemyRooks = pieceBitboards[3 + bonus] | pieceBitboards[4 + bonus];

		if (numCheckers == 1)
		{
			captureMask = checkingPieces;
			ulong kingBoard = pieceBitboards[6 - bonus + 5];
			pushMask = Constants::RAYS[ClearAndGetLSB(kingBoard)][ClearAndGetLSB(checkingPieces)];
		} 
		else 
		{
			captureMask = -1ULL;
			pushMask = -1ULL;
		}


		getPinnedMoves(capsOnly);
		getPawnMoves(capsOnly);
		getKnightMoves(capsOnly);
		getSlidingMoves(capsOnly);

		pieceBitboards[6 - bonus] |= pinnedPawns;
		pieceBitboards[7 - bonus] |= pinnedKnights;
		pieceBitboards[8 - bonus] |= pinnedBishops;
		pieceBitboards[9 - bonus] |= pinnedRooks;
		pieceBitboards[10 - bonus] |= pinnedQueens;
	}

	// above condition failed, so we are in double check
	// hence king moves found at top are the only ones available
	/*
	for (int i = 0; i < testMoves.size(); i++)
	{
		std::cout << testMoves.at(i) << std::endl;
	}*/

	if (testMoves.size() == 0)
	{
		if (kingInCheck(whiteToMove))
		{
			checkmate = 2 * whiteToMove - 1;
			gameOver = true;
		}
		else
		{
			stalemate = true;
			gameOver = true;
		}
	}

	return testMoves;
}

/*
std::vector<Move> Board::getPseudoLegalMoves(bool capsOnly)
{
	std::vector<Move> legalMoves;
	ulong pieceBitboard; 
	ulong candidateTargets = 0;
	ulong candidateCaptures;
	ulong moveMask;
	int magicIndex;
	ulong possibleSquares = -1ULL;

	int thisStartSquare;
	int thisTargetSquare;

	bool thisCapture;
	bool thisCastles;
	bool thisPromotion;
	bool thisEnPassant;
	bool thisDoublePawn;
	bool thisKingside;
	bool thisQueenside;

	int thisMovePieceType;
	int thisCapturePieceType;
	int thisPromotionPieceType;
	
	Move toBeAdded;
	toBeAdded.Reset();

	int indexBonus = 0;
	ulong myPieces = whitePieces;
	ulong enemyPieces = blackPieces;
	bool kingside = whiteKingside;
	bool queenside = whiteQueenside;
	bool genAsWhite = true;

	if (!whiteToMove)
	{
		indexBonus = 6;
		myPieces = blackPieces;
		enemyPieces = whitePieces;
		kingside = blackKingside;
		queenside = blackQueenside;
		genAsWhite = false;
	}

	if (capsOnly)
	{
		possibleSquares = enemyPieces;
	}

	// ---------------------- ADD PAWN MOVES ----------------------

	thisMovePieceType = indexBonus;
	toBeAdded.MovePieceType = thisMovePieceType;
	pieceBitboard = pieceBitboards[thisMovePieceType];

	// deal with en passants first
	if (enPassantSquare != 64)
	{
		//std::cout << "en passant available" << std::endl;
		if (whiteToMove)
		{
			pieceBitboard &= Constants::BLACK_PAWN_ATTACKS[enPassantSquare];
			toBeAdded.CapturePieceType = 7;
		}
		else 
		{
			pieceBitboard &= Constants::WHITE_PAWN_ATTACKS[enPassantSquare];
			toBeAdded.CapturePieceType = 1;
		}

		while (pieceBitboard != 0)
		{
			toBeAdded.IsCapture = true;
			toBeAdded.IsEnPassant = true;
			toBeAdded.StartSquare = ClearAndGetLSB(pieceBitboard);
			toBeAdded.TargetSquare = enPassantSquare;

			legalMoves.push_back(toBeAdded);
		}

		// reset capture status
		toBeAdded.IsCapture = false;
		toBeAdded.CapturePieceType = 0;

		// no more en passants
		toBeAdded.IsEnPassant = false;
	}

	pieceBitboard = pieceBitboards[thisMovePieceType];

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;

		if (whiteToMove)
		{
			candidateCaptures = Constants::WHITE_PAWN_ATTACKS[thisStartSquare] & blackPieces;
			if (!SquareIsSet(allPieces, thisStartSquare - 8))
			{
				candidateTargets = Constants::WHITE_PAWN_FORWARDS[thisStartSquare] & ~allPieces;
			}

			if (thisStartSquare < 16)
			{
				thisPromotion = true;
				toBeAdded.IsPromotion = true;
			}
		}
		else
		{
			candidateCaptures = Constants::BLACK_PAWN_ATTACKS[thisStartSquare] & whitePieces;
			if (!SquareIsSet(allPieces, thisStartSquare + 8))
			{
				candidateTargets = Constants::BLACK_PAWN_FORWARDS[thisStartSquare] & ~allPieces;
			}

			if (thisStartSquare > 47)
			{
				thisPromotion = true;
				toBeAdded.IsPromotion = true;
			}
		}

		while (candidateCaptures != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateCaptures);
			toBeAdded.TargetSquare = thisTargetSquare;
			toBeAdded.IsCapture = true;
			toBeAdded.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);

			if (thisPromotion)
			{
				for (int i = 0; i < 4; i++)
				{
					toBeAdded.PromotionPieceType = i + 2 + indexBonus;

					legalMoves.push_back(toBeAdded);
				}
			} 
			else
			{
				legalMoves.push_back(toBeAdded);
			}
		}

		// reset capture status
		toBeAdded.IsCapture = false;
		toBeAdded.CapturePieceType = 0;

		while (candidateTargets != 0 && !capsOnly)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			if (thisStartSquare - thisTargetSquare == 16 | thisStartSquare - thisTargetSquare == -16)
			{
				toBeAdded.IsDoublePawnPush = true;
			}

			if (thisPromotion)
			{
				for (int i = 0; i < 4; i++)
				{
					toBeAdded.PromotionPieceType = i + 2 + indexBonus;
					legalMoves.push_back(toBeAdded);
				}
			} 
			else
			{
				legalMoves.push_back(toBeAdded);
			}

			toBeAdded.IsDoublePawnPush = false;
		}

		thisPromotion = false;
		toBeAdded.IsPromotion = false;
		toBeAdded.PromotionPieceType = 0;
	}

	// we're done adding pawn moves, there won't be anymore promotions
	toBeAdded.PromotionPieceType = 0;

	// ---------------------- ADD KNIGHT MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;
		candidateTargets = Constants::KNIGHT_ATTACKS[thisStartSquare] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);

			legalMoves.push_back(toBeAdded);
		}
	}

	// ---------------------- ADD BISHOP MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;

		moveMask = Constants::BISHOP_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::BISHOP_MAGICS[thisStartSquare]) >> 54);
		candidateTargets = Constants::BISHOP_ATTACKS[thisStartSquare][magicIndex] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);

			legalMoves.push_back(toBeAdded);
		}
	}

	// ---------------------- ADD ROOK MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;

		toBeAdded.LosesKingside = (kingside && (thisStartSquare % 8) == 7);
		toBeAdded.LosesQueenside = (queenside && (thisStartSquare % 8) == 0);

		moveMask = Constants::ROOK_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[thisStartSquare]) >> 52);
		candidateTargets = Constants::ROOK_ATTACKS[thisStartSquare][magicIndex] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);

			legalMoves.push_back(toBeAdded);		}
	}

	// reset castling rights flags for now
	toBeAdded.LosesKingside = false;
	toBeAdded.LosesQueenside = false;

	// ---------------------- ADD QUEEN MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;

		moveMask = Constants::BISHOP_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::BISHOP_MAGICS[thisStartSquare]) >> 54);
		candidateTargets = Constants::BISHOP_ATTACKS[thisStartSquare][magicIndex] & ~myPieces;

		moveMask = Constants::ROOK_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[thisStartSquare]) >> 52);
		candidateTargets |= Constants::ROOK_ATTACKS[thisStartSquare][magicIndex] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);

			legalMoves.push_back(toBeAdded);
		}
	}	

	// ---------------------- ADD KING MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;
	toBeAdded.LosesKingside = true;
	toBeAdded.LosesQueenside = true;
	toBeAdded.IsCapture = false;
	toBeAdded.CapturePieceType = 0;

	// add castles
	if (kingside) 
	{
		ulong castleMask = Constants::CASTLE_MASKS[indexBonus / 3];
		if (((castleMask & allPieces) == 0) && !isInCheck())
		{
			bool addCastle = true;
			while (castleMask != 0)
			{
				if (isAttacked(ClearAndGetLSB(castleMask), !whiteToMove))
				{
					addCastle = false;
				}
			}

			if (addCastle)
			{
				toBeAdded.IsCastles = true;
				toBeAdded.StartSquare = Constants::CASTLE_STARTS[indexBonus / 6];
				toBeAdded.TargetSquare = toBeAdded.StartSquare + 2;

				toBeAdded.LosesKingside = true;
				toBeAdded.LosesQueenside = queenside;

				legalMoves.push_back(toBeAdded);

				toBeAdded.IsCastles = false;
			}
		}
	}

	if (queenside)
	{
		ulong castleMask = Constants::CASTLE_MASKS[(indexBonus / 3) + 1];
		if (((castleMask & allPieces) == 0) && !isInCheck()) // check blockers for all three bits
		{
			bool addCastle = true;
			ClearAndGetLSB(castleMask); // clear the first bit, no need to check for checks
			while (castleMask != 0)
			{
				if (isAttacked(ClearAndGetLSB(castleMask), !whiteToMove))
				{
					addCastle = false;
				}
			}

			if (addCastle)
			{
				toBeAdded.IsCastles = true;
				toBeAdded.StartSquare = Constants::CASTLE_STARTS[indexBonus / 6];
				toBeAdded.TargetSquare = toBeAdded.StartSquare - 2;

				toBeAdded.LosesKingside = kingside;
				toBeAdded.LosesQueenside = true;

				legalMoves.push_back(toBeAdded);

				toBeAdded.IsCastles = false;
			}
		}
	}

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;
		candidateTargets = Constants::KING_ATTACKS[thisStartSquare] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);
			if (thisCapturePieceType + indexBonus == 9)
			{
				// A ROOK IS BEING CAPTURED, COULD REVOKE CASTLING RIGHTS
				// CHANGE FLAG FOR CASTLING RIGHTS TO HANDLE OURS + OPPONENT'S3
			}

			// only want these to be true if we haven't already lost castling rights
			toBeAdded.LosesKingside = kingside;
			toBeAdded.LosesQueenside = queenside;

			bool inCheck = false;

			for (int i = 0; i < 6; i++)
			{
				if (SquareIsSet(attacks[i + 6 - indexBonus], thisTargetSquare))
				{
					inCheck = true;
				}
			}

			if (!inCheck)
			{
				legalMoves.push_back(toBeAdded);
			}
		}
	}


	return legalMoves;
}

std::vector<Move> Board::oldLegalMoveGen(bool capsOnly)
{
	std::vector<Move> legalMoves;
	ulong pieceBitboard; 
	ulong candidateTargets = 0;
	ulong candidateCaptures;
	ulong moveMask;
	int magicIndex;
	ulong possibleSquares = -1ULL;

	int thisStartSquare;
	int thisTargetSquare;

	bool thisCapture;
	bool thisCastles;
	bool thisPromotion;
	bool thisEnPassant;
	bool thisDoublePawn;
	bool thisKingside;
	bool thisQueenside;

	int thisMovePieceType;
	int thisCapturePieceType;
	int thisPromotionPieceType;
	
	Move toBeAdded;
	toBeAdded.Reset();

	int indexBonus = 0;
	ulong myPieces = whitePieces;
	ulong enemyPieces = blackPieces;
	bool kingside = whiteKingside;
	bool queenside = whiteQueenside;
	bool genAsWhite = true;

	if (!whiteToMove)
	{
		indexBonus = 6;
		myPieces = blackPieces;
		enemyPieces = whitePieces;
		kingside = blackKingside;
		queenside = blackQueenside;
		genAsWhite = false;
	}

	if (capsOnly)
	{
		possibleSquares = enemyPieces;
	}

	// ---------------------- ADD PAWN MOVES ----------------------

	thisMovePieceType = indexBonus;
	toBeAdded.MovePieceType = thisMovePieceType;
	pieceBitboard = pieceBitboards[thisMovePieceType];

	// deal with en passants first
	if (enPassantSquare != 64)
	{
		//std::cout << "en passant available" << std::endl;
		if (whiteToMove)
		{
			pieceBitboard &= Constants::BLACK_PAWN_ATTACKS[enPassantSquare];
			toBeAdded.CapturePieceType = 7;
		}
		else 
		{
			pieceBitboard &= Constants::WHITE_PAWN_ATTACKS[enPassantSquare];
			toBeAdded.CapturePieceType = 1;
		}

		while (pieceBitboard != 0)
		{
			toBeAdded.IsCapture = true;
			toBeAdded.IsEnPassant = true;
			toBeAdded.StartSquare = ClearAndGetLSB(pieceBitboard);
			toBeAdded.TargetSquare = enPassantSquare;

			MakeMove(toBeAdded);
			if (!(kingInCheck(genAsWhite)))
			{
				legalMoves.push_back(toBeAdded);
			}
			UndoMove(toBeAdded);
			//legalMoves.push_back(toBeAdded);
		}

		// reset capture status
		toBeAdded.IsCapture = false;
		toBeAdded.CapturePieceType = 0;

		// no more en passants
		toBeAdded.IsEnPassant = false;
	}

	pieceBitboard = pieceBitboards[thisMovePieceType];

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;

		if (whiteToMove)
		{
			candidateCaptures = Constants::WHITE_PAWN_ATTACKS[thisStartSquare] & blackPieces;
			if (!SquareIsSet(allPieces, thisStartSquare - 8))
			{
				candidateTargets = Constants::WHITE_PAWN_FORWARDS[thisStartSquare] & ~allPieces;
			}

			if (thisStartSquare < 16)
			{
				thisPromotion = true;
				toBeAdded.IsPromotion = true;
			}
		}
		else
		{
			candidateCaptures = Constants::BLACK_PAWN_ATTACKS[thisStartSquare] & whitePieces;
			if (!SquareIsSet(allPieces, thisStartSquare + 8))
			{
				candidateTargets = Constants::BLACK_PAWN_FORWARDS[thisStartSquare] & ~allPieces;
			}

			if (thisStartSquare > 47)
			{
				thisPromotion = true;
				toBeAdded.IsPromotion = true;
			}
		}

		while (candidateCaptures != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateCaptures);
			toBeAdded.TargetSquare = thisTargetSquare;
			toBeAdded.IsCapture = true;
			toBeAdded.CapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);

			if (thisPromotion)
			{
				for (int i = 0; i < 4; i++)
				{
					toBeAdded.PromotionPieceType = i + 2 + indexBonus;
					MakeMove(toBeAdded);
					if (!(kingInCheck(genAsWhite)))
					{
						legalMoves.push_back(toBeAdded);
					}
					UndoMove(toBeAdded);
				}
			} 
			else
			{
				MakeMove(toBeAdded);
				if (!(kingInCheck(genAsWhite)))
				{
					legalMoves.push_back(toBeAdded);
				}
				UndoMove(toBeAdded);
			}
		}

		// reset capture status
		toBeAdded.IsCapture = false;
		toBeAdded.CapturePieceType = 0;

		while (candidateTargets != 0 && !capsOnly)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			if (thisStartSquare - thisTargetSquare == 16 | thisStartSquare - thisTargetSquare == -16)
			{
				toBeAdded.IsDoublePawnPush = true;
			}

			if (thisPromotion)
			{
				for (int i = 0; i < 4; i++)
				{
					toBeAdded.PromotionPieceType = i + 2 + indexBonus;
					MakeMove(toBeAdded);
					if (!(kingInCheck(genAsWhite)))
					{
						legalMoves.push_back(toBeAdded);
					}
					UndoMove(toBeAdded);
				}
			} 
			else
			{
				MakeMove(toBeAdded);
				if (!(kingInCheck(genAsWhite)))
				{
					legalMoves.push_back(toBeAdded);
				}
				UndoMove(toBeAdded);
			}

			toBeAdded.IsDoublePawnPush = false;
		}

		thisPromotion = false;
		toBeAdded.IsPromotion = false;
		toBeAdded.PromotionPieceType = 0;
	}

	// we're done adding pawn moves, there won't be anymore promotions
	toBeAdded.PromotionPieceType = 0;

	// ---------------------- ADD KNIGHT MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;
		candidateTargets = Constants::KNIGHT_ATTACKS[thisStartSquare] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);

			MakeMove(toBeAdded);
			if (!(kingInCheck(genAsWhite)))
			{
				legalMoves.push_back(toBeAdded);
			}
			UndoMove(toBeAdded);
		}
	}

	// ---------------------- ADD BISHOP MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;

		moveMask = Constants::BISHOP_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::BISHOP_MAGICS[thisStartSquare]) >> 54);
		candidateTargets = Constants::BISHOP_ATTACKS[thisStartSquare][magicIndex] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);

			MakeMove(toBeAdded);
			if (!(kingInCheck(genAsWhite)))
			{
				legalMoves.push_back(toBeAdded);
			}
			UndoMove(toBeAdded);
		}
	}

	// ---------------------- ADD ROOK MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;

		toBeAdded.LosesKingside = (kingside && (thisStartSquare % 8) == 7);
		toBeAdded.LosesQueenside = (queenside && (thisStartSquare % 8) == 0);

		moveMask = Constants::ROOK_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[thisStartSquare]) >> 52);
		candidateTargets = Constants::ROOK_ATTACKS[thisStartSquare][magicIndex] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);

			MakeMove(toBeAdded);
			if (!(kingInCheck(genAsWhite)))
			{
				legalMoves.push_back(toBeAdded);
			}
			UndoMove(toBeAdded);		}
	}

	// reset castling rights flags for now
	toBeAdded.LosesKingside = false;
	toBeAdded.LosesQueenside = false;

	// ---------------------- ADD QUEEN MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;

		moveMask = Constants::BISHOP_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::BISHOP_MAGICS[thisStartSquare]) >> 54);
		candidateTargets = Constants::BISHOP_ATTACKS[thisStartSquare][magicIndex] & ~myPieces;

		moveMask = Constants::ROOK_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[thisStartSquare]) >> 52);
		candidateTargets |= Constants::ROOK_ATTACKS[thisStartSquare][magicIndex] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);

			MakeMove(toBeAdded);
			if (!(kingInCheck(genAsWhite)))
			{
				legalMoves.push_back(toBeAdded);
			}
			UndoMove(toBeAdded);
		}
	}	

	// ---------------------- ADD KING MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;
	toBeAdded.IsCapture = false;
	toBeAdded.CapturePieceType = 0;

	// add castles
	if (kingside) 
	{
		ulong castleMask = Constants::CASTLE_MASKS[indexBonus / 3];
		if (((castleMask & allPieces) == 0) && !isInCheck())
		{
			bool addCastle = true;
			while (castleMask != 0)
			{
				if (isAttacked(ClearAndGetLSB(castleMask), !whiteToMove))
				{
					addCastle = false;
				}
			}

			if (addCastle)
			{
				toBeAdded.IsCastles = true;
				toBeAdded.StartSquare = Constants::CASTLE_STARTS[indexBonus / 6];
				toBeAdded.TargetSquare = toBeAdded.StartSquare + 2;

				toBeAdded.LosesKingside = true; 	    // certainly we may currently kingside castle
				toBeAdded.LosesQueenside = queenside;	// but may already have lost queenside rights

				legalMoves.push_back(toBeAdded);

				toBeAdded.IsCastles = false;
			}
		}
	}

	if (queenside)
	{
		ulong castleMask = Constants::CASTLE_MASKS[(indexBonus / 3) + 1];
		if (((castleMask & allPieces) == 0) && !isInCheck())
		{
			bool addCastle = true;
			ClearAndGetLSB(castleMask); // clear the first bit, no need to check for checks
			while (castleMask != 0)
			{
				if (isAttacked(ClearAndGetLSB(castleMask), !whiteToMove))
				{
					addCastle = false;
				}
			}

			if (addCastle)
			{
				toBeAdded.IsCastles = true;
				toBeAdded.StartSquare = Constants::CASTLE_STARTS[indexBonus / 6];
				toBeAdded.TargetSquare = toBeAdded.StartSquare - 2;

				toBeAdded.LosesKingside = kingside;
				toBeAdded.LosesQueenside = true;

				legalMoves.push_back(toBeAdded);

				toBeAdded.IsCastles = false;
			}
		}
	}

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;
		candidateTargets = Constants::KING_ATTACKS[thisStartSquare] & ~myPieces & possibleSquares;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			thisCapturePieceType = getPieceTypeAtSquare(thisTargetSquare, 6 - indexBonus);
			toBeAdded.CapturePieceType = thisCapturePieceType;
			toBeAdded.IsCapture = (thisCapturePieceType != 0);

			// only want these to be true if we haven't already lost castling rights
			toBeAdded.LosesKingside = kingside;
			toBeAdded.LosesQueenside = queenside;

			bool inCheck = false;

			for (int i = 0; i < 6; i++)
			{
				if (SquareIsSet(attacks[i + 6 - indexBonus], thisTargetSquare))
				{
					inCheck = true;
				}
			}

			if (!inCheck)
			{
				legalMoves.push_back(toBeAdded);
			}
		}
	}


	return legalMoves;
}*/

void Board::MovePiece(int type, int start, int end)
{
	SetSquare(allPieces, end);
	ClearSquare(allPieces, start);

	SetSquare(colorPieces[whiteToMove], end);
	ClearSquare(colorPieces[whiteToMove], start);

	SetSquare(pieceBitboards[type], end);
	ClearSquare(pieceBitboards[type], start);

	// update Zobrist hash
	zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[type][start];
	zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[type][end];

	//std::cout << "changing Zobrist -- piece movement" << std::endl;
}

void Board::MakeMove(Move toMake)
{
	// get relevant info
	int bonus = whiteToMove * 6;
	int StartSquare = toMake.getStartSquare();
	int TargetSquare = toMake.getTargetSquare();

	int MovePieceType = getPieceTypeAtSquare(StartSquare, 6 - bonus);
	int CapturePieceType = 0;

	moveCountsToFifty.push_back(moveCountsToFifty.back() + 1); // we will reset it for pawn moves and captures later

	gamePhases.push_back(gamePhases.back());

	// get ready for a new Zobrist hash
	zobristHistory.push_back(zobristHistory.back());

	// castling rights in Zobrist hash -- get the old rights
	zobristHistory.back() ^= Constants::CASTLE_ZOBRISTS[castlingRights.back()];
	//std::cout << "changing Zobrist -- old castling rights" << std::endl;

	// get the old en passant file in Zobrist hash
	if (enPassantSquares.back() != 64) 
	{
		zobristHistory.back() ^= Constants::EN_PASSANT_FILE_ZOBRISTS[enPassantSquares.back() % 8];
		//std::cout << "changing Zobrist -- old en passant file" << std::endl;
	}

	if (MovePieceType == 6)
	{
		castlingRights.push_back(castlingRights.back() & 0b1100);
	}
	else if (MovePieceType == 12) 
	{
		castlingRights.push_back(castlingRights.back() & 0b0011);
	}
	else 
	{
		castlingRights.push_back(castlingRights.back());
	}

	MovePiece(MovePieceType - 1, StartSquare, TargetSquare);

	if (MovePieceType == 1 || MovePieceType == 7)
	{
		moveCountsToFifty.back() = 0;
	}

	if (toMake.isCapture())
	{
		moveCountsToFifty.back() = 0;

		CapturePieceType = getPieceTypeAtSquare(TargetSquare, bonus);

		ClearSquare(colorPieces[!whiteToMove], TargetSquare);
		ClearSquare(pieceBitboards[CapturePieceType - 1], TargetSquare);

		capturedPieces.push_back(CapturePieceType);

		// update Zobrist hash
		zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[CapturePieceType - 1][TargetSquare];
		//std::cout << "changing Zobrist -- capture" << std::endl;

		gamePhases.back() -= Constants::PIECE_PHASES[CapturePieceType - 1];
	}
	else 
	{
		capturedPieces.push_back(0);
	}

	if (toMake.isEnPassant())
	{
		int shift = whiteToMove ? 8 : -8;

		CapturePieceType = bonus + 1;

		ClearSquare(allPieces, TargetSquare + shift);
		ClearSquare(colorPieces[!whiteToMove], TargetSquare + shift);
		ClearSquare(pieceBitboards[bonus], TargetSquare + shift);

		// update Zobrist hash
		zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[CapturePieceType][TargetSquare + shift];
		//std::cout << "changing Zobrist -- en passant capture" << std::endl;
	}

	if (toMake.isDoublePawnPush())
	{
		enPassantSquares.push_back(StartSquare - (whiteToMove ? 8 : -8));
	}
	else
	{
		enPassantSquares.push_back(64);
	}

	if (toMake.isPromotion())
	{
		int promoteType = toMake.promotionPieceType(whiteToMove);

		ClearSquare(pieceBitboards[6 - bonus], TargetSquare);
		SetSquare(pieceBitboards[promoteType], TargetSquare);

		// update Zobrist hash
		zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[6 - bonus][TargetSquare];
		zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[promoteType][TargetSquare];

		//std::cout << "changing Zobrist -- promotion" << std::endl;

		gamePhases.back() += Constants::PIECE_PHASES[promoteType];
	}

	if (toMake.isCastles())
	{
		//std::cout << "changing Zobrist -- castling" << std::endl;
		if (TargetSquare == 62)
		{
			ClearSquare(pieceBitboards[3], 63); // remove rook at h1
			ClearSquare(colorPieces[1], 63); 
			ClearSquare(allPieces, 63); 
			SetSquare(pieceBitboards[3], 61);   // and place at f1
			SetSquare(colorPieces[1], 61);
			SetSquare(allPieces, 61);

			// update Zobrist hash
			zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[3][63];
			zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[3][61];
		}
		else if (TargetSquare == 58)
		{
			ClearSquare(pieceBitboards[3], 56); // remove rook at a1
			ClearSquare(colorPieces[1], 56);
			ClearSquare(allPieces, 56);
			SetSquare(pieceBitboards[3], 59);   // and place at d1
			SetSquare(colorPieces[1], 59);
			SetSquare(allPieces, 59);

			// update Zobrist hash
			zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[3][56];
			zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[3][59];
		}
		else if (TargetSquare == 6)
		{
			ClearSquare(pieceBitboards[9], 7); // remove rook at h8
			ClearSquare(colorPieces[0], 7); 
			ClearSquare(allPieces, 7); 
			SetSquare(pieceBitboards[9], 5);   // and place at f8
			SetSquare(colorPieces[0], 5); 
			SetSquare(allPieces, 5); 

			// update Zobrist hash
			zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[3][7];
			zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[3][5];
		}
		else 
		{
			ClearSquare(pieceBitboards[9], 0); // remove rook at a8
			ClearSquare(colorPieces[0], 0); 
			ClearSquare(allPieces, 0); 
			SetSquare(pieceBitboards[9], 3);   // and place at d8
			SetSquare(colorPieces[0], 3); 
			SetSquare(allPieces, 3); 

			// update Zobrist hash
			zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[3][0];
			zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[3][3];
		}
	}

	if (castlingRights.back() != 0)
	{
		// white 
		if (StartSquare == 63 || TargetSquare == 63)
		{
			castlingRights.back() &= 0b1110;
		}
		else if (StartSquare == 56 || TargetSquare == 56)
		{
			castlingRights.back() &= 0b1101;
		}

		// black
		if (StartSquare == 7 || TargetSquare == 7)
		{
			castlingRights.back() &= 0b1011;
		}
		else if (StartSquare == 0 || TargetSquare == 0)
		{
			castlingRights.back() &= 0b0111;
		}
	}

	// castling rights in Zobrist hash -- get the new rights
	zobristHistory.back() ^= Constants::CASTLE_ZOBRISTS[castlingRights.back()];
	//std::cout << "changing Zobrist -- new castling rights" << std::endl;

	// get the new en passant file in Zobrist hash
	if (enPassantSquares.back() != 64) 
	{
		zobristHistory.back() ^= Constants::EN_PASSANT_FILE_ZOBRISTS[enPassantSquares.back() % 8];
		//std::cout << "changing Zobrist -- new en passant file" << std::endl;
	}

	//std::cout << StartSquare << " " << TargetSquare << " " << MovePieceType << " " << CapturePieceType << std::endl;
	updateAttacks();
	moveHistory.push_back(toMake);
	whiteToMove = !whiteToMove;

	// side to move Zobrist hash
	zobristHistory.back() ^= Constants::WHITE_ZOBRIST;
	//std::cout << "changing Zobrist -- side to move" << std::endl;

	// add position to repetition table
	//repetitionTable[zobristHistory.back() & 16383]++;

	/*if (repetitionTable[zobristHistory.back() & 16383] == 3) 
	{
		threefoldRep = true;
		gameOver = true;
	}*/

	if (moveCountsToFifty.back() == 50) 
	{
		fiftyMoves = true;
		gameOver = true;
	}

	if (gamePhases.back() < 2 && pieceBitboards[0] == 0 && pieceBitboards[6] == 0) 
	{
		insufficientMaterial = true;
		gameOver = true;
	}

	/*std::cout << "----" << std::endl;
	for (int i = 0; i < moveHistory.size(); i++)
	{
		std::cout << moveHistory.at(i) << " " << repetitionTable[zobristHistory.at(i) & 16383] << std::endl;
	}*/
}

void Board::Undo()
{
	if (moveHistory.size() > 0)
	{
		Move toUndo = moveHistory.back();
		whiteToMove = !whiteToMove;

		int bonus = whiteToMove * 6;
		int StartSquare = toUndo.getStartSquare();
		int TargetSquare = toUndo.getTargetSquare();

		int MovePieceType = getPieceTypeAtSquare(TargetSquare, 6 - bonus);
		int CapturePieceType = capturedPieces.back();

		MovePiece(MovePieceType - 1, TargetSquare, StartSquare);

		if (toUndo.isCapture())
		{
			SetSquare(allPieces, TargetSquare);
			SetSquare(colorPieces[!whiteToMove], TargetSquare);
			SetSquare(pieceBitboards[CapturePieceType - 1], TargetSquare);
		}

		if (toUndo.isEnPassant())
		{
			int shift = whiteToMove ? 8 : -8;

			SetSquare(allPieces, TargetSquare + shift);
			SetSquare(colorPieces[!whiteToMove], TargetSquare + shift);
			SetSquare(pieceBitboards[bonus], TargetSquare + shift);
		}

		if (toUndo.isPromotion())
		{
			ClearSquare(pieceBitboards[toUndo.promotionPieceType(whiteToMove)], TargetSquare);
			ClearSquare(pieceBitboards[toUndo.promotionPieceType(whiteToMove)], StartSquare);
			SetSquare(pieceBitboards[6 - bonus], StartSquare);
		}

		if (toUndo.isCastles())
		{
			if (TargetSquare == 62)
			{
				SetSquare(pieceBitboards[3], 63); // place rook at h1
				SetSquare(colorPieces[1], 63);
				SetSquare(allPieces, 63);
				ClearSquare(pieceBitboards[3], 61);   // remove at f1
				ClearSquare(colorPieces[1], 61);
				ClearSquare(allPieces, 61);
			}
			else if (TargetSquare == 58)
			{
				// queenside castling
				SetSquare(pieceBitboards[3], 56); // place rook at a1
				SetSquare(colorPieces[1], 56);
				SetSquare(allPieces, 56);
				ClearSquare(pieceBitboards[3], 59);   // remove at d1
				ClearSquare(colorPieces[1], 59);
				ClearSquare(allPieces, 59);
			}
			else if (TargetSquare == 6)
			{
				SetSquare(pieceBitboards[9], 7); // place rook at h8
				SetSquare(colorPieces[0], 7);
				SetSquare(allPieces, 7);
				ClearSquare(pieceBitboards[9], 5);   // and remove at f8
				ClearSquare(colorPieces[0], 5);
				ClearSquare(allPieces, 5);
			}
			else 
			{
				SetSquare(pieceBitboards[9], 0); // place rook at a8
				SetSquare(colorPieces[0], 0);
				SetSquare(allPieces, 0);
				ClearSquare(pieceBitboards[9], 3);   // and remove at d8
				ClearSquare(colorPieces[0], 3);
				ClearSquare(allPieces, 3);
			}
		}

		castlingRights.pop_back();
		enPassantSquares.pop_back();
		capturedPieces.pop_back();
		moveHistory.pop_back();
		moveCountsToFifty.pop_back();
		gamePhases.pop_back();

		gameOver = false;
		checkmate = 0;
		stalemate = false;
		insufficientMaterial = false;
		fiftyMoves = false;

		/*// before popping end of zobrist history, decrement position counter in repetition table
		repetitionTable[zobristHistory.back() & 16383]--;
		std::cout << repetitionTable[zobristHistory.back() & 16383] << std::endl;
		zobristHistory.pop_back();*/

		//std::cout << gamePhases.back() << std::endl;
	}

	updateAttacks();
}

/*
void Board::OldMake(Move toPlay)
{
	//std::cout << allPieces << std::endl;
	SetSquare(allPieces, toPlay.TargetSquare);
	ClearSquare(allPieces, toPlay.StartSquare);

	SetSquare(pieceBitboards[toPlay.MovePieceType], toPlay.TargetSquare);
	ClearSquare(pieceBitboards[toPlay.MovePieceType], toPlay.StartSquare);

	if (whiteToMove)
	{
		SetSquare(whitePieces, toPlay.TargetSquare);
		ClearSquare(whitePieces, toPlay.StartSquare);

		if (toPlay.IsCapture)
		{
			ClearSquare(blackPieces, toPlay.TargetSquare);
			ClearSquare(pieceBitboards[toPlay.CapturePieceType - 1], toPlay.TargetSquare);
		} 
		
		if (toPlay.IsEnPassant)
		{
			ClearSquare(allPieces, toPlay.TargetSquare + 8);
			ClearSquare(blackPieces, toPlay.TargetSquare + 8);
			ClearSquare(pieceBitboards[6], toPlay.TargetSquare + 8);
		}

		if (toPlay.IsDoublePawnPush)
		{
			enPassantSquare = toPlay.StartSquare - 8;
		}
		else
		{
			enPassantSquare = 64;
		}

		if (toPlay.IsPromotion)
		{
			ClearSquare(pieceBitboards[0], toPlay.TargetSquare);
			SetSquare(pieceBitboards[toPlay.PromotionPieceType - 1], toPlay.TargetSquare);
		}

		if (toPlay.LosesKingside)
		{
			whiteKingside = false;
		}

		if (toPlay.LosesQueenside)
		{
			whiteQueenside = false;
		}

		if (toPlay.IsCastles)
		{
			if (toPlay.TargetSquare == 62)
			{
				// kingside castling
				ClearSquare(pieceBitboards[3], 63); // remove rook at h1
				ClearSquare(whitePieces, 63); 
				ClearSquare(allPieces, 63); 
				SetSquare(pieceBitboards[3], 61);   // and place at f1
				SetSquare(whitePieces, 61);
				SetSquare(allPieces, 61);
			}
			else 
			{
				// queenside castling
				ClearSquare(pieceBitboards[3], 56); // remove rook at a1
				ClearSquare(whitePieces, 56);
				ClearSquare(allPieces, 56);
				SetSquare(pieceBitboards[3], 59);   // and place at d1
				SetSquare(whitePieces, 59);
				SetSquare(allPieces, 59);
			}
		}
	} 
	else 
	{
		SetSquare(blackPieces, toPlay.TargetSquare);
		ClearSquare(blackPieces, toPlay.StartSquare);

		if (toPlay.IsCapture)
		{
			ClearSquare(whitePieces, toPlay.TargetSquare);
			ClearSquare(pieceBitboards[toPlay.CapturePieceType - 1], toPlay.TargetSquare);
		}
		
		if (toPlay.IsEnPassant)
		{
			ClearSquare(allPieces, toPlay.TargetSquare - 8);
			ClearSquare(whitePieces, toPlay.TargetSquare - 8);
			ClearSquare(pieceBitboards[0], toPlay.TargetSquare - 8);
		}

		if (toPlay.IsDoublePawnPush)
		{
			enPassantSquare = toPlay.StartSquare + 8;
		}
		else
		{
			enPassantSquare = 64;
		}

		if (toPlay.IsPromotion)
		{
			ClearSquare(pieceBitboards[6], toPlay.TargetSquare);
			SetSquare(pieceBitboards[toPlay.PromotionPieceType - 1], toPlay.TargetSquare);
		}

		if (toPlay.LosesKingside)
		{
			blackKingside = false;
		}

		if (toPlay.LosesQueenside)
		{
			blackQueenside = false;
		}

		if (toPlay.IsCastles)
		{
			if (toPlay.TargetSquare == 6)
			{
				// kingside castling
				ClearSquare(pieceBitboards[9], 7); // remove rook at h8
				ClearSquare(blackPieces, 7); 
				ClearSquare(allPieces, 7); 
				SetSquare(pieceBitboards[9], 5);   // and place at f8
				SetSquare(blackPieces, 5); 
				SetSquare(allPieces, 5); 
			}
			else 
			{
				// queenside castling
				ClearSquare(pieceBitboards[9], 0); // remove rook at a8
				ClearSquare(blackPieces, 0); 
				ClearSquare(allPieces, 0); 
				SetSquare(pieceBitboards[9], 3);   // and place at d8
				SetSquare(blackPieces, 3); 
				SetSquare(allPieces, 3); 
			}
		}
	}

	updateAttacks();
	whiteToMove = !whiteToMove;
}

void Board::UndoMove(Move toUndo)
{
	//std::cout << "undoing " << toUndo << std::endl;
	//std::cout << toUndo.MovePieceType << std::endl;

	if (toUndo.StartSquare != 64)
	{
		whiteToMove = !whiteToMove;

		//std::cout << "move made by side " << whiteToMove << std::endl;

		ClearSquare(allPieces, toUndo.TargetSquare);
		SetSquare(allPieces, toUndo.StartSquare);

		ClearSquare(pieceBitboards[toUndo.MovePieceType], toUndo.TargetSquare);
		SetSquare(pieceBitboards[toUndo.MovePieceType], toUndo.StartSquare);

		if (whiteToMove)
		{
			ClearSquare(whitePieces, toUndo.TargetSquare);
			SetSquare(whitePieces, toUndo.StartSquare);

			if (toUndo.IsEnPassant)
			{
				SetSquare(allPieces, toUndo.TargetSquare + 8);
				SetSquare(blackPieces, toUndo.TargetSquare + 8);
				SetSquare(pieceBitboards[6], toUndo.TargetSquare + 8);
			}
			else if (toUndo.IsCapture)
			{
				//std::cout << "undoing a capture" << std::endl;
				SetSquare(allPieces, toUndo.TargetSquare);
				SetSquare(blackPieces, toUndo.TargetSquare);
				SetSquare(pieceBitboards[toUndo.CapturePieceType - 1], toUndo.TargetSquare);
			} 

			if (toUndo.IsPromotion)
			{
				ClearSquare(pieceBitboards[toUndo.PromotionPieceType - 1], toUndo.TargetSquare);
			}

			if (toUndo.IsCastles)
			{
				if (toUndo.TargetSquare == 62)
				{
					// kingside castling
					SetSquare(pieceBitboards[3], 63); // place rook at h1
					SetSquare(whitePieces, 63);
					SetSquare(allPieces, 63);
					ClearSquare(pieceBitboards[3], 61);   // remove at f1
					ClearSquare(whitePieces, 61);
					ClearSquare(allPieces, 61);

					whiteKingside = true;
				}
				else 
				{
					// queenside castling
					SetSquare(pieceBitboards[3], 56); // place rook at a1
					SetSquare(whitePieces, 56);
					SetSquare(allPieces, 56);
					ClearSquare(pieceBitboards[3], 59);   // remove at d1
					ClearSquare(whitePieces, 59);
					ClearSquare(allPieces, 59);

					whiteQueenside = true;
				}
			}
			
			if (toUndo.LosesKingside)
			{
				whiteKingside = true;
			}
			
			if (toUndo.LosesQueenside)
			{
				whiteQueenside = true;
			}
		} 
		else 
		{
			ClearSquare(blackPieces, toUndo.TargetSquare);
			SetSquare(blackPieces, toUndo.StartSquare);

			if (toUndo.IsEnPassant)
			{
				SetSquare(allPieces, toUndo.TargetSquare - 8);
				SetSquare(whitePieces, toUndo.TargetSquare - 8);
				SetSquare(pieceBitboards[0], toUndo.TargetSquare - 8);
			}
			else if (toUndo.IsCapture)
			{
				//std::cout << "undoing a capture" << std::endl;
				SetSquare(allPieces, toUndo.TargetSquare);
				SetSquare(whitePieces, toUndo.TargetSquare);
				SetSquare(pieceBitboards[toUndo.CapturePieceType - 1], toUndo.TargetSquare);
			}

			if (toUndo.IsPromotion)
			{
				ClearSquare(pieceBitboards[toUndo.PromotionPieceType - 1], toUndo.TargetSquare);
			}

			if (toUndo.IsCastles)
			{
				if (toUndo.TargetSquare == 6)
				{
					// kingside castling
					SetSquare(pieceBitboards[9], 7); // place rook at h8
					SetSquare(blackPieces, 7);
					SetSquare(allPieces, 7);
					ClearSquare(pieceBitboards[9], 5);   // and remove at f8
					ClearSquare(blackPieces, 5);
					ClearSquare(allPieces, 5);
				}
				else 
				{
					// queenside castling
					SetSquare(pieceBitboards[9], 0); // place rook at a8
					SetSquare(blackPieces, 0);
					SetSquare(allPieces, 0);
					ClearSquare(pieceBitboards[9], 3);   // and remove at d8
					ClearSquare(blackPieces, 3);
					ClearSquare(allPieces, 3);
				}
			}
			
			if (toUndo.LosesKingside)
			{
				blackKingside = true;
			}
			
			if (toUndo.LosesQueenside)
			{
				blackQueenside = true;
			}
		}

		enPassantSquare = toUndo.PriorEnPassantSquare;
		updateAttacks();
	}
}*/

int Board::getPieceTypeAtSquare(int index, int start)
{
	for (int i = start; i < 6 + start; i++)
	{
		if (SquareIsSet(pieceBitboards[i], index))
		{
			return i + 1;
		}
	}

	return 0;
}

ulong Board::moveBoardFromSquare(int index)
{
	ulong movesBoard = 0;

	std::vector<Move> legalMoves = testMoves;

	for (int i = 0; i < legalMoves.size(); i++)
	{
		if (legalMoves.at(i).getStartSquare() == index)
		{
			SetSquare(movesBoard, legalMoves.at(i).getTargetSquare());
		}
	}

	return movesBoard;
}

void Board::fillBoardArray(int* arr)
{
	for (int i = 0; i < 64; i++)
	{
		arr[i] = forDrawing[i];
	}
}

void Board::updateDrawingArray()
{
	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 12; j++)
		{
			if (SquareIsSet(pieceBitboards[j], i))
			{
				forDrawing[i] = j + 1;
				break;
			}

			forDrawing[i] = 0;
		}
	}
}

bool Board::isWhitePiece(int index)
{
	return SquareIsSet(colorPieces[1], index);
}

bool Board::isBlackPiece(int index)
{
	return SquareIsSet(colorPieces[0], index);
}

bool Board::attackedByEnemyPawns(int square)
{
	return SquareIsSet(attacks[6 * whiteToMove], square);
}

bool Board::isWhiteToMove()
{
	return whiteToMove;
}

int Board::getEnPassant()
{
	return enPassantSquare;
}

int Board::isDrawn()
{
	if (stalemate)
	{
		return 1;
	}
	else if (insufficientMaterial)
	{
		return 2;
	}
	else if (threefoldRep)
	{
		return 3;
	}
	else if (fiftyMoves)
	{
		return 4;
	}

	return 0;
}

int Board::isCheckmate()
{
	return checkmate;
}

bool Board::gameIsOver()
{
	return gameOver;
}

/*bool Board::isRepetition()
{
	return (repetitionTable[zobristHistory.back() & 16383] > 1);
}*/

int Board::getKingSquare(bool white)
{
	ulong kingBoard = pieceBitboards[6 * (!white) + 5];
	return ClearAndGetLSB(kingBoard);
}

void Board::Reset(std::string fen)
{
	castlingRights.clear();
	enPassantSquares.clear();
	capturedPieces.clear();
	moveHistory.clear();
	zobristHistory.clear();
	gamePhases.clear();
	moveCountsToFifty.clear();

	for (int i = 0; i < 12; i++)
	{
		pieceBitboards[i] = 0ULL;
	}

	colorPieces[0] = 0ULL;
	colorPieces[1] = 0ULL;

	/*for (int i = 0; i < 12384; i++)
	{
		repetitionTable[i] = 0;
	}*/

	allPieces = 0ULL;
	colorAttacks[0] = 0ULL;
	colorAttacks[1] = 0ULL;

	fenString = fen;

	/*whiteKingside = false;
	whiteQueenside = false;
	blackKingside = false;
	blackQueenside = false;*/

	ushort canCastle = 0;

	std::string enPassantString;

	zobristHistory.push_back(0ULL);

	gameOver = false;

	stalemate = false;
	insufficientMaterial = false;
	threefoldRep = false;
	fiftyMoves = false;

	moveCountsToFifty.push_back(0);

	gamePhases.push_back(0);

	checkmate = 0; // no side has been checkmated

	int index = 0;
	int section = 0;
	for (int i = 0; i < fen.size(); i++)
	{
		if (fen[i] == ' ')
		{
			if (section > 2) break;
			section++;
		} else {
			switch (section)
			{
				// parsing pieces on the board
				case 0:
				{
					int possiblyNumeric = fen[i] - '0';
					if (possiblyNumeric > 0 && possiblyNumeric < 9)
					{
						index += possiblyNumeric;
					} else if (fen[i] != '/') {
						int pieceCode;

						for (int j = 0; j < 12; j++)
						{
							if (fen[i] == charList[j])
							{
								pieceCode = j + 1;
								break;
							}
						}

						forDrawing[index] = pieceCode;

						SetSquare(pieceBitboards[pieceCode - 1], index);
						zobristHistory.back() ^= Constants::PIECE_SQUARE_ZOBRISTS[pieceCode - 1][index];

						gamePhases.back() += Constants::PIECE_PHASES[pieceCode - 1];

						index++;
					}

					break;
				}

				// parsing side to move
				case 1:
				{
					whiteToMove = (fen[i] == 'w');
					if (whiteToMove) zobristHistory.back() ^= Constants::WHITE_ZOBRIST;
					break;
				}

				// parsing castling rights
				case 2:
				{
					switch (fen[i])
					{
						case 'K':
						{
							//whiteKingside = true;
							canCastle |= 1U;
							break;
						}

						case 'Q':
						{
							//whiteQueenside = true;
							canCastle |= 2U;
							break;
						}

						case 'k':
						{
							//blackKingside = true;
							canCastle |= 4U;
							break;
						}

						case 'q':
						{
							//blackQueenside = true;
							canCastle |= 8U;
							break;
						}
					}

					break;
				}

				// parsing en passant moves
				case 3:
				{
					enPassantString += fen[i];
					break;
				}
			}
		}
	}

	for (int i = 0; i < 6; i++)
	{
		//whitePieces |= pieceBitboards[i];
		colorPieces[1] |= pieceBitboards[i];
		//blackPieces |= pieceBitboards[i + 6];
		colorPieces[0] |= pieceBitboards[i + 6];
	}

	allPieces = colorPieces[0] | colorPieces[1];

	castlingRights.push_back(canCastle);
	zobristHistory.back() ^= Constants::CASTLE_ZOBRISTS[castlingRights.back()];

	enPassantSquare = algToInt(enPassantString);
	enPassantSquares.push_back(algToInt(enPassantString));
	if (enPassantSquare != 64) zobristHistory.back() ^= Constants::EN_PASSANT_FILE_ZOBRISTS[enPassantSquare % 8];

	// add this position to the repetition table, using lowest 14 bits as index
	//repetitionTable[zobristHistory.back() & 16383]++;

	//std::cout << zobristHistory.back() << std::endl;

	//std::cout << gamePhases.back() << std::endl;

	updateAttacks();

	getLegalMoves();
}

int Board::scoreMaterial()
{
	int material = 0;

	for (int i = 0; i < 6; i++)
	{
		material += Constants::PIECE_VALUES[i] * (NumSetBits(pieceBitboards[i]) - NumSetBits(pieceBitboards[i + 6]));
	}

	return material;
}

int Board::pieceSquareScore()
{
	int mgscore = 0;
	int escore = 0;
	int phase = gamePhases.back();
	int square;
	ulong thisBoard;

	for (int i = 0; i < 6; i++)
	{
		thisBoard = pieceBitboards[i];
		while (thisBoard != 0)
		{
			square = ClearAndGetLSB(thisBoard);
			mgscore += Constants::MIDDLEGAME_PSTS[i][square];
			escore += Constants::ENDGAME_PSTS[i][square];
		}

		thisBoard = pieceBitboards[i + 6];
		while (thisBoard != 0)
		{
			square = ClearAndGetLSB(thisBoard);
			mgscore -= Constants::MIDDLEGAME_PSTS[i + 6][square];
			escore -= Constants::ENDGAME_PSTS[i + 6][square];
		}
	}

	return ((phase * mgscore + (24 - phase) * escore) / 24);
}
