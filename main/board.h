#ifndef BOARD_H
#define BOARD_H
#include <string>
#include <vector>
#include "bitboards.h"
#include "move.h"

class Board
{
private:
	ulong pieceBitboards[12] = {};

	ulong whitePieces;
	ulong blackPieces;
	ulong allPieces;

	std::string fenString;
	int forDrawing[64] = {};

	bool whiteToMove;

	bool whiteKingside;
	bool whiteQueenside;
	bool blackKingside;
	bool blackQueenside;

	int enPassantSquare;

public:
	// default constructor -- sets to starting position
	Board();

	// constructor -- takes a fen string
	Board(std::string fen);

	void fillBoardArray(int* arr);

	void updateDrawingArray();

	bool isWhitePiece(int index);
	bool isBlackPiece(int index);

	int algToInt(std::string sqName);
	std::string intToAlg(int index);

	std::vector<Move> getLegalMoves(bool capsOnly = false);

	ulong moveBoardFromSquare(int sqInd);

	int getPieceTypeAtSquare(int sqInd, int start);

	void MakeMove(Move toPlay);

	bool isWhiteToMove();

	int getEnPassant();

	//std::string getFenString();
};

#endif