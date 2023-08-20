#ifndef GUI_SQUARE_H
#define GUI_SQUARE_H

class GUISquare
{
private:
	SDL_Rect sq; 
	int sqInd;
	int sqFile;
	int sqRank; 

	bool is_white;

	// shown
	int red, green, blue, alpha;

	// default
	int dred, dgreen, dblue;

public: 
	// constructor
	GUISquare(int index);

	// default constructor
	GUISquare();

	void RenderSquare(SDL_Renderer* renderer);

	void RenderPiece(SDL_Renderer* renderer, SDL_Texture* texture);

	void setColor(int r, int g, int b);

	void setAlpha(int a);

	void resetColor();

	int getIndex();

	SDL_Rect* getRect();
};

#endif