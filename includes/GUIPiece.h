#ifndef GUI_PIECE_H
#define GUI_PIECE_H
#include "GUISquare.h"

class GUIPiece
{
private:
	int pieceType;
	int drawType;
	int squareIndex;
	bool isWhite;

	std::string filePath;
	SDL_Surface* surface;

public:
	// constructor
	GUIPiece(int sqInd, int type);

	std::string getPath();

	int getType();

	int getDrawType();
};

#endif