#include "includes/GUI.h"
#include <SDL.h>
#include <iostream>
#include <string>
#include "main/all_headers.h"

GUI::GUI(Board& board, bool whiteIsHuman, bool blackIsHuman)
{
	squareSize = 100;
	boardSize = 8 * squareSize;

	for (int i = 0; i < 64; i++)
	{
		displayBoard.push_back(GUISquare(i));
		pieceBoard.push_back(GUIPiece(i, boardArray[i]));
	}

	squareInCheck = 64;
	pieceSquareSelected = 64;
	pieceTypeSelected = 0;
	dragPath = "";
	pieceAttacksBitboard = 0ULL;

	window = SDL_CreateWindow("Chess++",
		300,
		0,
		boardSize,
		boardSize,
		SDL_WINDOW_SHOWN);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	leftMouseDown = false;

	draggableRect.x = boardSize;
	draggableRect.y = boardSize;
	draggableRect.w = squareSize;
	draggableRect.h = squareSize;

	playedMove.Reset();

	legalMoves = board.getLegalMoves();
	board.fillBoardArray(boardArray);

	whiteHuman = whiteIsHuman;
	blackHuman = blackIsHuman;
}

GUI::~GUI()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void GUI::DrawBoard()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderClear(renderer);

    //std::cout << "draw board method called" << std::endl;

    //std::cout << std::bitset<64>(pieceAttacksBitboard) << std::endl;

    // draw the board
    for (int i = 0; i < 64; i++)
    {
    	//std::cout << displayBoard.at(i).getIndex() << std::endl;
    	// change color if in check
    	if (squareInCheck == i)
    	{
    		displayBoard.at(i).setAlpha(120);
    		displayBoard.at(i).setColor(200, 0, 0);
    		displayBoard.at(i).RenderSquare(renderer);
    		displayBoard.at(i).resetColor();
    	} else {
    		// render the square at least once so that move highlighting looks nice
    		displayBoard.at(i).RenderSquare(renderer);
    	}

    	// highlight legal moves for piece
    	if (SquareIsSet(pieceAttacksBitboard, i) && squareInCheck != i)
    	{
    		displayBoard.at(i).setAlpha(150);
    		displayBoard.at(i).setColor(100, 200, 100);
    		displayBoard.at(i).RenderSquare(renderer);
    		displayBoard.at(i).resetColor();
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

    	if (pieceSquareSelected != 64)
	    {
	    	SDL_Surface* dragSurface = SDL_LoadBMP(dragPath.c_str());
	    	SDL_Texture* dragTexture = SDL_CreateTextureFromSurface(renderer, dragSurface);

	    	SDL_FreeSurface(dragSurface);

	    	SDL_RenderCopy(renderer, dragTexture, NULL, &draggableRect);

	    	SDL_DestroyTexture(dragTexture);
	    }
    }

    SDL_RenderPresent(renderer);
}

void GUI::ComputerMove(Board& board)
{
	board.updateDrawingArray();
	board.fillBoardArray(boardArray);

	legalMoves = board.getLegalMoves();
}

void GUI::UpdateBoard(Board& board)
{
	//board.updateDrawingArray();
	//board.fillBoardArray(boardArray);

	pieceBoard.clear();
    for (int i = 0; i < 64; i++)
	{
		pieceBoard.push_back(GUIPiece(i, boardArray[i]));
	}

	if (board.isDrawn() != 0)
	{
		std::cout << "Game over: ";

		switch (board.isDrawn())
		{
			case 1:
			{
				std::cout << "no legal moves (stalemate)" << std::endl;
				break;
			}

			case 2:
			{
				std::cout << "insufficient material" << std::endl;
				break;
			}

			case 3:
			{
				std::cout << "threefold repetition" << std::endl;
				break;
			}

			case 4:
			{
				std::cout << "fifty move rule" << std::endl;
				break;
			}
		}
	}

	if (board.isCheckmate() != 0)
	{
		std::cout << "Checkmate! ";

		switch (board.isCheckmate())
		{
			case 1: 
			{
				std::cout << "Black wins." << std::endl;
				break;
			}

			case -1:
			{
				std::cout << "White wins." << std::endl;
				break;
			}
		}
	}

	if (board.kingInCheck(board.isWhiteToMove()))
	{
		squareInCheck = board.getKingSquare(board.isWhiteToMove());
	} 
	else
	{
		squareInCheck = 64;
	}
}

void GUI::HandleMouseMotion(Board& board)
{
	SDL_GetMouseState(&mousePos.x, &mousePos.y);

	if (leftMouseDown && pieceSquareSelected != 64)
	{
		draggableRect.x = mousePos.x - squareSize / 2;
		draggableRect.y = mousePos.y - squareSize / 2;
	}
}

void GUI::HandleMouseDown(Board& board)
{
	if (!leftMouseDown)
	{
		leftMouseDown = true;

		// loop over squares to see which was selected
		for (int i = 0; i < 64; i++)
		{
			if (SDL_PointInRect(&mousePos, displayBoard.at(i).getRect()))
			{
				if (boardArray[i] != 0)
				{
					pieceTypeSelected = pieceBoard.at(i).getDrawType();

					if (!board.gameIsOver() && ((board.isWhiteToMove() && (pieceTypeSelected <= 6) && whiteHuman) || !board.isWhiteToMove() && (pieceTypeSelected > 6) && blackHuman))
					{
						pieceSquareSelected = i;
						pieceAttacksBitboard = board.moveBoardFromSquare(i);
			
						draggableRect.x = mousePos.x - squareSize / 2;
						draggableRect.y = mousePos.y - squareSize / 2;
					} else {
						// wrong color pieces chosen, reset
						pieceTypeSelected = 0;
					}
				}

				break;
			}
		}
	}
}

void GUI::HandleMouseUp(Board& board)
{
	if (leftMouseDown)
	{
		leftMouseDown = false;

		// find out where to put the dragged piece
		for (int i = 0; i < 64; i++)
		{
			if (SDL_PointInRect(&mousePos, displayBoard.at(i).getRect()) && (pieceTypeSelected != 0))
			{
				bool moveIsLegal = false;

				for (int j = 0; j < legalMoves.size(); j++)
				{
					if (legalMoves.at(j).getStartSquare() == pieceSquareSelected && legalMoves.at(j).getTargetSquare() == i)
					{
						moveIsLegal = true;
						playedMove = legalMoves.at(j);
						if (playedMove.isPromotion())
						{
							// for now just default to queen promotion
							playedMove.moveCode |= (7 << 13);

							//std::cin >> playedMove.PromotionPieceType;
						}
						break;
					}
				}

				if (moveIsLegal)
				{
					board.MakeMove(playedMove);

					board.updateDrawingArray();
					board.fillBoardArray(boardArray);

					//humanMove = true;

					legalMoves = board.getLegalMoves();

					break;
				}
				else
				{
					boardArray[pieceSquareSelected] = pieceTypeSelected;
					break;
				}
			}
		}

		pieceSquareSelected = 64;
		pieceAttacksBitboard = 0;
		pieceTypeSelected = 0;
		dragPath = "";
		draggableRect.x = boardSize;
		draggableRect.y = boardSize;
	}
}

void GUI::ChangePlayers(bool whiteIsHuman, bool blackIsHuman)
{
	whiteHuman = whiteIsHuman;
	blackHuman = blackIsHuman;
}

void GUI::Reset()
{
	for (int i = 0; i < 64; i++)
	{
		boardArray[i] = 0;
	}

	squareInCheck = 64;

	SDL_RenderClear(renderer);

	pieceBoard.clear();
	displayBoard.clear();

	for (int i = 0; i < 64; i++)
	{
		displayBoard.push_back(GUISquare(i));
		pieceBoard.push_back(GUIPiece(i, boardArray[i]));
	}

}

