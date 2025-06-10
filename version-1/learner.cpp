#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <map>

#include "bitboard.h"
#include "moveGenerator.h"
#include "random.h"
#include "learner.h"


// Rook, bishop, gold mover, silver, knight, lance, dragon bonus, horse bonus

double pieceValue       [16]        = {0};
double pieceInHand      [7][19]     = {0};

// 'Kiki' is the term for square control

double kikiKingSafety     [289][4][4] = {0};

double pieceSquareTable     [16][81] = {0};

int oneSquareMap     [81][81]    = {0};
int twoSquareMap     [81][81]    = {0};

double del_pieceValue   [16]        = {0};
double del_pieceInHand  [7][19]     = {0};
double del_kikiKingSafety [289][4][4] = {0};
double del_pieceSquareTable     [16][81] = {0};

double counts_pieceInHand[7][19] = {0};

double counts_kikiKingSafety[289][4][4] = {0};
double counts_pieceSquareTable     [16][81] = {0};
double totalPositions = 0;

double LL_2 = 0;

double evaluation(const Position& pos)
{
    double eval = 0;
    // compute kiki
    Bitboard kiki[4];
    pos.kikiBitboards(kiki);
    int k1 = (pos.pieceMaps[KING] &   pos.pieceMaps[8] ).BSF();
    int k2 = (pos.pieceMaps[KING] & (~pos.pieceMaps[8])).BSF();
    // evaluation
    for (int square = 0; square < 81; square++)
    {
        int piece = pos.mailbox[square];
        eval += (pos.pieceMaps[8] & squareMask[square] ?
                 pieceValue[piece] + pieceSquareTable[piece][square] :
                -pieceValue[piece] - pieceSquareTable[piece][81 - square]);

        int kiki_defence = (kiki[0] & squareMask[square] ? 1 : 0) + (kiki[1] & squareMask[square] ? 2 : 0);
        int kiki_attack  = (kiki[2] & squareMask[square] ? 1 : 0) + (kiki[3] & squareMask[square] ? 2 : 0);
        eval += kikiKingSafety[oneSquareMap[k1][square]][kiki_defence][kiki_attack];
        eval -= kikiKingSafety[twoSquareMap[k2][square]][kiki_attack][kiki_defence];
    }
    for (int i = 1; i < 8; i++)
    {
        eval += pieceInHand[i - 1][hand_count(pos.hand[true], i)];
        eval -= pieceInHand[i - 1][hand_count(pos.hand[false], i)];
    }



    return eval;
}


void partialEval(const Position& pos, double score)
{
    double eval = evaluation(pos);
    eval = std::min(5.0, std::max(-5.0, eval));
    double winProbability = 1 / (1 + exp(-eval));
    double LL_partial = score - winProbability;
    LL_2 += score * log(winProbability) + (1 - score) * log(1 - winProbability);

    // compute kiki
    Bitboard kiki[4];
    pos.kikiBitboards(kiki);
    int k1 = (pos.pieceMaps[KING] &   pos.pieceMaps[8] ).BSF();
    int k2 = (pos.pieceMaps[KING] & (~pos.pieceMaps[8])).BSF();

    for (int square = 0; square < 81; square++)
    {
        int piece = pos.mailbox[square];
        if (piece > 0 && pos.pieceMaps[8] & squareMask[square])
        {
            del_pieceValue[piece] += LL_partial;
            del_pieceSquareTable[piece][square] += LL_partial;
        }
        if (piece > 0 && (~pos.pieceMaps[8]) & squareMask[square])
        {
            del_pieceValue[piece] -= LL_partial;
            del_pieceSquareTable[piece][81 - square] -= LL_partial;
        }

        int kiki_defence = (kiki[0] & squareMask[square] ? 1 : 0) + (kiki[1] & squareMask[square] ? 2 : 0);
        int kiki_attack  = (kiki[2] & squareMask[square] ? 1 : 0) + (kiki[3] & squareMask[square] ? 2 : 0);
        del_kikiKingSafety[oneSquareMap[k1][square]][kiki_defence][kiki_attack] += LL_partial;
        del_kikiKingSafety[twoSquareMap[k2][square]][kiki_attack][kiki_defence] -= LL_partial;
    }
    for (int i = 1; i < 8; i++)
    {
        if (hand_count(pos.hand[true], i) > 0)
        {
            del_pieceInHand[i - 1][hand_count(pos.hand[true], i)] += LL_partial;
        }
        if (hand_count(pos.hand[false], i) > 0)
        {
            del_pieceInHand[i - 1][hand_count(pos.hand[false], i)] -= LL_partial;
        }
    }
}

void descent()
{
    std::ifstream positions("stableQueriedFast.txt");

    std::string board;
    int handOne, handTwo, side;
    double score;

    Position pos;
    double iters = 0;
    while (positions >> board >> handOne >> handTwo >> side >> score) {
        pos.loadRegularFormat(board, handOne, handTwo, side);
        partialEval(pos, score);
        iters += 1.0;

        if (iters > 100000)
        {
            std::cout << "Log Likelihood: " << -LL_2 / iters << "\n";
            evaluationStep(iters);
            iters = 0;
            LL_2 = 0;
        }
    }
    std::cout << "Log Likelihood: " << -LL_2 / iters << "\n";
    //evaluationStep_2(iters);
    positions.close();
    saveParameters();
    std::cout << "Cycle Saved\n";
    LL_2 = 0;


}

void evaluationStep(double samples)
{
    LL_2 = 0;
    double momentum = 0.9;
    double factor = 5 * samples / totalPositions;
/*
    for (int i = 0; i < 16; i++) {
        pieceValue_2[i] += eta * del_pieceValue_2[i];
        del_pieceValue_2[i] *= momentum;
    }

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 19; j++) {
            pieceInHand_2[i][j] += factor * del_pieceInHand_2[i][j] / (10 + counts_pieceInHand_2[i][j]);
            //pieceInHand_2[i][j] *= 0.99;
            del_pieceInHand_2[i][j] *= momentum;
        }
    }*/

    for (int i = 0; i < 287; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                kikiKingSafety[i][j][k] += factor * del_kikiKingSafety[i][j][k] / (10 + counts_kikiKingSafety[i][j][k]);
                kikiKingSafety[i][j][k] *= 0.99;
                del_kikiKingSafety[i][j][k] *= momentum;
            }
        }
    }

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 81; j++) {
            pieceSquareTable[i][j] += factor * del_pieceSquareTable[i][j] / (10 + counts_pieceSquareTable[i][j]);
            pieceSquareTable[i][j] *= 0.99;
            del_pieceSquareTable[i][j] *= momentum;

        }
    }


}

void saveParameters()
{
    std::ofstream outFile("parameters-10-06-2025.txt");
    if (outFile.is_open()) {

        for (int i = 0; i < 16; i++)
            outFile << pieceValue[i] << " ";

        for (int i = 0; i < 7; i++)
            for (int j = 0; j < 19; j++)
                outFile << pieceInHand[i][j] << " ";

        for (int i = 0; i < 287; i++)
            for (int j = 0; j < 4; j++)
                for (int k = 0; k < 4; k++)
                    outFile << kikiKingSafety[i][j][k] << " ";

        for (int i = 0; i < 16; i++)
            for (int j = 0 ; j < 81; j++)
                outFile << pieceSquareTable[i][j] << " ";

        outFile.close();
    }
}

void loadParameters()
{
    std::cout << "Loading parameters: ";
    // Compute relative square map
    for (int i = 0; i < 81; i++) {
        for (int j = 0; j < 81; j++) {
            oneSquareMap[i][j] = 8 + j % 9 - i % 9 + 17 * (8 + j / 9 - i / 9);
            twoSquareMap[i][j] = 8 + (80 - j) % 9 - (80 - i) % 9 + 17 * (8 + (80 - j) / 9 - (80 - i) / 9);
        }
    }

    std::ifstream inFile("parameters-09-06-2025.txt");
    if (inFile.is_open()) {

        for (int i = 0; i < 16; i++)
            inFile >> pieceValue[i];


        for (int i = 0; i < 7; i++)
            for (int j = 0; j < 19; j++)
                inFile >> pieceInHand[i][j];

        for (int i = 0; i < 287; i++)
            for (int j = 0; j < 4; j++)
                for (int k = 0; k < 4; k++)
                    inFile >> kikiKingSafety[i][j][k];

        for (int i = 0; i < 16; i++)
            for (int j = 0 ; j < 81; j++)
                inFile >> pieceSquareTable[i][j];

        inFile.close();
    }
    std::cout << "Completed\n";
}

void randomWeights()
{
    double scale = 0.0001;
    for (int i = 0; i < 16; i++)
        pieceValue[i] = (randomInteger() % 100) * scale;


    for (int i = 0; i < 7; i++)
        for (int j = 0; j < 19; j++)
            pieceInHand[i][j] = (randomInteger() % 100) * scale;
}

void counts()
{
    std::ifstream positions("stableQueriedFast.txt");

    std::string board;
    int handOne, handTwo, side;
    double score;

    Position pos;
    double iters = 0;
    while (positions >> board >> handOne >> handTwo >> side >> score) {
        totalPositions++;
        pos.loadRegularFormat(board, handOne, handTwo, side);

        // compute kiki
        Bitboard kiki[4];
        pos.kikiBitboards(kiki);
        int k1 = (pos.pieceMaps[KING] &   pos.pieceMaps[8] ).BSF();
        int k2 = (pos.pieceMaps[KING] & (~pos.pieceMaps[8])).BSF();
        // evaluation
        for (int square = 0; square < 81; square++)
        {
            int kiki_defence = (kiki[0] & squareMask[square] ? 1 : 0) + (kiki[1] & squareMask[square] ? 2 : 0);
            int kiki_attack  = (kiki[2] & squareMask[square] ? 1 : 0) + (kiki[3] & squareMask[square] ? 2 : 0);
            counts_kikiKingSafety[oneSquareMap[k1][square]][kiki_defence][kiki_attack]++;
            counts_kikiKingSafety[twoSquareMap[k2][square]][kiki_attack][kiki_defence]++;

            int piece = pos.mailbox[square];
            counts_pieceSquareTable[piece][(pos.pieceMaps[8] & squareMask[square] ? square : 81 - square)]++;
        }

        for (int i = 1; i < 8; i++)
        {
            counts_pieceInHand[i - 1][hand_count(pos.hand[true], i)]++;
            counts_pieceInHand[i - 1][hand_count(pos.hand[false], i)]++;
        }
    }

    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 19; j++) {
            std::cout << counts_pieceInHand[i][j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    positions.close();
}

