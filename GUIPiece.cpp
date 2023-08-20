#include <SDL.h>
#include <string>
#include "includes/GUIPiece.h"
#include "includes/GUISquare.h"

GUIPiece::GUIPiece(int sqInd, int type)
{
	squareIndex = sqInd;
	isWhite = true;
	pieceType = type;
	drawType = type;
	
	if (type > 6)
	{
		isWhite = false;
		pieceType -= 6;
	}

	if (type != 0)
	{
		filePath = "./pieces/";
		filePath.append(std::to_string(type));
		filePath.append(".bmp");
	} else {
		filePath = "";
	}
}

std::string GUIPiece::getPath()
{
	return filePath;
}

int GUIPiece::getType()
{
	return pieceType;
}

int GUIPiece::getDrawType()
{
	return drawType;
}