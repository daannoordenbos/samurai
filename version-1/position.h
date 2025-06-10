#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include "bitboard.h"


constexpr int PIECE_HAND_LOCATION[8] = {0, 0, 4, 8, 12, 16, 20, 24};
constexpr int PIECE_HAND_MASK[8] = {0, 15, 15, 15, 15, 15, 15, 255};

enum Hand : uint32_t {EMPTY_HAND = 0, };

constexpr void add_hand(Hand &hand, PieceType piece, int amount = 1) { hand = (Hand)(hand + (amount << PIECE_HAND_LOCATION[piece])); }
constexpr void sub_hand(Hand &hand, PieceType piece, int amount = 1) { hand = (Hand)(hand - (amount << PIECE_HAND_LOCATION[piece])); }
constexpr int hand_count(Hand hand, PieceType piece) { return (int)(hand >> PIECE_HAND_LOCATION[piece]) & PIECE_HAND_MASK[piece]; }

constexpr void add_hand(Hand &hand, int piece, int amount = 1) { hand = (Hand)(hand + (amount << PIECE_HAND_LOCATION[piece])); }
constexpr void sub_hand(Hand &hand, int piece, int amount = 1) { hand = (Hand)(hand - (amount << PIECE_HAND_LOCATION[piece])); }
constexpr int hand_count(Hand hand, int piece) { return (int)(hand >> PIECE_HAND_LOCATION[piece]) & PIECE_HAND_MASK[piece]; }

// xxxxxxxx xxxxxxxx xxxxxxxx x1111111  Origin (127 for drops)
// xxxxxxxx xxxxxxxx xx111111 1xxxxxxx  Destination
// xxxxxxxx xxxxxxxx x1xxxxxx xxxxxxxx  Promotion
// xxxxxxxx xxxxxxxx 1xxxxxxx xxxxxxxx  Capture
// xxxxxxxx xxxx1111 xxxxxxxx xxxxxxxx  movedPiece
// xxxxxxxx 1111xxxx xxxxxxxx xxxxxxxx  capturedPiece

const int DROP_SQUARE = 81;

struct Move {
    int value;

    Move()
    {
        value = 0;
    }
    Move(const int origin, const int destination,
         const bool promotion, const bool capture,
         const int movedPiece, const int capturedPiece)
    {
        value = (origin) | (destination << 7) | (promotion << 14) | (capture << 15) | (movedPiece << 16) | (capturedPiece << 20);
    }
    Move(const int destination, const int movedPiece)
    {
        value = (DROP_SQUARE) | (destination << 7) | (movedPiece << 16);
    }

    void set(const int origin, const int destination,
             const bool promotion, const bool capture,
             const int movedPiece, const int capturedPiece)
    {
        value = (origin) | (destination << 7) | (promotion << 14) | (capture << 15) | (movedPiece << 16) | (capturedPiece << 20);
    }

    void set(const int destination, const int movedPiece)
    {
        value = (DROP_SQUARE) | (destination << 7) | (movedPiece << 16);
    }

    void setScore(const int score)
    {
        value |= score << 24;
    }
    /** Macro functions **/
    inline int  from()          const { return  value & 0x7F; }
    inline int  to()            const { return (value >> 7) & 0x7F; }
    inline bool isDrop()        const { return (value & 0x7F) == DROP_SQUARE; }
    inline bool isPromotion()   const { return  value & 0x4000; }
    inline bool isCapture()     const { return  value & 0x8000; }
    inline int  movedPiece()    const { return (value >> 16) & 0xF; }
    inline int  capturedPiece() const { return (value >> 20) & 0xF; }
    inline int  movedType()     const { return (value >> 16) & 0x7; }
    inline int  capturedType()  const { return (value >> 20) & 0x7; }
    inline int  moveScore()     const { return (value >> 24); }

};

struct Position {
    bool playerOne = true;
    Hand hand[2] = {EMPTY_HAND, EMPTY_HAND};
    // The bitboards represent: king, rook, bishop, gold general, silver general, Knight, lance, pawns, color, promoted
    Bitboard pieceMaps[10];
    uint8_t mailbox[81] = {0};

    void kikiBitboards(Bitboard (&out)[4]) const;
    void loadInitial();
    void loadMailbox();
    void loadSFEN(const char* sfen);
    void createSFEN();
    std::string regularFormat() const;
    void loadRegularFormat(std::string& board, int handOne, int handTwo, bool onMove);
    void print();
    Move USIToMove(const char* move);
    void makeMove(Move& move);
    void undoMove(Move& move);
};

std::ostream& operator<<(std::ostream& os, const Move& move);

#endif // POSITION_H_INCLUDED
