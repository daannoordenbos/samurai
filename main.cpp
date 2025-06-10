#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <cmath>

#include "bitboard.h"
#include "position.h"
#include "moveGenerator.h"
#include "random.h"
#include "search.h"
#include "learner.h"


void moveGeneratorTest()
{
    Position pos;
    std::ifstream file("testsfens.txt");
    std::string sfen;
    uint64_t total = 0;
    while (std::getline(file, sfen)) {
        pos.loadSFEN(sfen.c_str());
        uint64_t leaves = nearPerft(pos, 4);
        std::cout << leaves << "\n";
        total += leaves;
    }
    std::cout << "Total: " << total << "\n";

    file.close(); // Close the file
}



int main()
{
    Position test;
    initialiseBitboards();
    test.loadInitial();

    test.loadSFEN("ln3g1nl/1r1s2k2/ppp1ppspp/4g1p2/3N5/2PP1B3/PP2PPPPP/2G4R1/L1S1KGSNL b Pb 1");
    //Move move = test.USIToMove("4c5e");

    //staticExchangeValue(test, move);
    //return 0;
    //std::cout << test.playerOne << "\n";


    //Bitboard kiki[4];
    //test.kikiBitboards(kiki);
    //return 0;
    /*
    std::ifstream positions("takaniEvalFast.txt");

    std::string board;
    std::string turn;
    std::string hand;
    std::string ply;
    double score;

    std::ofstream outputFile("stableQueriedFast.txt");

    Position pos;
    double iters = 0;
    while (positions >> board >> turn >> hand >> ply >> score) {
        std::string sfen = board + " " + turn + " " + hand + " " + ply;
        pos.loadSFEN(sfen.c_str());
        std::string stabled = pos.regularFormat();
        outputFile << stabled << " " << (1 + tanh(score / 850)) / 2 << "\n";
    }
    outputFile.close();
    return 0;*/

    loadParameters();

    //loadParameters();
    engineMove(test, 100000);
    for (int iter = 0; iter < 1000; iter++)
    {

        test.print();
        std::string move;
        std::cout << "Enter move: ";
        std::cin >> move; // Reads a single word (stops at whitespace)
        Move playerTwo = test.USIToMove(move.c_str());
        test.makeMove(playerTwo);
        engineMove(test, 1000);
        //break;
    }

    return 0;

    counts();

    for (int iter = 0; iter < 1000; iter++)
        descent();
    return 0;
}
