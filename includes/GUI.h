#ifndef GUI_H 
#define GUI_H 
#include <vector>
#include <string>
#include <SDL.h>
#include "GUIPiece.h"
#include "GUISquare.h"
#include "../main/all_headers.h"

class GUI
{

private:

	int squareSize, boardSize;

	int boardArray[64] = {};
	std::vector<GUIPiece> pieceBoard;
	std::vector<GUISquare> displayBoard;

	int squareInCheck, pieceSquareSelected, pieceTypeSelected;
	std::string dragPath;
	ulong pieceAttacksBitboard;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr; 

	bool displaying;

	bool humanMove = true;

	// keep track of mouse
	bool leftMouseDown;
	SDL_Point mousePos;

	// for moving pieces around
	SDL_Rect draggableRect;

	// for making moves on the board
	Move playedMove;

	std::vector<Move> legalMoves;

	bool whiteHuman, blackHuman;

public:

	GUI(Board& board, bool whiteIsHuman, bool blackIsHuman);
	~GUI();

	void Reset();

	void DrawBoard();

	void UpdateBoard(Board& board);

	void ComputerMove(Board& board);

	void HandleMouseMotion(Board& board);
	void HandleMouseDown(Board& board);
	void HandleMouseUp(Board& board);

	void ChangePlayers(bool whiteIsHuman, bool blackIsHuman);
};

#endif