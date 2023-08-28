#ifndef CHESS_BOT_H
#define CHESS_BOT_H
#include "move.h"
#include "board.h"
#include "constants.h"
#include <chrono>

class Timer
{
private:
    // Type aliases to make accessing nested type easier
    using Clock = std::chrono::steady_clock;
    using Second = std::chrono::duration<double, std::milli>;

    std::chrono::time_point<Clock> m_beg{ Clock::now() };

public:

    void reset()
    {
        m_beg = Clock::now();
    }

    double elapsed() const
    {
        return std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
    }
};

struct Transposition
{
    ulong key;
    Move move;
    int depth, score, kind;
};


class V6
{

private:
	int versionNum = 6;

	Move bestMove;
	int bestScore;

	Move thisBestMove;
	int thisBestScore;

	int maxDepth;
	int absMax = 50;

	bool mateFound;

	ulong positionsSearched;

	int turnTime;
	Timer turnTimer;
	bool timedOut;

public:
	int getVersion() { return versionNum; }

	Move getMove(Board& board, int timeLeft, bool display = false);
	int negaMax(Board& board, int turnMult, int depth, int alpha, int beta);
	void orderMoves(std::vector<Move>& moveList, Board& board);
	int quiesce(Board& board, int turnMult, int alpha, int beta);
};


class V5
{

private:
	int versionNum = 5;

	Move bestMove;
	int bestScore;

	Move thisBestMove;
	int thisBestScore;

	int maxDepth;
	int absMax = 50;

	bool mateFound;

	ulong positionsSearched;

	int turnTime;
	Timer turnTimer;
	bool timedOut;

public:
	int getVersion() { return versionNum; }

	Move getMove(Board& board, int timeLeft, bool display = false);
	int negaMax(Board& board, int turnMult, int depth, int alpha, int beta);
	void orderMoves(std::vector<Move>& moveList, Board& board);
};



class V4
{

private:
	int versionNum = 4;

	Move bestMove;
	int bestScore;

	Move thisBestMove;
	int thisBestScore;

	int maxDepth;
	int absMax = 50;

	bool mateFound;

	ulong positionsSearched;

	int turnTime;
	Timer turnTimer;
	bool timedOut;

public:
	int getVersion() { return versionNum; }

	Move getMove(Board& board, int timeLeft, bool display = false);
	int negaMax(Board& board, int turnMult, int depth, int alpha, int beta);
	void orderMoves(std::vector<Move>& moveList, Board& board);
};


class V3
{

private:
	int versionNum = 3;

	Move bestMove;
	int bestScore;

	Move thisBestMove;
	int thisBestScore;

	int maxDepth;
	int absMax = 50;

	bool mateFound;

	ulong positionsSearched;

	int turnTime;
	Timer turnTimer;
	bool timedOut;

public:
	int getVersion() { return versionNum; }

	Move getMove(Board& board, int timeLeft, bool display = false);
	int negaMax(Board& board, int turnMult, int depth, int alpha, int beta);
	void orderMoves(std::vector<Move>& moveList, Board& board);
};


class V2
{

private:
	int versionNum = 2;

	Move bestMove;
	int bestScore;

	Move thisBestMove;
	int thisBestScore;

	int maxDepth;

	ulong positionsSearched;

	int turnTime;
	Timer turnTimer;
	bool timedOut;

public:
	int getVersion() { return versionNum; }

	Move getMove(Board& board, int timeLeft, bool display = false);
	int negaMax(Board& board, int turnMult, int depth, int alpha, int beta);
	void orderMoves(std::vector<Move>& moveList);
};


class V1
{

private:
	int versionNum = 1;

	Move bestMove;
	int bestScore;

	Move thisBestMove;
	int thisBestScore;

	int maxDepth;

	ulong positionsSearched;

	int turnTime;
	Timer turnTimer;
	bool timedOut;

public:
	int getVersion() { return versionNum; }

	Move getMove(Board& board, int timeLeft, bool display = false);
	int negaMax(Board& board, int turnMult, int depth);
	void orderMoves(std::vector<Move>& moveList);
};


class V0
{

private:
	int versionNum = 0;

public:
	int getVersion() { return versionNum; }

	Move getMove(Board& board);

};

#endif