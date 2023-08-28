#include "chessBots.h"
#include "move.h"
#include "board.h"
#include "constants.h"
#include <vector>
#include <random>
#include <iostream>



// ------------------ VERSION 6 -- QUIESCENCE -----------------------

void V6::orderMoves(std::vector<Move>& moveList, Board& board)
{
	int numMoves = moveList.size();

	if (numMoves > 1)
	{
		int i, j;
		double key;
		Move comparing;
		double score;

		std::vector<double> scores;

		for (i = 0; i < numMoves; i++)
		{
			score = 0;
			comparing = moveList.at(i);

			score += comparing.Equals(bestMove) ? 10000 : 0;

			score += comparing.isPromotion() ? 900 : 0;

			if (comparing.isCapture())
			{
				score += Constants::PIECE_VALUES[comparing.CapturePieceType] - Constants::PIECE_VALUES[comparing.MovePieceType] / 10;
			}
			else
			{
				score += ((double) std::rand() / (RAND_MAX));
			}

			score -= board.attackedByEnemyPawns(comparing.getTargetSquare()) ? 100 : 0;

			scores.push_back(score);
			//std::cout << scores.at(i) << " ";
		}

		//std::cout << std::endl;

		for (i = 1; i < numMoves; i++)
		{
			key = scores.at(i);
			comparing = moveList.at(i);
			j = i - 1;

			while (j >= 0 && scores.at(j) < key)
			{
				scores.at(j + 1) = scores.at(j);
				moveList.at(j + 1) = moveList.at(j);
				j--;
			}
			scores.at(j + 1) = key;
			moveList.at(j + 1) = comparing;
		}

		/*for (i = 0; i < numMoves; i++)
		{
			std::cout << scores.at(i) << " " << moveList.at(i) << std::endl;
		}

		std::cout << std::endl;*/

	}
}

int V6::quiesce(Board& board, int turnMult, int alpha, int beta)
{
	if (turnTimer.elapsed() > turnTime)
	{
		timedOut = true;
		return beta;
	}

	positionsSearched++;

	int currEval = turnMult * (board.scoreMaterial() + board.pieceSquareScore());

	//std::cout << currEval << " " << alpha << " " << beta << std::endl;

	if (currEval >= beta)
	{
		return beta;
	}

	if (currEval > alpha)
	{
		alpha = currEval;
	}

	std::vector<Move> captures = board.getLegalMoves(true);

	V6::orderMoves(captures, board);

	int score;

	for (int i = 0; i < captures.size(); i++)
	{
		board.MakeMove(captures.at(i));
		score = -V6::quiesce(board, -turnMult, -beta, -alpha);
		board.Undo();

		//std::cout << captures.at(i) << std::endl;
		//std::cout << score << " " << alpha << " " << beta << std::endl;

		if (score >= beta)
		{
			return beta;
		}

		if (score > alpha)
		{
			alpha = score;
		}
	}

	//std::cout << "quiescence search is returning " << alpha << std::endl;
	return alpha;
}

int V6::negaMax(Board& board, int turnMult, int depth, int alpha, int beta)
{
	if (turnTimer.elapsed() > turnTime)
	{
		timedOut = true;
		return beta;
	}

	positionsSearched++;

	if (depth == 0)
	{
		//return turnMult * (board.scoreMaterial() + board.pieceSquareScore());
		return V6::quiesce(board, turnMult, alpha, beta);
	}

	std::vector<Move> allMoves = board.getLegalMoves();

	V6::orderMoves(allMoves, board);

	if (allMoves.size() == 0)
	{
		int mate = board.isCheckmate();
		if (mate != 0)
		{
			return -turnMult * mate * Constants::CHECKMATE;
		}
		else
		{
			return 0;
		}
	}

	int maxScore = -Constants::CHECKMATE;
	int score;

	for (int i = 0; i < allMoves.size(); i++)
	{
		board.MakeMove(allMoves.at(i));
		score = -V6::negaMax(board, -turnMult, depth - 1, -beta, -alpha);
		board.Undo();

		/*for (int j = 0; j < maxDepth - depth; j++)
		{
			std::cout << "\t";
		}

		std::cout << allMoves.at(i) << std::endl;

		for (int j = 0; j < maxDepth - depth; j++)
		{
			std::cout << "\t";
		}

		std::cout << maxScore << " " << score << " " << alpha << " " << beta << std::endl;*/

		if (score > maxScore)
		{
			maxScore = score;
			if (depth == maxDepth && !timedOut)
			{
				thisBestMove = allMoves.at(i);
				thisBestScore = score;
				if (score == Constants::CHECKMATE)
				{
					mateFound = true;
				}
			}
		}

		if (maxScore > alpha)
		{
			alpha = maxScore;
		}

		if (alpha >= beta)
		{
			break;
		}
	}

	return maxScore;
}

Move V6::getMove(Board& board, int millisecondsRemaining, bool display)
{
	positionsSearched = 0;

	timedOut = false;
	//turnTime = millisecondsRemaining / 30;
	turnTime = 100;
	turnTimer.reset();

	maxDepth = 1;
	int mult = board.isWhiteToMove() ? 1 : -1;

	Timer t;

	while (!timedOut && maxDepth <= absMax) 
	{
		V6::negaMax(board, mult, maxDepth, -Constants::CHECKMATE, Constants::CHECKMATE);
		if (!timedOut)
		{
			bestMove = thisBestMove;
			bestScore = thisBestScore;
		}

		if (display)
		{
			if (timedOut) std::cout << "(PARTIAL) ";
			std::cout << "Depth " << maxDepth << ": " << positionsSearched << " positions in " << t.elapsed() << " milliseconds -- " << (static_cast<double>(1000 * positionsSearched) / t.elapsed()) << " nps" << std::endl;
			std::cout << "\tBest move: " << thisBestMove << " with score " << thisBestScore << std::endl;
		}

		maxDepth++;
	} 

	if (display)
	{
		std::cout << std::endl;
	}

	return thisBestMove;
}

// ------------------------------------------------------------------


// ------------------- VERSION 5 -- PSTS ----------------------------

void V5::orderMoves(std::vector<Move>& moveList, Board& board)
{
	int numMoves = moveList.size();

	if (numMoves > 1)
	{
		int i, j;
		double key;
		Move comparing;
		double score;

		std::vector<double> scores;

		for (i = 0; i < numMoves; i++)
		{
			score = 0;
			comparing = moveList.at(i);

			score += comparing.Equals(bestMove) ? 10000 : 0;

			score += comparing.isPromotion() ? 900 : 0;

			if (comparing.isCapture())
			{
				score += Constants::PIECE_VALUES[comparing.CapturePieceType] - Constants::PIECE_VALUES[comparing.MovePieceType] / 10;
			}
			else
			{
				score += ((double) std::rand() / (RAND_MAX));
			}

			score -= board.attackedByEnemyPawns(comparing.getTargetSquare()) ? 100 : 0;

			scores.push_back(score);
			//std::cout << scores.at(i) << " ";
		}

		//std::cout << std::endl;

		for (i = 1; i < numMoves; i++)
		{
			key = scores.at(i);
			comparing = moveList.at(i);
			j = i - 1;

			while (j >= 0 && scores.at(j) < key)
			{
				scores.at(j + 1) = scores.at(j);
				moveList.at(j + 1) = moveList.at(j);
				j--;
			}
			scores.at(j + 1) = key;
			moveList.at(j + 1) = comparing;
		}

		/*for (i = 0; i < numMoves; i++)
		{
			std::cout << scores.at(i) << " " << moveList.at(i) << std::endl;
		}

		std::cout << std::endl;*/

	}
}

int V5::negaMax(Board& board, int turnMult, int depth, int alpha, int beta)
{
	if (turnTimer.elapsed() > turnTime)
	{
		timedOut = true;
		return beta;
	}

	positionsSearched++;

	if (depth == 0)
	{
		return turnMult * (board.scoreMaterial() + board.pieceSquareScore());
	}

	std::vector<Move> allMoves = board.getLegalMoves();

	V5::orderMoves(allMoves, board);

	if (allMoves.size() == 0)
	{
		int mate = board.isCheckmate();
		if (mate != 0)
		{
			return -turnMult * mate * Constants::CHECKMATE;
		}
		else
		{
			return 0;
		}
	}

	int maxScore = -Constants::CHECKMATE;
	int score;

	for (int i = 0; i < allMoves.size(); i++)
	{
		board.MakeMove(allMoves.at(i));
		score = -V5::negaMax(board, -turnMult, depth - 1, -beta, -alpha);
		board.Undo();

		if (score > maxScore)
		{
			maxScore = score;
			if (depth == maxDepth && !timedOut)
			{
				thisBestMove = allMoves.at(i);
				thisBestScore = score;
				if (score == Constants::CHECKMATE)
				{
					mateFound = true;
				}
			}
		}

		if (maxScore > alpha)
		{
			alpha = maxScore;
		}

		if (alpha >= beta)
		{
			break;
		}
	}

	return maxScore;
}

Move V5::getMove(Board& board, int millisecondsRemaining, bool display)
{
	positionsSearched = 0;

	timedOut = false;
	//turnTime = millisecondsRemaining / 30;
	turnTime = 100;
	turnTimer.reset();

	maxDepth = 1;
	int mult = board.isWhiteToMove() ? 1 : -1;

	Timer t;

	while (!timedOut && maxDepth <= absMax) 
	{
		V5::negaMax(board, mult, maxDepth, -Constants::CHECKMATE, Constants::CHECKMATE);
		if (!timedOut)
		{
			bestMove = thisBestMove;
			bestScore = thisBestScore;
		}

		if (display)
		{
			if (timedOut) std::cout << "(PARTIAL) ";
			std::cout << "Depth " << maxDepth << ": " << positionsSearched << " positions in " << t.elapsed() << " milliseconds -- " << (static_cast<double>(1000 * positionsSearched) / t.elapsed()) << " nps" << std::endl;
			std::cout << "\tBest move: " << thisBestMove << " with score " << thisBestScore << std::endl;
		}

		maxDepth++;
	} 

	if (display)
	{
		std::cout << std::endl;
	}

	return thisBestMove;
}

// ------------------------------------------------------------------


// ------------------- VERSION 4 -- DEEPENING -----------------------

void V4::orderMoves(std::vector<Move>& moveList, Board& board)
{
	int numMoves = moveList.size();

	if (numMoves > 1)
	{
		int i, j;
		double key;
		Move comparing;
		double score;

		std::vector<double> scores;

		for (i = 0; i < numMoves; i++)
		{
			score = 0;
			comparing = moveList.at(i);

			score += comparing.Equals(bestMove) ? 10000 : 0;

			score += comparing.isPromotion() ? 900 : 0;

			if (comparing.isCapture())
			{
				score += Constants::PIECE_VALUES[comparing.CapturePieceType] - Constants::PIECE_VALUES[comparing.MovePieceType] / 10;
			}
			else
			{
				score += ((double) std::rand() / (RAND_MAX));
			}

			score -= board.attackedByEnemyPawns(comparing.getTargetSquare()) ? 100 : 0;

			scores.push_back(score);
			//std::cout << scores.at(i) << " ";
		}

		//std::cout << std::endl;

		for (i = 1; i < numMoves; i++)
		{
			key = scores.at(i);
			comparing = moveList.at(i);
			j = i - 1;

			while (j >= 0 && scores.at(j) < key)
			{
				scores.at(j + 1) = scores.at(j);
				moveList.at(j + 1) = moveList.at(j);
				j--;
			}
			scores.at(j + 1) = key;
			moveList.at(j + 1) = comparing;
		}

		/*for (i = 0; i < numMoves; i++)
		{
			std::cout << scores.at(i) << " " << moveList.at(i) << std::endl;
		}

		std::cout << std::endl;*/

	}
}

int V4::negaMax(Board& board, int turnMult, int depth, int alpha, int beta)
{
	if (turnTimer.elapsed() > turnTime)
	{
		timedOut = true;
		return beta;
	}

	positionsSearched++;

	if (depth == 0)
	{
		return turnMult * board.scoreMaterial();
	}

	std::vector<Move> allMoves = board.getLegalMoves();

	V4::orderMoves(allMoves, board);

	if (allMoves.size() == 0)
	{
		int mate = board.isCheckmate();
		if (mate != 0)
		{
			return -turnMult * mate * Constants::CHECKMATE;
		}
		else
		{
			return 0;
		}
	}

	int maxScore = -Constants::CHECKMATE;
	int score;

	for (int i = 0; i < allMoves.size(); i++)
	{
		board.MakeMove(allMoves.at(i));
		score = -V4::negaMax(board, -turnMult, depth - 1, -beta, -alpha);
		board.Undo();

		if (score > maxScore)
		{
			maxScore = score;
			if (depth == maxDepth && !timedOut)
			{
				thisBestMove = allMoves.at(i);
				thisBestScore = score;
				if (score == Constants::CHECKMATE)
				{
					mateFound = true;
				}
			}
		}

		if (maxScore > alpha)
		{
			alpha = maxScore;
		}

		if (alpha >= beta)
		{
			break;
		}
	}

	return maxScore;
}

Move V4::getMove(Board& board, int millisecondsRemaining, bool display)
{
	positionsSearched = 0;

	timedOut = false;
	//turnTime = millisecondsRemaining / 30;
	turnTime = 100;
	turnTimer.reset();

	maxDepth = 1;
	int mult = board.isWhiteToMove() ? 1 : -1;

	Timer t;

	while (!timedOut && maxDepth <= absMax) 
	{
		V4::negaMax(board, mult, maxDepth, -Constants::CHECKMATE, Constants::CHECKMATE);
		if (!timedOut)
		{
			bestMove = thisBestMove;
			bestScore = thisBestScore;
		}

		if (display)
		{
			if (timedOut) std::cout << "(PARTIAL) ";
			std::cout << "Depth " << maxDepth << ": " << positionsSearched << " positions in " << t.elapsed() << " milliseconds -- " << (static_cast<double>(1000 * positionsSearched) / t.elapsed()) << " nps" << std::endl;
			std::cout << "\tBest move: " << thisBestMove << " with score " << thisBestScore << std::endl;
		}

		maxDepth++;
	} 

	if (display)
	{
		std::cout << std::endl;
	}

	return thisBestMove;
}

// ------------------------------------------------------------------


// ---------------- VERSION 3 -- MOVE ORDERING ----------------------

void V3::orderMoves(std::vector<Move>& moveList, Board& board)
{
	int numMoves = moveList.size();

	if (numMoves > 1)
	{
		int i, j;
		double key;
		Move comparing;
		double score;

		std::vector<double> scores;

		for (i = 0; i < numMoves; i++)
		{
			score = 0;
			comparing = moveList.at(i);

			score += comparing.isPromotion() ? 900 : 0;

			score -= board.attackedByEnemyPawns(comparing.getTargetSquare()) ? 100 : 0;

			if (comparing.isCapture())
			{
				score += Constants::PIECE_VALUES[comparing.CapturePieceType] - Constants::PIECE_VALUES[comparing.MovePieceType] / 10;
			}
			else
			{
				score += ((double) std::rand() / (RAND_MAX));
			}

			scores.push_back(score);
			//std::cout << scores.at(i) << " ";
		}

		//std::cout << std::endl;

		for (i = 1; i < numMoves; i++)
		{
			key = scores.at(i);
			comparing = moveList.at(i);
			j = i - 1;

			while (j >= 0 && scores.at(j) < key)
			{
				scores.at(j + 1) = scores.at(j);
				moveList.at(j + 1) = moveList.at(j);
				j--;
			}
			scores.at(j + 1) = key;
			moveList.at(j + 1) = comparing;
		}

		/*for (i = 0; i < numMoves; i++)
		{
			std::cout << scores.at(i) << " " << moveList.at(i) << std::endl;
		}

		std::cout << std::endl;*/

	}
}

int V3::negaMax(Board& board, int turnMult, int depth, int alpha, int beta)
{
	if (turnTimer.elapsed() > turnTime)
	{
		timedOut = true;
		return beta;
	}

	positionsSearched++;

	if (depth == 0)
	{
		return turnMult * board.scoreMaterial();
	}

	std::vector<Move> allMoves = board.getLegalMoves();

	V3::orderMoves(allMoves, board);

	if (allMoves.size() == 0)
	{
		int mate = board.isCheckmate();
		if (mate != 0)
		{
			return -turnMult * mate * Constants::CHECKMATE;
		}
		else
		{
			return 0;
		}
	}

	int maxScore = -Constants::CHECKMATE;
	int score;

	for (int i = 0; i < allMoves.size(); i++)
	{
		board.MakeMove(allMoves.at(i));
		score = -V3::negaMax(board, -turnMult, depth - 1, -beta, -alpha);
		board.Undo();

		if (score > maxScore)
		{
			maxScore = score;
			if (depth == maxDepth && !timedOut)
			{
				thisBestMove = allMoves.at(i);
				thisBestScore = score;
				if (score == Constants::CHECKMATE)
				{
					mateFound = true;
				}
			}
		}

		if (maxScore > alpha)
		{
			alpha = maxScore;
		}

		if (alpha >= beta)
		{
			break;
		}
	}

	return maxScore;
}

Move V3::getMove(Board& board, int millisecondsRemaining, bool display)
{
	positionsSearched = 0;

	timedOut = false;
	//turnTime = millisecondsRemaining / 30;
	turnTime = 100;
	turnTimer.reset();

	maxDepth = 1;
	int mult = board.isWhiteToMove() ? 1 : -1;

	Timer t;

	while (!timedOut && maxDepth <= absMax) 
	{
		V3::negaMax(board, mult, maxDepth, -Constants::CHECKMATE, Constants::CHECKMATE);
		if (!timedOut)
		{
			bestMove = thisBestMove;
			bestScore = thisBestScore;

			if (display)
			{
				std::cout << "Depth " << maxDepth << ": " << positionsSearched << " positions in " << t.elapsed() << " milliseconds -- " << (static_cast<double>(1000 * positionsSearched) / t.elapsed()) << " nps" << std::endl;
			}

			maxDepth++;
		}
	} 

	if (display)
	{
		std::cout << std::endl;
	}

	return bestMove;
}

// ------------------------------------------------------------------


// ----------------- VERSION 2 -- ALPHA BETA ------------------------

void V2::orderMoves(std::vector<Move>& moveList)
{
	int numMoves = moveList.size();

	if (numMoves > 1)
	{
		int i, j;
		double key;
		Move comparing;

		std::vector<double> scores;

		for (i = 0; i < numMoves; i++)
		{
			scores.push_back(((double) std::rand() / (RAND_MAX)));
			//std::cout << scores.at(i) << " ";
		}

		//std::cout << std::endl;

		for (i = 1; i < numMoves; i++)
		{
			key = scores.at(i);
			comparing = moveList.at(i);
			j = i - 1;

			while (j >= 0 && scores.at(j) < key)
			{
				scores.at(j + 1) = scores.at(j);
				moveList.at(j + 1) = moveList.at(j);
				j--;
			}
			scores.at(j + 1) = key;
			moveList.at(j + 1) = comparing;
		}

		/*for (i = 0; i < numMoves; i++)
		{
			std::cout << scores.at(i) << " " << moveList.at(i) << std::endl;
		}

		std::cout << std::endl;*/

	}
}

int V2::negaMax(Board& board, int turnMult, int depth, int alpha, int beta)
{
	if (turnTimer.elapsed() > turnTime)
	{
		timedOut = true;
		return beta;
	}

	positionsSearched++;

	if (depth == 0)
	{
		return turnMult * board.scoreMaterial();
	}

	std::vector<Move> allMoves = board.getLegalMoves();

	V2::orderMoves(allMoves);

	if (allMoves.size() == 0)
	{
		int mate = board.isCheckmate();
		if (mate != 0)
		{
			return -turnMult * mate * Constants::CHECKMATE;
		}
		else
		{
			return 0;
		}
	}

	int maxScore = -Constants::CHECKMATE;
	int score;

	for (int i = 0; i < allMoves.size(); i++)
	{
		board.MakeMove(allMoves.at(i));
		score = -V2::negaMax(board, -turnMult, depth - 1, -beta, -alpha);
		board.Undo();

		if (score > maxScore)
		{
			maxScore = score;
			if (depth == maxDepth)
			{
				thisBestMove = allMoves.at(i);
				thisBestScore = score;
			}
		}

		if (maxScore > alpha)
		{
			alpha = maxScore;
		}

		if (alpha >= beta)
		{
			break;
		}
	}

	return maxScore;
}

Move V2::getMove(Board& board, int millisecondsRemaining, bool display)
{
	positionsSearched = 0;

	timedOut = false;
	//turnTime = millisecondsRemaining / 30;
	turnTime = 100;
	turnTimer.reset();

	maxDepth = 1;
	int mult = board.isWhiteToMove() ? 1 : -1;

	Timer t;

	while (!timedOut) 
	{
		V2::negaMax(board, mult, maxDepth, -Constants::CHECKMATE, Constants::CHECKMATE);
		if (!timedOut)
		{
			bestMove = thisBestMove;
			bestScore = thisBestScore;

			if (display)
			{
				std::cout << "Depth " << maxDepth << ": " << positionsSearched << " positions in " << t.elapsed() << " milliseconds -- " << (static_cast<double>(1000 * positionsSearched) / t.elapsed()) << " nps" << std::endl;
			}

			maxDepth++;
		}
	} 

	if (display)
	{
		std::cout << std::endl;
	}

	return bestMove;
}

// ------------------------------------------------------------------


// ----------------- VERSION 1 -- NEGAMAX --------------------------

void V1::orderMoves(std::vector<Move>& moveList)
{
	int numMoves = moveList.size();

	if (numMoves > 1)
	{
		int i, j;
		double key;
		Move comparing;

		std::vector<double> scores;

		for (i = 0; i < numMoves; i++)
		{
			scores.push_back(((double) std::rand() / (RAND_MAX)));
			//std::cout << scores.at(i) << " ";
		}

		//std::cout << std::endl;

		for (i = 1; i < numMoves; i++)
		{
			key = scores.at(i);
			comparing = moveList.at(i);
			j = i - 1;

			while (j >= 0 && scores.at(j) < key)
			{
				scores.at(j + 1) = scores.at(j);
				moveList.at(j + 1) = moveList.at(j);
				j--;
			}
			scores.at(j + 1) = key;
			moveList.at(j + 1) = comparing;
		}

		/*for (i = 0; i < numMoves; i++)
		{
			std::cout << scores.at(i) << " " << moveList.at(i) << std::endl;
		}

		std::cout << std::endl;*/

	}
}

int V1::negaMax(Board& board, int turnMult, int depth)
{
	if (turnTimer.elapsed() > turnTime)
	{
		timedOut = true;
		return 0;
	}

	positionsSearched++;

	if (depth == 0)
	{
		return turnMult * board.scoreMaterial();
	}

	std::vector<Move> allMoves = board.getLegalMoves();

	V1::orderMoves(allMoves);

	if (allMoves.size() == 0)
	{
		int mate = board.isCheckmate();
		if (mate != 0)
		{
			return -turnMult * mate * Constants::CHECKMATE;
		}
		else
		{
			return 0;
		}
	}

	int maxScore = -Constants::CHECKMATE;
	int score;

	for (int i = 0; i < allMoves.size(); i++)
	{
		board.MakeMove(allMoves.at(i));
		score = -V1::negaMax(board, -turnMult, depth - 1);
		board.Undo();

		if (score > maxScore)
		{
			maxScore = score;
			if (depth == maxDepth)
			{
				thisBestMove = allMoves.at(i);
				thisBestScore = score;
			}
		}
	}

	return maxScore;
}

Move V1::getMove(Board& board, int millisecondsRemaining, bool display)
{
	positionsSearched = 0;

	timedOut = false;
	turnTime = millisecondsRemaining / 30;
	//turnTime = 10000;
	turnTimer.reset();

	maxDepth = 1;
	int mult = board.isWhiteToMove() ? 1 : -1;

	Timer t;

	while (!timedOut) 
	{
		V1::negaMax(board, mult, maxDepth);
		if (!timedOut)
		{
			bestMove = thisBestMove;
			bestScore = thisBestScore;

			if (display)
			{
				std::cout << "Depth " << maxDepth << ": " << positionsSearched << " positions in " << t.elapsed() << " milliseconds -- " << (static_cast<double>(1000 * positionsSearched) / t.elapsed()) << " nps" << std::endl;
			}

			maxDepth++;
		}
	} 

	if (display)
	{
		std::cout << std::endl;
	}

	return bestMove;
}

// ------------------------------------------------------------------



// ------------------ VERSION 0 -- RANDOM ---------------------------

Move V0::getMove(Board& board)
{
	std::vector<Move> legalMoves = board.getLegalMoves();

	double r = ((double) std::rand() / (RAND_MAX));
	int index = std::floor(r * legalMoves.size());

	return legalMoves.at(index);
}

// ------------------------------------------------------------------