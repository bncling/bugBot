#include <SDL.h>
#include "includes/GUISquare.h"

const int SQUARE_SIZE = 100;

GUISquare::GUISquare(int index) 
{
	sqInd = index;
	sqFile = sqInd % 8;
	sqRank = (sqInd - sqFile) / 8; // inverted

	sq.x = sqFile * SQUARE_SIZE;
	sq.y = sqRank * SQUARE_SIZE;
	sq.w = SQUARE_SIZE;
	sq.h = SQUARE_SIZE;

	is_white = (sqFile + sqRank) % 2 != 0;

	if (is_white) {
		dred = 112;
		dgreen = 142;
		dblue = 140;
	} else {
		dred = 200;
		dgreen = 216;
		dblue = 209;
	}

	// set values used for display;
	red = dred; 
	green = dgreen;
	blue = dblue;
	alpha = SDL_ALPHA_OPAQUE;
}

GUISquare::GUISquare()
{
	GUISquare(0);
}

void GUISquare::RenderSquare(SDL_Renderer* renderer) 
{
	// set the color to use
	SDL_SetRenderDrawColor(renderer, red, green, blue, alpha);

	// draw the square
	SDL_RenderDrawRect(renderer, &sq);
	SDL_RenderFillRect(renderer, &sq);
}

void GUISquare::RenderPiece(SDL_Renderer* renderer, SDL_Texture* texture)
{
	SDL_RenderCopy(renderer, texture, NULL, &sq);
}

void GUISquare::setColor(int r, int g, int b) 
{
	red = r;
	green = g;
	blue = b;
}

void GUISquare::setAlpha(int a)
{
	alpha = a;
}

void GUISquare::resetColor() 
{
	red = dred;
	green = dgreen;
	blue = dblue;
	alpha = SDL_ALPHA_OPAQUE;
}

int GUISquare::getIndex()
{
	return sqInd;
}

SDL_Rect* GUISquare::getRect()
{
	return &sq;
}