#include <SDL.h>
#include <iostream>
#include <vector>
#include "main/all_headers.h"
#include "includes/GUISquare.h"
#include "includes/GUIPiece.h"

const int SQUARE_SIZE = 100;

int main() {

	int boardSize = 8 * SQUARE_SIZE;

	/*
	// testing bitboards
	ulong testBitboard;
	SetSquare(testBitboard, 8);
	SetSquare(testBitboard, 11);
	ClearSquare(testBitboard, 8);
	ToggleSquare(testBitboard, 13);
	std::cout << std::bitset<64>(testBitboard) << std::endl;
	std::cout << NumSetBits(testBitboard) << std::endl;
	ToggleSquare(testBitboard, 11);
	std::cout << std::bitset<64>(testBitboard) << std::endl;
	std::cout << SquareIsSet(testBitboard, 8) << std::endl;
	std::cout << SquareIsSet(testBitboard, 13) << std::endl;
	std::cout << ClearAndGetLSB(testBitboard) << std::endl;
	std::cout << std::bitset<64>(testBitboard) << std::endl;

	// test the square struct
	Square testSquare("e4");
	std::cout << testSquare.Name << " " << testSquare.File << " " << testSquare.Rank << std::endl; 
	*/


	// create the board of squares to be displayed
	std::vector<GUISquare> displayBoard;
	for (int i = 0; i < 64; i++) 
	{
		displayBoard.push_back(GUISquare(i));
	}

	// create a board object initialized to the starting position
	Board board("rn1qk2r/pb1ppp2/2p1Pbpn/P5NQ/1pP4P/5P2/1P1P2P1/RNB1KB1R b KQkq c3 0 11");

	std::vector<Move> legalMoves = board.getLegalMoves();
	std::cout << "--------------------" << std::endl;
	for (int i = 0; i < legalMoves.size(); i++)
	{
		std::cout << legalMoves.at(i) << std::endl;
	}
	std::cout << "--------------------" << std::endl;

	// get an array and fill it with the Board method
	int boardArray[64];
	board.fillBoardArray(boardArray);

	// position pieces on the board based on the above test board
	std::vector<GUIPiece> pieceBoard;
	for (int i = 0; i < 64; i++)
	{
		pieceBoard.push_back(GUIPiece(i, boardArray[i]));
	}

	// keep track of mouse
	bool leftMouseDown = false;
	SDL_Point mousePos;

	// square to highlight (in check)
	int squareInCheck = 64;

	// piece to drag 
	int pieceSquareSelected = 64;
	std::string dragPath = "";
	int pieceTypeSelected = 0;
	ulong pieceAttacksBitboard;

	// rect for dragging
	SDL_Rect draggableRect;
	draggableRect.x = boardSize;
	draggableRect.y = boardSize;
	draggableRect.w = SQUARE_SIZE;
	draggableRect.h = SQUARE_SIZE;

	// dummy move to change when moves are played
	Move playedMove;

	// create the display window
	SDL_Window* window = nullptr;
	window = SDL_CreateWindow("Chess++",
		150,
		0,
		boardSize,
		boardSize,
		SDL_WINDOW_SHOWN);

	// create a renderer to put objects in the window
	SDL_Renderer* renderer = nullptr;
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	// start game loop
	bool gameRunning = true;
	while (gameRunning) {

		SDL_Delay(10);

		// event to be listening for
		SDL_Event event;

		// (1) handle any input
		while (SDL_PollEvent(&event)) {

			switch (event.type)
			{
				case SDL_QUIT:
				{
					gameRunning = false;
					break;
				}

				// keep track of mouse position
				case SDL_MOUSEMOTION:
				{
					SDL_GetMouseState(&mousePos.x, &mousePos.y);

					if (leftMouseDown && pieceSquareSelected != 64)
					{
						draggableRect.x = mousePos.x - SQUARE_SIZE / 2;
						draggableRect.y = mousePos.y - SQUARE_SIZE / 2;
					}

					break;
				}

				// if the mouse button comes up, a square has been selected, reset 
				case SDL_MOUSEBUTTONUP:
				{
					if (leftMouseDown && event.button.button == SDL_BUTTON_LEFT)
					{
						leftMouseDown = false;

						// find out where to put the dragged piece
						for (int i = 0; i < 64; i++)
						{
							if (SDL_PointInRect(&mousePos, displayBoard.at(i).getRect()) && pieceTypeSelected != 0)
							{
								// EVENTUALLY: all of this could be done probably by
								// looping over legal moves and seeing which (if any)
								// have the same start and target squares
								bool moveIsLegal = false;

								for (int j = 0; j < legalMoves.size(); j++)
								{
									if (legalMoves.at(j).StartSquare == pieceSquareSelected && legalMoves.at(j).TargetSquare == i)
									{
										moveIsLegal = true;
										playedMove = legalMoves.at(j);
										if (playedMove.IsPromotion)
										{
											std::cin >> playedMove.PromotionPieceType;
										}
										std::cout << playedMove << std::endl;
										break;
									}
								}

								/*// collect all the relevant move data in the Move instance
								playedMove.StartSquare = pieceSquareSelected;
								playedMove.TargetSquare = i;

								playedMove.MovePieceType = pieceTypeSelected;

								if (boardArray[i] != 0)
								{
									playedMove.IsCapture = true;
									playedMove.CapturePieceType = boardArray[i];
								}

								// eventually we'll want to check if this is legal
								// something like if (board.IsLegal(playedMove))
								// for now just prevent a capture of the same color piece
								if (board.isWhiteToMove() && !board.isWhitePiece(i) || !board.isWhiteToMove() && !board.isBlackPiece(i))
								{
									// pass Move instance to the internal move function
									board.MakeMove(playedMove);
								}*/

								if (moveIsLegal)
								{
									board.MakeMove(playedMove);
								}

								// reset the Move instance
								playedMove.Reset();

								// use the internal board rep to update the array guiding the external rep
								board.updateDrawingArray();
								board.fillBoardArray(boardArray);

								legalMoves = board.getLegalMoves();
								/*
								for (int i = 0; i < legalMoves.size(); i++)
								{
									std::cout << legalMoves.at(i) << std::endl;
								}
								std::cout << "--------------------" << std::endl;*/

								break;
							}
						}

						// reset everything
						// reset the Move instance
						playedMove.Reset();

						// use the internal board rep to update the array guiding the external rep
						board.updateDrawingArray();
						board.fillBoardArray(boardArray);

						squareInCheck = 64;
						pieceSquareSelected = 64;
						pieceAttacksBitboard = 0;
						pieceTypeSelected = 0;
						dragPath = "";
						draggableRect.x = boardSize;
						draggableRect.y = boardSize;
					}
					break;
				}

				// if the button goes down, change the selected square
				case SDL_MOUSEBUTTONDOWN:
				{
					if (!leftMouseDown && event.button.button == SDL_BUTTON_LEFT)
					{
						leftMouseDown = true;

						// loop over squares to see which was selected
						for (int i = 0; i < 64; i++)
						{
							if (SDL_PointInRect(&mousePos, displayBoard.at(i).getRect()))
							{
								//squareInCheck = i;

								if (boardArray[i] != 0)
								{
									pieceTypeSelected = pieceBoard.at(i).getDrawType();

									if ((board.isWhiteToMove() && (pieceTypeSelected <= 6)) || !board.isWhiteToMove() && (pieceTypeSelected > 6))
									{
										pieceSquareSelected = i;
										pieceAttacksBitboard = board.moveBoardFromSquare(i);
							
										draggableRect.x = mousePos.x - SQUARE_SIZE / 2;
										draggableRect.y = mousePos.y - SQUARE_SIZE / 2;
									} else {
										// wrong color pieces chosen, reset
										pieceTypeSelected = 0;
									}
								}

								break;
							}
						}
					}

					break;
				}
			}
	    }

	    // (2) handle any updates 

	    // update external board
	    pieceBoard.clear();
	    for (int i = 0; i < 64; i++)
		{
			pieceBoard.push_back(GUIPiece(i, boardArray[i]));
		}

	    // (3) clear everything and redraw it with the updates
	    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	    SDL_RenderClear(renderer);

	    // draw the board
	    for (int i = 0; i < 64; i++)
	    {
	    	// change color if in check
	    	if (squareInCheck == i)
	    	{
	    		displayBoard.at(i).setAlpha(100);
	    		displayBoard.at(i).setColor(200, 0, 0);
	    		displayBoard.at(i).RenderSquare(renderer);
	    		displayBoard.at(i).resetColor();
	    	} else {
	    		displayBoard.at(i).RenderSquare(renderer);
	    	}

	    	// change color of squares attacked 
	    	// EVENTUALLY MAKE THIS LEGAL MOVES FOR PIECE
	    	//std::cout << std::bitset<64>(pieceAttacksBitboard) << std::endl;
	    	if (SquareIsSet(pieceAttacksBitboard, i))
	    	{
	    		displayBoard.at(i).setAlpha(150);
	    		displayBoard.at(i).setColor(100, 200, 100);
	    		displayBoard.at(i).RenderSquare(renderer);
	    		displayBoard.at(i).resetColor();
	    	} else {
	    		displayBoard.at(i).RenderSquare(renderer);
	    	}
	    	
	    	// render piece, if applicable
	    	if (pieceBoard.at(i).getPath() != "") 
	    	{
		    	if (pieceSquareSelected == i)
		    	{
		    		dragPath = pieceBoard.at(i).getPath();
		    		boardArray[i] = 0;
		    	}

	    		SDL_Surface* pieceSurface = SDL_LoadBMP(pieceBoard.at(i).getPath().c_str());
	    		SDL_Texture* pieceTexture = SDL_CreateTextureFromSurface(renderer, pieceSurface);

	    		// done with the surface
		    	SDL_FreeSurface(pieceSurface);

		    	// draw the square with the piece
		    	displayBoard.at(i).RenderPiece(renderer, pieceTexture);

		    	// done with texture
		    	SDL_DestroyTexture(pieceTexture);
	    	}
	    }

	    // if a piece is selected, draw it
	    if (pieceSquareSelected != 64)
	    {
	    	SDL_Surface* dragSurface = SDL_LoadBMP(dragPath.c_str());
	    	SDL_Texture* dragTexture = SDL_CreateTextureFromSurface(renderer, dragSurface);

	    	SDL_FreeSurface(dragSurface);

	    	SDL_RenderCopy(renderer, dragTexture, NULL, &draggableRect);

	    	SDL_DestroyTexture(dragTexture);
	    }

	    // show the drawings
	    SDL_RenderPresent(renderer);
	}

	// deallocate memory
	//SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}