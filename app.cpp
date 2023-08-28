#include <SDL.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>
#include "main/all_headers.h"
#include "includes/GUISquare.h"
#include "includes/GUIPiece.h"
#include "includes/GUI.h"


const int timeBank = 5000;
const int increment = 100;

Move getRandomMove(Board& board)
{
	std::vector<Move> legalMoves = board.getLegalMoves();

	double r = ((double) std::rand() / (RAND_MAX));
	int index = std::floor(r * legalMoves.size());

	return legalMoves.at(index);
}


int main()
{
	//Board board("r3k2r/p1ppqpb1/Bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPB1PPP/R3K2R b KQkq - 0 1");
	Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

	srand(time(0));

	bool whiteIsHuman = false;
	bool blackIsHuman = false;

	int whiteTimeRemaining = timeBank;
	int blackTimeRemaining = timeBank;

	int wins = 0;
	int draws = 0;
	int losses = 0;

	int timeTaken;
	Timer t;

	V0 v0Bot;
	V1 v1Bot;
	V2 v2Bot;
	V3 v3Bot;
	V4 v4Bot;
	V5 v5Bot;
	V6 v6Bot;

	GUI gameGUI(board, whiteIsHuman, blackIsHuman);

	bool displaying = true;
	while (displaying)
	{
		SDL_Event event;

		while (SDL_PollEvent(&event)) 
		{
			switch (event.type)
			{
				case SDL_QUIT:
				{
					displaying = false;

					std::cout << "White player won " << wins << " games, drew " << draws << " games, and lost " << losses << " games." << std::endl;
					break;
				}

				case SDL_MOUSEMOTION:
				{
					gameGUI.HandleMouseMotion(board);
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					if (event.button.button == SDL_BUTTON_LEFT) gameGUI.HandleMouseDown(board);
					break;
				}

				case SDL_MOUSEBUTTONUP:
				{
					if (event.button.button == SDL_BUTTON_LEFT) gameGUI.HandleMouseUp(board);
					break;
				}
			}
		}

		if (!whiteIsHuman && board.isWhiteToMove())
		{
			// get + make computer move for white
			if (!board.gameIsOver())
			{
				t.reset();
				Move botMove = v5Bot.getMove(board, whiteTimeRemaining);
				timeTaken = t.elapsed();

				whiteTimeRemaining -= timeTaken;
				//std::cout << whiteTimeRemaining << std::endl;

				if (whiteTimeRemaining >= 0)
				{
					board.MakeMove(botMove);
					gameGUI.ComputerMove(board);
				}

				whiteTimeRemaining += increment;
			}
		}
		else if (!blackIsHuman && !board.isWhiteToMove())
		{
			// get + make computer move for black
			if (!board.gameIsOver())
			{
				t.reset();
				Move botMove = v6Bot.getMove(board, blackTimeRemaining, true);
				timeTaken = t.elapsed();

				blackTimeRemaining -= timeTaken;
				//std::cout << blackTimeRemaining << std::endl;

				if (blackTimeRemaining >= 0)
				{
					board.MakeMove(botMove);
					gameGUI.ComputerMove(board);
				}

				blackTimeRemaining += increment;

				//std::cout << botMove << std::endl;
			}
		}
	
		gameGUI.UpdateBoard(board);
		gameGUI.DrawBoard();

		if (board.gameIsOver() || whiteTimeRemaining <= 0 || blackTimeRemaining <= 0)
		{
			if (whiteTimeRemaining <= 0)
			{
				std::cout << "Timeout: Black wins" << std::endl;
				losses++;
			}

			if (blackTimeRemaining <= 0)
			{
				std::cout << "Timeout: White wins" << std::endl;
				wins++;
			}

			if (board.isCheckmate())
			{
				if (board.isWhiteToMove())
				{
					losses++;
				}
				else
				{
					wins++;
				}
			}

			if (board.isDrawn())
			{
				draws++;
			}

			SDL_Delay(300);
			board.Reset("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

			gameGUI.Reset();

			gameGUI.ComputerMove(board);
			gameGUI.UpdateBoard(board);
			gameGUI.DrawBoard();

			whiteTimeRemaining = timeBank;
			blackTimeRemaining = timeBank;
		}
	}

	return 0;
}