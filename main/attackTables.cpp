#include "bitboards.h"
#include "constants.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <random>

const ulong A_FILE = 72340172838076673;
const ulong B_FILE = 144680345676153346;
const ulong G_FILE = 4629771061636907072;
const ulong H_FILE = 9259542123273814144;

const ulong RANK_8 = 255;
const ulong RANK_1 = 18374686479671623680;

int main()
{
	ulong singlePiece;
	ulong attacks;

	// white pawn forwards
	for (int i = 0; i < 64; i++)
	{
		int file = i % 8;
		int rank = 7 - (i - file) / 8;

		attacks = RANK_1 >> 8 * (rank + 1);

		if (rank == 0 || rank == 7)
		{
			attacks = 0;
		} 
		else if (rank == 1)
		{
			attacks |= attacks >> 8;
		}

		attacks &= (A_FILE << file);

		//std::cout << std::bitset<64>(attacks) << std::endl;
		std::cout << attacks << "ULL," << std::endl;
	}

	// black pawn forwards
	for (int i = 0; i < 64; i++)
	{
		int file = i % 8;
		int rank = 7 - (i - file) / 8;

		attacks = RANK_1 >> 8 * (rank - 1);

		if (rank == 0 || rank == 7)
		{
			attacks = 0;
		} 
		else if (rank == 6)
		{
			attacks |= attacks << 8;
		}

		attacks &= (A_FILE << file);

		//std::cout << std::bitset<64>(attacks) << std::endl;
		//std::cout << attacks << "ULL," << std::endl;
	}

	// white pawns
	for (int i = 0; i < 64; i++)
	{
		singlePiece = 0;
		SetSquare(singlePiece, i);

		attacks = (
			(singlePiece >> 7 & ~A_FILE) |
			(singlePiece >> 9 & ~H_FILE)
		);

		//std::cout << std::bitset<64>(attacks) << std::endl;
		//std::cout << attacks << "ULL," << std::endl;
	}

	// black pawns
	for (int i = 0; i < 64; i++)
	{
		singlePiece = 0;
		SetSquare(singlePiece, i);

		attacks = (
			(singlePiece << 7 & ~H_FILE) |
			(singlePiece << 9 & ~A_FILE)
		);

		//std::cout << std::bitset<64>(attacks) << std::endl;
		//std::cout << attacks << "ULL," << std::endl;
	}

	// knights
	for (int i = 0; i < 64; i++)
	{
		singlePiece = 0;
		SetSquare(singlePiece, i);

		attacks = (
			((singlePiece << 6 | singlePiece >> 10) & ~(G_FILE | H_FILE)) |
			((singlePiece << 10 | singlePiece >> 6) & ~(A_FILE | B_FILE)) |
			((singlePiece << 15 | singlePiece >> 17) & ~H_FILE) |
			((singlePiece << 17 | singlePiece >> 15) & ~A_FILE)
		);

		//std::cout << std::bitset<64>(attacks) << std::endl;
		//std::cout << attacks << "ULL," << std::endl;
	}

	// kings
	for (int i = 0; i < 64; i++)
	{
		singlePiece = 0;
		SetSquare(singlePiece, i);

		attacks = (
			((singlePiece << 1 | singlePiece << 9 | singlePiece >> 7) & ~A_FILE) |
			((singlePiece >> 1 | singlePiece >> 9 | singlePiece << 7) & ~H_FILE) |
			(singlePiece >> 8 | singlePiece << 8)
		);

		//std::cout << std::bitset<64>(attacks) << std::endl;
		//std::cout << attacks << "ULL," << std::endl;
	}

	// rook masks
	for (int i = 0; i < 64; i++)
	{
		attacks = 0;

		ulong mask = 0;

		int file = (i % 8);
		int rank = (i - file);

		if (file > 0)
		{
			mask |= A_FILE;
		}

		if (file < 7)
		{
			mask |= H_FILE;
		}

		if (rank > 0)
		{
			mask |= RANK_8;
		}

		if (rank < 56)
		{
			mask |= RANK_1;
		}

		attacks = (A_FILE << file | RANK_8 << rank) & ~mask;
		ClearSquare(attacks, i);

		//std::cout << std::bitset<64>(attacks) << std::endl;
		//std::cout << attacks << "ULL," << std::endl;
	}

	// bishop masks
	for (int i = 0; i < 64; i++)
	{
		attacks = 0; 
		singlePiece = 0;
		SetSquare(singlePiece, i);
		int file = (i % 8);

		ulong mask = RANK_8 | RANK_1;

		// eastern rays
		for (int j = 1; j < 7 - file; j++)
		{
			attacks |= (singlePiece >> 7 * j);
			attacks |= (singlePiece << 9 * j);
		}

		// western rays
		for (int j = 1; j < file; j++)
		{
			//std::cout << "here" << std::endl;
			attacks |= (singlePiece << 7 * j);
			attacks |= (singlePiece >> 9 * j);
		}

		attacks &= ~mask;

		//std::cout << std::bitset<64>(attacks) << std::endl;
		//std::cout << attacks << "ULL," << std::endl;
	}

	ulong mask;
	ulong blockers;
	ulong moves;

	/*
	// generate all bishop blocker patterns + corresponding move bitboards
	ulong bishop_blockers[64][512];
	ulong bishop_moves[64][512];
	ulong bishop_table_sizes[64];

	int thisRoundIndex;

	for (int i = 0; i < 64; i++)
	{
		thisRoundIndex = 0;
		mask = Constants::BISHOP_MASKS[i];
		blockers = 0;

		do
		{
			bishop_blockers[i][thisRoundIndex] = blockers;

			// figure out the move bitboard
			moves = 0;
			singlePiece = 0;
			SetSquare(singlePiece, i);

			// northwest
			for (int j = 1; j <= 7; j++)
			{
				if (((singlePiece >> 9 * j) & H_FILE) != 0)
				{
					// wrapped around the edge
					break;
				}

				moves |= (singlePiece >> 9 * j);

				if (((singlePiece >> 9 * j) & blockers) != 0)
				{
					// hit a blocker
					break;
				}
			}

			// northeast
			for (int j = 1; j <= 7; j++)
			{
				if (((singlePiece >> 7 * j) & A_FILE) != 0)
				{
					// wrapped around the edge
					break;
				}

				moves |= (singlePiece >> 7 * j);

				if (((singlePiece >> 7 * j) & blockers) != 0)
				{
					// hit a blocker
					break;
				}
			}

			// southwest
			for (int j = 1; j <= 7; j++)
			{
				if (((singlePiece << 7 * j) & H_FILE) != 0)
				{
					// wrapped around the edge
					break;
				}

				moves |= (singlePiece << 7 * j);

				if (((singlePiece << 7 * j) & blockers) != 0)
				{
					// hit a blocker
					break;
				}
			}

			// southeast
			for (int j = 1; j <= 7; j++)
			{
				if (((singlePiece << 9 * j) & A_FILE) != 0)
				{
					// wrapped around the edge
					break;
				}

				moves |= (singlePiece << 9 * j);

				if (((singlePiece << 9 * j) & blockers) != 0)
				{
					// hit a blocker
					break;
				}
			}

			bishop_moves[i][thisRoundIndex] = moves;

			//std::cout << i << " " << thisRoundIndex << " " << std::bitset<64>(blockers) << std::endl;
			//std::cout << std::bitset<64>(moves) << std::endl;
			//std::cout << blockers << "ULL," << std::endl;

			blockers = (blockers - mask) & mask;
			thisRoundIndex++;
		} while (blockers != 0);

		bishop_table_sizes[i] = thisRoundIndex;
	}


	//std::cout << std::bitset<64>(bishop_blockers[2][0]) << std::endl;
	//std::cout << std::bitset<64>(bishop_moves[2][0]) << std::endl;


	// random number generation
	std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
	std::mt19937_64 gen(rd()); //Use the 64-bit Mersenne Twister 19937 generator
	                           //and seed it with entropy.

	//Define the distribution, by default it goes from 0 to MAX(unsigned long long)
	//or what have you.
	std::uniform_int_distribution<unsigned long long> distr(0,0xFFFFFFFFFFFFFFFF);


	// generate magic numbers for bishops
	std::cout << "const ulong BISHOP_MAGICS[64] {" << std::endl;

	ulong bishop_magics[64];

	int max_index_size = 10;
	int table_size = pow(2, max_index_size);
	ulong candidate_magic;

	ulong test;

	bool noMagicFound = true;

	ulong testTable[table_size];
	bool prevSeen[table_size];

	for (int j = 0; j < 64; j++)
	{
		noMagicFound = true;

		while (noMagicFound)
		{
			bool badMagic = false;

			//bool indexSeen[table_size];

			candidate_magic = distr(gen) & distr(gen) & distr(gen);
			//candidate_magic = 18446740762247425535;

			for (int i = 0; i < 512; i++)
			{
				//std::cout << std::bitset<64>(bishop_blockers[0][i]) << std::endl;
				test = (bishop_blockers[j][i] * candidate_magic) >> (64 - max_index_size);
				//std::cout << test << std::endl;

				if (prevSeen[test])
				{
					if (testTable[i] != bishop_moves[j][i])
					{
						//std::cout << "bad candidate -- collision at index " << test << std::endl;
						badMagic = true;
						break;
					}
				} else 
				{
					testTable[test] = bishop_moves[j][i];
					prevSeen[test] = true;
				}
			}

			if (!badMagic) 
			{
				noMagicFound = false;
				std::cout << "\t" << candidate_magic << "ULL," << std::endl;
				bishop_magics[j] = candidate_magic;
			}

			for (int i = 0; i < table_size; i++)
			{
				testTable[i] = 0;
				prevSeen[i] = false;
			}
		}
	} 

	ulong bishop_magic_moves[64][table_size];

	// generate bishop move tables
	for (int i = 0; i < 64; i++)
	{
		//std::cout << bishop_blockers[i][0] << " " << bishop_moves[i][0] << std::endl;
		for (int j = 0; j < bishop_table_sizes[i]; j++)
		{

			int magicIndex = ((bishop_blockers[i][j] * bishop_magics[i]) >> (64 - max_index_size));
			bishop_magic_moves[i][magicIndex] = bishop_moves[i][j];
		}
		//std::cout << ((bishop_blockers[i][0] * bishop_magics[i]) >> (64 - 10)) << std::endl;
		std::cout << bishop_moves[i][0] << " " << bishop_magic_moves[i][0] << std::endl;
	}

	
	std::cout << "};\n\nconst ulong BISHOP_ATTACKS[64][1024] = {" << std::endl;

	for (int i = 0; i < 64; i++)
	{
		std::cout << "\t{" << std::endl;

		for (int j = 0; j < table_size; j++)
		{
			std::cout << "\t\t" << bishop_magic_moves[i][j] << "ULL," << std::endl;
		}

		std::cout << "\t}," << std::endl;
	}

	std::cout << "};" << std::endl;*/

	//13751243240004310174 -- 16 bits
	//901282893144525106 -- 15 bits
	//1157565912623112848 -- 14 bits
	//9223512847626539026 -- 10 bits
	//577023840786319392 -- 9 bits

	/*
	// a8 bishop magic test
	int max_index_size = 9;
	int table_size = pow(2, max_index_size);
	ulong candidate_magic;

	ulong test;

	bool noMagicFound = true;

	ulong a8TestTable[table_size];

	while (noMagicFound)
	{
		bool badMagic = false;

		//bool indexSeen[table_size];

		candidate_magic = distr(gen) & distr(gen) & distr(gen);
		//candidate_magic = 577023840786319392;

		for (int i = 0; i < sizeof(bishop_blockers[27]) / sizeof(*bishop_blockers[27]); i++)
		{
			//std::cout << std::bitset<64>(bishop_blockers[0][i]) << std::endl;
			test = (bishop_blockers[27][i] * candidate_magic) >> (64 - max_index_size);
			//std::cout << test << std::endl;

			if (a8TestTable[test] != 0)
			{
				if (a8TestTable[i] != bishop_moves[27][i])
				{
					std::cout << "bad candidate -- collision at index " << test << std::endl;
					badMagic = true;
					break;
				}
			} else 
			{
				a8TestTable[test] = bishop_moves[27][i];
			}
		}

		if (!badMagic) 
		{
			noMagicFound = false;
			std::cout << candidate_magic << std::endl;
		}

		for (int i = 0; i < table_size; i++)
		{
			a8TestTable[i] = 0;
		}
	}*/

	/*
	for (int i = 0; i < sizeof(bishop_blockers[0]) / sizeof(*bishop_blockers[0]); i++)
	{
		//std::cout << std::bitset<64>(bishop_blockers[0][i]) << std::endl;
		test = (bishop_blockers[0][i] * candidate_magic) >> (64 - 5);
		//std::cout << test << std::endl;

		if (indexSeen[test])
		{
			std::cout << "seen it: " << test << std::endl;
			if (a8TestTable[i] != bishop_moves[0][i])
			{
				std::cout << "bad candidate" << std::endl;
				break;
			}
		} else 
		{
			a8TestTable[test] = bishop_moves[0][i];
			indexSeen[test] = true;
		}
	}

	std::cout << candidate_magic << std::endl;*/


	// MAGICS FOR THE ROOKS

	// generate all rook blocker patterns + corresponding move bitboards

	/*
	ulong rook_blockers[64][4096];
	ulong rook_moves[64][4096];
	ulong rook_table_sizes[64];

	int thisRoundIndex;

	for (int i = 0; i < 64; i++)
	{
		thisRoundIndex = 0;
		mask = Constants::ROOK_MASKS[i];
		blockers = 0;

		do
		{
			rook_blockers[i][thisRoundIndex] = blockers;

			// figure out the move bitboard
			moves = 0;
			singlePiece = 0;
			SetSquare(singlePiece, i);

			// north
			for (int j = 1; j <= 7; j++)
			{
				moves |= (singlePiece >> 8 * j);

				if (((singlePiece >> 8 * j) & blockers) != 0)
				{
					// hit a blocker
					break;
				}
			}

			// west
			for (int j = 1; j <= 7; j++)
			{
				if (((singlePiece >> j) & H_FILE) != 0)
				{
					// wrapped around the edge
					break;
				}

				moves |= (singlePiece >> j);

				if (((singlePiece >> j) & blockers) != 0)
				{
					// hit a blocker
					break;
				}
			}

			// south
			for (int j = 1; j <= 7; j++)
			{
				moves |= (singlePiece << 8 * j);

				if (((singlePiece << 8 * j) & blockers) != 0)
				{
					// hit a blocker
					break;
				}
			}

			// east
			for (int j = 1; j <= 7; j++)
			{
				if (((singlePiece << j) & A_FILE) != 0)
				{
					// wrapped around the edge
					break;
				}

				moves |= (singlePiece << j);

				if (((singlePiece << j) & blockers) != 0)
				{
					// hit a blocker
					break;
				}
			}

			rook_moves[i][thisRoundIndex] = moves;

			//std::cout << i << " " << thisRoundIndex << " " << std::bitset<64>(blockers) << std::endl;
			//std::cout << std::bitset<64>(moves) << std::endl;
			//std::cout << blockers << "ULL," << std::endl;

			blockers = (blockers - mask) & mask;
			thisRoundIndex++;
		} while (blockers != 0);

		rook_table_sizes[i] = thisRoundIndex;
	}


	// random number generation
	std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
	std::mt19937_64 gen(rd()); //Use the 64-bit Mersenne Twister 19937 generator
	                           //and seed it with entropy.

	//Define the distribution, by default it goes from 0 to MAX(unsigned long long)
	//or what have you.
	std::uniform_int_distribution<unsigned long long> distr(0,0xFFFFFFFFFFFFFFFF);


	// generate magic numbers for rooks
	std::cout << "const ulong ROOK_MAGICS[64] {" << std::endl;

	ulong rook_magics[64];

	int max_index_size = 12;
	int table_size = pow(2, max_index_size);
	ulong candidate_magic;

	ulong test;

	bool noMagicFound = true;

	ulong testTable[table_size];
	bool prevSeen[table_size];

	for (int j = 0; j < 64; j++)
	{
		noMagicFound = true;

		while (noMagicFound)
		{
			bool badMagic = false;

			//bool indexSeen[table_size];

			candidate_magic = distr(gen) & distr(gen) & distr(gen);
			//candidate_magic = 18446740762247425535;

			for (int i = 0; i < rook_table_sizes[j]; i++)
			{
				//std::cout << std::bitset<64>(bishop_blockers[0][i]) << std::endl;
				test = (rook_blockers[j][i] * candidate_magic) >> (64 - max_index_size);
				//std::cout << test << std::endl;

				if (prevSeen[test])
				{
					if (testTable[i] != rook_moves[j][i])
					{
						//std::cout << "bad candidate -- collision at index " << test << std::endl;
						badMagic = true;
						break;
					}
				} else 
				{
					testTable[test] = rook_moves[j][i];
					prevSeen[test] = true;
				}
			}

			if (!badMagic) 
			{
				noMagicFound = false;
				std::cout << "\t" << candidate_magic << "ULL," << std::endl;
				rook_magics[j] = candidate_magic;
			}

			for (int i = 0; i < table_size; i++)
			{
				testTable[i] = 0;
				prevSeen[i] = false;
			}
		}
	} 

	ulong rook_magic_moves[64][table_size];

	// generate bishop move tables
	for (int i = 0; i < 64; i++)
	{
		//std::cout << bishop_blockers[i][0] << " " << bishop_moves[i][0] << std::endl;
		for (int j = 0; j < rook_table_sizes[i]; j++)
		{

			int magicIndex = ((rook_blockers[i][j] * rook_magics[i]) >> (64 - max_index_size));
			rook_magic_moves[i][magicIndex] = rook_moves[i][j];
		}
		//std::cout << ((bishop_blockers[i][0] * bishop_magics[i]) >> (64 - 10)) << std::endl;
		std::cout << rook_moves[i][0] << " " << rook_magic_moves[i][0] << std::endl;
	}

	
	std::cout << "};\n\nconst ulong ROOK_ATTACKS[64][4096] = {" << std::endl;

	for (int i = 0; i < 64; i++)
	{
		std::cout << "\t{" << std::endl;

		for (int j = 0; j < table_size; j++)
		{
			std::cout << "\t\t" << rook_magic_moves[i][j] << "ULL," << std::endl;
		}

		std::cout << "\t}," << std::endl;
	}

	std::cout << "};" << std::endl;*/
}