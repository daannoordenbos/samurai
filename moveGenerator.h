#ifndef MOVEGENERATOR_H_INCLUDED
#define MOVEGENERATOR_H_INCLUDED

#include "position.h"
#include <algorithm> // For std::sort

// The most legal moves in a shogi position is 593 according to other engines.
const unsigned int MAX_LEGAL_MOVES = 600;

struct moveList {
    Move moveList[MAX_LEGAL_MOVES];
    bool inCheck = false;
    int size = 0;

    void addMove(const Move& move) {
        moveList[size++] = move;
    }

    void addMove(const int origin, const int destination,
                 const bool promotion, const bool capture,
                 const int movedPiece, const int capturedPiece) {
        moveList[size++].set(origin, destination, promotion, capture, movedPiece, capturedPiece);
    }

    void addMove(const int destination, const int movedPiece) {
        moveList[size++].set(destination, movedPiece);
    }

    Move& getMove(int index) {
        return moveList[index];
    }

    void clear() {
        size = 0;
        inCheck = false;
    }

    void sort() {
        std::sort(moveList, moveList + size, [](const Move& a, const Move& b) {
            return a.value > b.value;
        });
    }
};


const Bitboard unforcedPromotion[8][2] = {
    {Bitboard(true), Bitboard(true)},
    {Bitboard(true), Bitboard(true)},
    {Bitboard(true), Bitboard(true)},
    {Bitboard(true), Bitboard(true)},
    {Bitboard(true), Bitboard(true)},
    {Bitboard(0x7FFFFFFFFFFFFFFF, 0x0000000000000000), Bitboard(0x7FFFFFFFFFFC0000, 0x000000000003FFFF)},
    {Bitboard(0x7FFFFFFFFFFFFFFF, 0x00000000000001FF), Bitboard(0x7FFFFFFFFFFFFE00, 0x000000000003FFFF)},
    {Bitboard(0x7FFFFFFFFFFFFFFF, 0x00000000000001FF), Bitboard(0x7FFFFFFFFFFFFE00, 0x000000000003FFFF)}
};

moveList generateMoves(const Position& pos, bool allMoves = false);
void generateMoves(const Position& pos, moveList& legalMoves, bool allMoves = false);
void generateTacticalMoves(const Position& pos, moveList& legalMoves, bool allMoves = false);
uint64_t perft(Position& pos, int depth, Move lastMove);
void perftBreakdown(Position& pos, int depth);
uint64_t nearPerft(Position& pos, int depth);
void speedTest(Position& pos, const int depth);
#endif // MOVEGENERATOR_H_INCLUDED
