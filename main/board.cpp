#include "board.h"
#include "bitboards.h"
#include "move.h"
#include "constants.h"
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

	whiteKingside = false;
	whiteQueenside = false;
	blackKingside = false;
	blackQueenside = false;
}

Board::Board(std::string fen)
{
	fenString = fen;

	whiteKingside = false;
	whiteQueenside = false;
	blackKingside = false;
	blackQueenside = false;

	std::string enPassantString;

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

						index++;
					}

					break;
				}

				// parsing side to move
				case 1:
				{
					whiteToMove = (fen[i] == 'w');
					break;
				}

				// parsing castling rights
				case 2:
				{
					switch (fen[i])
					{
						case 'K':
						{
							whiteKingside = true;
							break;
						}

						case 'Q':
						{
							whiteQueenside = true;
							break;
						}

						case 'k':
						{
							blackKingside = true;
							break;
						}

						case 'q':
						{
							blackQueenside = true;
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
		whitePieces |= pieceBitboards[i];
		blackPieces |= pieceBitboards[i + 6];
	}

	allPieces = whitePieces | blackPieces;

	enPassantSquare = algToInt(enPassantString);

	//std::cout << enPassantSquare << std::endl;
}

// right now just does pseudo legal moves with no castling
std::vector<Move> Board::getLegalMoves(bool capsOnly)
{
	std::vector<Move> legalMoves;
	ulong pieceBitboard; 
	ulong candidateTargets = 0;
	ulong candidateCaptures;
	ulong moveMask;
	int magicIndex;

	int thisStartSquare;
	int thisTargetSquare;

	bool thisCapture;
	bool thisCastles;
	bool thisPromotion;
	bool thisEnPassant;
	bool thisDoublePawn;

	int thisMovePieceType;
	int thisCapturePieceType;
	int thisPromotionPieceType;
	
	Move toBeAdded{64, 64, false, false, false, false, 0, 0, 0};

	int indexBonus = 0;
	ulong myPieces = whitePieces;
	ulong enemyPieces = blackPieces;

	if (!whiteToMove)
	{
		indexBonus = 6;
		myPieces = blackPieces;
		enemyPieces = whitePieces;
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

		while (candidateTargets != 0)
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
		candidateTargets = Constants::KNIGHT_ATTACKS[thisStartSquare] & ~myPieces;

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
		candidateTargets = Constants::BISHOP_ATTACKS[thisStartSquare][magicIndex] & ~myPieces;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

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

		moveMask = Constants::ROOK_MASKS[thisStartSquare];
		magicIndex = (((moveMask & allPieces) * Constants::ROOK_MAGICS[thisStartSquare]) >> 52);
		candidateTargets = Constants::ROOK_ATTACKS[thisStartSquare][magicIndex] & ~myPieces;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			legalMoves.push_back(toBeAdded);
		}
	}

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
		candidateTargets |= Constants::ROOK_ATTACKS[thisStartSquare][magicIndex] & ~myPieces;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			legalMoves.push_back(toBeAdded);
		}
	}	

	// ---------------------- ADD KING MOVES ----------------------

	thisMovePieceType++;
	pieceBitboard = pieceBitboards[thisMovePieceType];
	toBeAdded.MovePieceType = thisMovePieceType;

	while (pieceBitboard != 0)
	{
		thisStartSquare = ClearAndGetLSB(pieceBitboard);
		toBeAdded.StartSquare = thisStartSquare;
		candidateTargets = Constants::KING_ATTACKS[thisStartSquare] & ~myPieces;

		while (candidateTargets != 0)
		{
			thisTargetSquare = ClearAndGetLSB(candidateTargets);
			toBeAdded.TargetSquare = thisTargetSquare;

			legalMoves.push_back(toBeAdded);
		}
	}


	return legalMoves;
}

// TODO: Update for promotions, en passant, and castling
void Board::MakeMove(Move toPlay)
{
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
			ClearSquare(pieceBitboards[toPlay.CapturePieceType], toPlay.TargetSquare);
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
	} 
	else 
	{
		SetSquare(blackPieces, toPlay.TargetSquare);
		ClearSquare(blackPieces, toPlay.StartSquare);

		if (toPlay.IsCapture)
		{
			ClearSquare(whitePieces, toPlay.TargetSquare);
			ClearSquare(pieceBitboards[toPlay.CapturePieceType], toPlay.TargetSquare);
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
	}

	whiteToMove = !whiteToMove;
}

int Board::getPieceTypeAtSquare(int index, int start)
{
	for (int i = start; i < 5 + start; i++)
	{
		if (SquareIsSet(pieceBitboards[i], index))
		{
			return i;
		}
	}

	return 0;
}

ulong Board::moveBoardFromSquare(int index)
{
	ulong movesBoard = 0;

	std::vector<Move> legalMoves = getLegalMoves();

	for (int i = 0; i < legalMoves.size(); i++)
	{
		if (legalMoves.at(i).StartSquare == index)
		{
			SetSquare(movesBoard, legalMoves.at(i).TargetSquare);
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
	return SquareIsSet(whitePieces, index);
}

bool Board::isBlackPiece(int index)
{
	return SquareIsSet(blackPieces, index);
}

bool Board::isWhiteToMove()
{
	return whiteToMove;
}

int Board::getEnPassant()
{
	return enPassantSquare;
}
