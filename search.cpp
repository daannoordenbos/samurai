#include <iostream>

#include "moveGenerator.h"
#include "position.h"
#include "learner.h"
#include "random.h"

// Constants
const int INF = 10000;
const int MVV_LVA[16] = {0, 11, 9, 8, 7, 5, 3, 1, 0, 23, 18, 0, 11, 9, 11, 10};
const int captureValue[16] = {0, 165, 135, 122, 106, 62, 56, 23, 0, 217, 173, 0, 113, 81, 83, 65};

// Timing
int start_time;
int last_time;
int max_time;

// Stats
uint64_t search_count = 0;
uint64_t evaluations = 0;
uint64_t ply_sum = 0;
uint64_t historyHeuristic[16][81][81] = {0};

// Move generation
moveList moveListStack[100];
Move globalBestMove;

// Function declarations
int negamax(Position& node, int depth, int plies, int alpha, int beta);
int quiescence(Position& node, int plies, int qsPlies, int alpha, int beta);

void engineMove (Position& pos, int timeAllowance) {
    pos.print();
    /* initialize root */
    max_time = timeAllowance;
    int score;
    start_time = getTime();
    Move computerMove;
    // do search
    for (int i = 100; i <= 3000; i += 100) {

        score = negamax(pos, i, 0, -INF, INF);
        last_time = getTime();
        int time = last_time - start_time;

        if (time > timeAllowance) break;
        // search output

        std::cout<<"Depth("<< (double) (10 * ply_sum / (evaluations + 1)) / 10 <<")\tEval: " << (pos.playerOne ? score : -score) <<"    \t"<< (double) time / 1000 <<" sec\t";
        if (time % 100 == 0) {
            std::cout<<"\t";
        }
        std::cout << globalBestMove << "\n";
        computerMove.value = globalBestMove.value;
    }
    // play move
    pos.makeMove(computerMove);
    // statistic output
    std::cout << "Thinking time      \t" << getTime() - start_time << "ms\n";
    std::cout << "Search calls:      \t" << search_count    << "\n";
    std::cout << "Evaluation calls:  \t" << evaluations     << "\n";
    std::cout << "Speed (leaf nodes):\t" << evaluations / timeAllowance << " Kn/s\n";
    // reset statistics
    search_count  = 0;
    evaluations   = 0;
    ply_sum       = 0;
    for (int p = 0; p < 16; ++p)
        for (int f = 0; f < 81; ++f)
            for (int t = 0; t < 81; ++t)
                historyHeuristic[p][f][t] /= 8;
}



int negamax(Position& node, int depth, int plies, int alpha, int beta) {

    if ((search_count & 8191)== 0) {
        last_time = getTime();
        if (last_time - start_time > max_time) {
            // kill the search
            return 0;
        }
    }
    search_count++;

    if (plies != 0) {
        alpha = std::max(-INF + 100 + plies, alpha);
        beta = std::min(-(-INF + 100 + plies + 1), beta);
        if (alpha >= beta) {
            return alpha;
        }
    }


    if (depth <= 0 || plies > 20) {
        return quiescence(node, plies, 0, alpha, beta);
    }

    generateMoves(node, moveListStack[plies]);
    int depthMin = 100;
  /*if (moveListStack[plies].inCheck)
    {
        depthMin = 0;
    }*/
     // step 7: Check for no legal moves
    if (moveListStack[plies].size == 0) {
        return -INF + 100 + plies;
    }

    // order moves
    for (int i = 0; i < moveListStack[plies].size; i++)
    {
        Move move = moveListStack[plies].moveList[i];
        int moveScore = 31 + MVV_LVA[move.capturedPiece()] - MVV_LVA[move.movedPiece()]
                           + 10 * move.isPromotion()
                           - 4 * move.isDrop()
                           + (historyHeuristic[move.movedPiece()][move.from()][move.to()] >> 4);
        moveScore = std::min(255, moveScore);
        moveListStack[plies].moveList[i].setScore(moveScore);
    }

    moveListStack[plies].sort();

    int bestValue = -INF;
    Move bestMove;
    for (int i = 0; i < moveListStack[plies].size; i++) {
        Move move = moveListStack[plies].getMove(i);

        node.makeMove(move);
        int childValue = -negamax(node, depth - depthMin, plies + 1, -beta, -alpha);
        node.undoMove(move);

        if (childValue >= beta) {
            historyHeuristic[move.movedPiece()][move.from()][move.to()] += depth * depth >> 13;
            return beta;  // Early cutoff
        }

        if (childValue > bestValue) {
            bestValue = childValue;
            bestMove.value = move.value & 0x00FFFFFF;  // Preserve only move part
        }

        alpha = std::max(alpha, bestValue);
    }
    if (plies == 0) {
        globalBestMove.value = bestMove.value;
    }


    return bestValue;
}



int staticExchangeValue(Position& pos, const Move& move) {
    int attackerValue[16] = {0};
    int defenderValue[16] = {0};
    int attackers = 0;
    int defenders = 0;

    int captureSquare = move.to();
    // attack is removed from occupied
    const Bitboard occupied = (pos.pieceMaps[0] | pos.pieceMaps[1] | pos.pieceMaps[2] | pos.pieceMaps[3] |
                               pos.pieceMaps[4] | pos.pieceMaps[5] | pos.pieceMaps[6] | pos.pieceMaps[7]) ^ squareMask[move.from()];
    // We clear the squares around the capture square to a better sliding piece influence approximation.
    // const Bitboard empty       = (~occupied & Bitboard(true)) | kingMask[captureSquare];
    Bitboard empty       = ~occupied & Bitboard(true);
    const Bitboard ownPieces   = (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & occupied;
    const Bitboard enemyPieces = ~ownPieces & occupied;
    const Bitboard promoted    = pos.pieceMaps[9];

    Bitboard attackPieces, defencePieces;

    /** build attack table **/
    int priority[13] = {PAWN, LANCE, KNIGHT, PROMOTED_PAWN, PROMOTED_KNIGHT, PROMOTED_LANCE, SILVER_GENERAL, PROMOTED_SILVER_GENERAL, GOLD_GENERAL, BISHOP, ROOK, PROMOTED_BISHOP, PROMOTED_ROOK};
    int baseType[13] = {PAWN, LANCE, KNIGHT, PAWN, KNIGHT, LANCE, SILVER_GENERAL, SILVER_GENERAL, GOLD_GENERAL, BISHOP, ROOK, BISHOP, ROOK};
    for (int j = 0; j < 13; j++) {
        attackPieces  = ownPieces   & pos.pieceMaps[baseType[j]] & (priority[j] > 7 ? promoted : ~promoted) &
                        attackMap(priority[j], captureSquare, empty, !pos.playerOne);
        defencePieces = enemyPieces & pos.pieceMaps[baseType[j]] & (priority[j] > 7 ? promoted : ~promoted) &
                        attackMap(priority[j], captureSquare, empty, pos.playerOne);
        int attackCount = attackPieces.count();
        for (int i = 0; i < attackCount; i++) {
            attackerValue[attackers] = captureValue[priority[j]];
            attackers++;
        }

        int defenceCount = defencePieces.count();
        for (int i = 0; i < defenceCount; i++) {
            defenderValue[defenders] = captureValue[priority[j]];
            defenders++;
        }

        empty |= attackPieces | defencePieces;
    }

    // compute SEE (could be optimized?)
    int gain[32] = {0};
    gain[0] = captureValue[move.capturedPiece()];
    gain[1] = captureValue[move.movedPiece()] - gain[0];

    int depth = 1, a = 0, d = 0;
    bool side = false;
    while ((side && a < attackers) || (!side && d < defenders)) {
        depth++;
        int nextPiece = side ? attackerValue[a++] : defenderValue[d++];
        gain[depth] = nextPiece - gain[depth - 1];
        side = !side;
    }

    // Minimax backpropagation
    while (--depth > 0) {
        gain[depth - 1] = -std::max(-gain[depth - 1], gain[depth]);
    }


    return gain[0];
}


int quiescence(Position& node, int plies, int qsPlies, int alpha, int beta) {
    if ((evaluations & 65535)== 0) {
        last_time = getTime();
        if (last_time - start_time > max_time) {
            // Exceeded time limit — terminate search
            return 0;
        }
    }
    evaluations++;
    int stand_pat = 100 * evaluation(node);
    if (!node.playerOne) {
        stand_pat = -stand_pat;
    }

    // Stand pat pruning
    if (stand_pat >= beta) {
        ply_sum += plies;
        return beta;
    }
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }
    // Prevent needlessly deep searches
    if (qsPlies > 6) return stand_pat;
    generateTacticalMoves(node, moveListStack[plies]);

    // No tactical moves? Return static eval
    if (moveListStack[plies].size == 0) {
        ply_sum += plies;
        return stand_pat;
    }

    // order moves
    for (int i = 0; i < moveListStack[plies].size; i++)
    {
        Move move = moveListStack[plies].moveList[i];
        int moveScore = 31 + MVV_LVA[move.capturedPiece()] - MVV_LVA[move.movedPiece()]
                                                  + 10 * move.isPromotion() - 4 * move.isDrop();
        moveListStack[plies].moveList[i].setScore(moveScore);
    }
    moveListStack[plies].sort();



    for (int i = 0; i < moveListStack[plies].size; i++) {
        Move move = moveListStack[plies].getMove(i);

        // Filter out bad captures
        if (move.isCapture() && staticExchangeValue(node, move) < -captureValue[PAWN]) continue;

        node.makeMove(move);
        int score = -quiescence(node, plies + 1, qsPlies + 1, -beta, -alpha);
        node.undoMove(move);

        if (score >= beta) {
            ply_sum += plies;
            return beta;
        }
        if (score > alpha) alpha = score;
    }

    return alpha;
}




