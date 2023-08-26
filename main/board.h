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

	/*
	ulong whitePieces;
	ulong blackPieces;
	*/

	ulong allPieces;

	ulong colorPieces[2] = {};

	// attack tables, eventually will want these to be incrementally updated?
	ulong attacks[12] = {};
	ulong colorAttacks[2] = {};
	ulong kingAttacks[2] = {};

	ulong checkingPieces;
	int numCheckers;

	ulong pushMask;
	ulong captureMask;

	ulong enemyBishops;
	ulong enemyRooks;

	ulong pinnedPawns;
	ulong pinnedKnights;
	ulong pinnedBishops;
	ulong pinnedRooks;
	ulong pinnedQueens;

	ulong repetitionTable[16384] = {};

	std::string fenString;
	int forDrawing[64] = {};

	bool whiteToMove;

	/*
	bool whiteKingside;
	bool whiteQueenside;
	bool blackKingside;
	bool blackQueenside;
	*/

	int enPassantSquare;

	bool gameOver;

	bool stalemate;
	bool insufficientMaterial;
	bool threefoldRep;
	bool fiftyMoves;

	int checkmate;

	std::vector<ushort> castlingRights;
	std::vector<int> enPassantSquares;
	std::vector<int> capturedPieces;
	std::vector<Move> moveHistory;
	std::vector<ulong> zobristHistory;
	std::vector<int> gamePhases;
	std::vector<int> moveCountsToFifty;

	std::vector<Move> testMoves;

	Move toPlay;

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

	void getPawnMoves(bool capsOnly = false);
	void getKnightMoves(bool capsOnly = false);
	void getSlidingMoves(bool capsOnly = false);
	void getKingMoves(bool capsOnly = false);

	void getPinnedMoves(bool capsOnly = false);

	//std::vector<Move> oldLegalMoveGen(bool capsOnly = false);
	//std::vector<Move> getPseudoLegalMoves(bool capsOnly = false);
	std::vector<Move> getLegalMoves(bool capsOnly = false);

	ulong moveBoardFromSquare(int sqInd);

	int getPieceTypeAtSquare(int sqInd, int start);

	void updateAttacks();

	bool isAttacked(int sqInd, bool white);

	bool isInCheck();

	bool kingInCheck(bool white);

	ulong getAttacks(int attackingPiece);

	void MovePiece(int type, int start, int end);
	void MakeMove(Move toPlay);
	void Undo();
	void OldMake(Move toPlay);
	void UndoMove(Move toUndo);

	bool isWhiteToMove();

	int getEnPassant();

	int isDrawn();
	int isCheckmate();

	bool gameIsOver();

	//std::string getFenString();
};

#endif