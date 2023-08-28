#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include "main/all_headers.h"



ulong plainPerft(int depth, Board& board)
{
	ulong nodes = 0;

	//std::cout << depth << std::endl;

	std::vector<Move> legalMoves = board.getLegalMoves();

	ulong numMoves = legalMoves.size();

	if (depth == 1)
	{
		//std::cout << "returning " << numMoves << std::endl;
		return numMoves;
	}

	for (int i = 0; i < numMoves; i++)
	{
		board.MakeMove(legalMoves.at(i));
		nodes += plainPerft(depth - 1, board);
		board.Undo();
	}

	return nodes;
}

std::vector<ulong> perft(int depth, Board& board)
{
	std::vector<ulong> results(5, 0);

	std::vector<Move> legalMoves = board.getLegalMoves();

	ulong numMoves = legalMoves.size();

	if (depth == 1)
	{
		for (int i = 0; i < numMoves; i++)
		{
			results.at(0) += 1ULL;
			if (legalMoves.at(i).isCapture()) results.at(1) += 1ULL;
			if (legalMoves.at(i).isEnPassant()) 
			{
				results.at(2) += 1ULL;
				results.at(1) += 1ULL;
			}
			if (legalMoves.at(i).isCastles()) results.at(3) += 1ULL;
			if (legalMoves.at(i).isPromotion()) results.at(4) += 1ULL;
		}

		return results;
	}

	std::vector<ulong> temp;

	for (int i = 0; i < numMoves; i++)
	{
		board.MakeMove(legalMoves.at(i));
		temp = perft(depth - 1, board);
		for (int j = 0; j < 5; j++)
		{
			results.at(j) += temp.at(j);
		}
		board.Undo();
	}

	return results;
}

/*std::vector<ulong> pperft(int depth, Board& board)
{
	std::vector<ulong> results(5, 0);

	std::vector<Move> legalMoves = board.getPseudoLegalMoves();

	ulong numMoves = legalMoves.size();

	if (depth == 1)
	{
		for (int i = 0; i < numMoves; i++)
		{
			board.OldMake(legalMoves.at(i));
			if (!board.kingInCheck(!board.isWhiteToMove()))
			{
				results.at(0) += 1ULL;
				if (legalMoves.at(i).IsCapture) results.at(1) += 1ULL;
				if (legalMoves.at(i).IsEnPassant) results.at(2) += 1ULL;
				if (legalMoves.at(i).IsCastles) results.at(3) += 1ULL;
				if (legalMoves.at(i).IsPromotion) results.at(4) += 1ULL;
			}
			board.UndoMove(legalMoves.at(i));
		}

		return results;
	}

	std::vector<ulong> temp;

	for (int i = 0; i < numMoves; i++)
	{
		board.OldMake(legalMoves.at(i));
		if (!board.kingInCheck(!board.isWhiteToMove()))
		{
			temp = pperft(depth - 1, board);
			for (int j = 0; j < 5; j++)
			{
				results.at(j) += temp.at(j);
			}
		}
		board.UndoMove(legalMoves.at(i));
	}

	return results;
}*/

ulong perftExtra(int depth, Board& board, int maxDepth)
{
	std::vector<Move> legalMoves = board.getLegalMoves();

	ulong numMoves = legalMoves.size();

	ulong nodes = 0;
	ulong temp = 0;

	if (depth == 0)
	{
		return 1;
	}

	if (depth == 1 && depth != maxDepth)
	{
		return numMoves;
	}

	for (int i = 0; i < numMoves; i++)
	{
		board.MakeMove(legalMoves.at(i));

		temp = perftExtra(depth - 1, board, maxDepth);
		nodes += temp;

		if (depth == maxDepth)
		{
			std::cout << legalMoves.at(i) << ": " << temp << std::endl;
		}

		board.Undo();
	}

	return nodes;
}

/*
ulong pperftExtra(int depth, Board& board, int maxDepth)
{
	std::vector<Move> legalMoves = board.getPseudoLegalMoves();

	ulong numMoves = legalMoves.size();

	ulong nodes = 0;
	ulong temp = 0;

	if (depth == 0)
	{
		return 1;
	}

	if (depth == 1 && depth != maxDepth)
	{
		for (int i = 0; i < numMoves; i++)
		{
			board.OldMake(legalMoves.at(i));
			if (!board.kingInCheck(!board.isWhiteToMove()))
			{
				nodes += 1ULL;
				std::cout << "\t" << legalMoves.at(i) << std::endl;
			}
			board.UndoMove(legalMoves.at(i));
		}

		return nodes;
	}

	for (int i = 0; i < numMoves; i++)
	{
		board.OldMake(legalMoves.at(i));
		if (!board.kingInCheck(!board.isWhiteToMove()))
		{
			temp = pperftExtra(depth - 1, board, maxDepth);
			nodes += temp;
			if (depth == maxDepth)
			{
				std::cout << legalMoves.at(i) << ": " << temp << std::endl;
			}
		}
		board.UndoMove(legalMoves.at(i));
	}

	return nodes;
}
*/

void newMovePerft(Board& board)
{
	Timer t;
	std::vector<Move> moves = board.getLegalMoves();
	std::cout << "move gen took " << t.elapsed() << " milliseconds" << std::endl;
	for (int i = 0; i < moves.size(); i++)
	{
		std::cout << std::bitset<16>(moves.at(i).moveCode) << std::endl;
	}

	std::cout << "found " << moves.size() << " moves" << std::endl;

	t.reset();
	board.MakeMove(moves.at(0));
	std::cout << "took " << t.elapsed() << " milliseconds to make first move" << std::endl;
}

void fastSuite(std::vector<std::string> positions)
{
	for (int j = 0; j < positions.size(); j++)
	{
		Timer t;
		std::vector<ulong> results;
		Board board(positions.at(j));

		std::cout << "\n\nstarting test position " << j + 1 << "\n" << std::endl;

		std::cout << std::setw(6) << "Depth" << std::setw(15) << "Nodes" << std::setw(14) << "NPS";
		std::cout << std::setw(16) << "Captures" << std::setw(15) << "EPs"; std::cout << std::setw(15) << "Castles";
		std::cout << std::setw(15) << "Promotions" << std::endl;

		for (int i = 0; i < 5; i++)
		{
			results = perft(i + 1, board);
			std::cout << std::setw(4) << i + 1 << std::setw(17) << results.at(0);
			std::cout << std::setw(14) << (static_cast<double>(results.at(0)) / t.elapsed());
			std::cout << std::setw(16) << results.at(1) << std::setw(15) << results.at(2);
			std::cout << std::setw(15) << results.at(3) << std::setw(15) << results.at(4) << std::endl;
		}
	}
}

void fullSuite(std::vector<std::string> positions, std::vector<int> depths)
{
	for (int j = 0; j < positions.size(); j++)
	{
		Timer t;
		std::vector<ulong> results;
		Board board(positions.at(j));

		std::cout << "\n\nstarting test position " << j + 1 << "\n" << std::endl;

		std::cout << std::setw(6) << "Depth" << std::setw(15) << "Nodes" << std::setw(14) << "NPS";
		std::cout << std::setw(16) << "Captures" << std::setw(15) << "EPs"; std::cout << std::setw(15) << "Castles";
		std::cout << std::setw(15) << "Promotions" << std::endl;

		for (int i = 0; i < depths.at(j); i++)
		{
			results = perft(i + 1, board);
			std::cout << std::setw(4) << i + 1 << std::setw(17) << results.at(0);
			std::cout << std::setw(14) << 1000 * (static_cast<double>(results.at(0)) / t.elapsed());
			std::cout << std::setw(16) << results.at(1) << std::setw(15) << results.at(2);
			std::cout << std::setw(15) << results.at(3) << std::setw(15) << results.at(4) << std::endl;
		}
	}
}

int main()
{
	// Board board("rnbqk1nr/ppp2ppp/4p3/b2p4/2PP4/8/PP1BPPPP/RN1QKBNR w KQkq - 5 5"); -- testing pinned piece moves
	Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	std::vector<Move> legal = board.getLegalMoves();

	std::vector<std::string> testPositions;
	std::vector<int> maxDepths;

	testPositions.push_back("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	maxDepths.push_back(7);

	testPositions.push_back("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");
	maxDepths.push_back(6);

	testPositions.push_back("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");
	maxDepths.push_back(8);

	testPositions.push_back("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
	maxDepths.push_back(6);

	testPositions.push_back("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ");
	maxDepths.push_back(5);

	testPositions.push_back("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");
	maxDepths.push_back(6);

	//board.MakeMove(legal.at(4));


	//std::cout << board.getPieceTypeAtSquare(45, 0) << std::endl;

	//newMovePerft(board);

	//std::cout << perftExtra(6, board, 6) << std::endl;

	//fastSuite(testPositions);

	fullSuite(testPositions, maxDepths);

	//std::cout << plainPerft(3, board) << std::endl;

	/*
	Timer t;
	ulong nodes = 0;

	std::cout << std::setw(6) << "Depth" << std::setw(15) << "Nodes" << std::setw(14) << "NPS" << std::endl;

	for (int i = 0; i < 6; i++)
	{
		nodes = plainPerft((i + 1), board);
		std::cout << std::setw(4) << i + 1 << std::setw(17) << nodes;
		std::cout << std::setw(14) << (static_cast<double>(nodes) / t.elapsed()) << std::endl;
	}*/

	/*
	Timer t;
	std::vector<ulong> results;

	std::cout << std::setw(6) << "Depth" << std::setw(15) << "Nodes" << std::setw(14) << "NPS";
	std::cout << std::setw(16) << "Captures" << std::setw(15) << "EPs"; std::cout << std::setw(15) << "Castles";
	std::cout << std::setw(15) << "Promotions" << std::endl;

	for (int i = 0; i < 6; i++)
	{
		results = perft(i + 1, board);
		std::cout << std::setw(4) << i + 1 << std::setw(17) << results.at(0);
		std::cout << std::setw(14) << (static_cast<double>(results.at(0)) / t.elapsed());
		std::cout << std::setw(16) << results.at(1) << std::setw(15) << results.at(2);
		std::cout << std::setw(15) << results.at(3) << std::setw(15) << results.at(4) << std::endl;
	}*/

}