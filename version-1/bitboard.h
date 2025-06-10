#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

// for 128 bit register
#include <emmintrin.h>

enum PieceType {
    KING                    = 0,
    ROOK                    = 1,
    BISHOP                  = 2,
    GOLD_GENERAL            = 3,
    SILVER_GENERAL          = 4,
    KNIGHT                  = 5,
    LANCE                   = 6,
    PAWN                    = 7,
    // Promoted pieces are +8
    PROMOTED_ROOK           = 9,
    PROMOTED_BISHOP         = 10,
    PROMOTED_SILVER_GENERAL = 12,
    PROMOTED_KNIGHT         = 13,
    PROMOTED_LANCE          = 14,
    PROMOTED_PAWN           = 15
};


// Use 'alignas' to align the blocks properly, useful since we use a 128 bit union type.
// When the CPU loads it can load it all at once.
struct alignas(16) Bitboard
{
    union
    {
        uint64_t p[2];
        __m128i m;
    };

    /** Initializers **/
    Bitboard() : m(_mm_setzero_si128()) {}
    Bitboard(const Bitboard& bb) : m(bb.m) {} // Use copy constructor directly

    Bitboard(const uint64_t p0, const uint64_t p1)
    {
        m = _mm_set_epi64x(p1, p0);
    }

    // create an empty or filled bit board
    Bitboard(bool filled)
    {
        m = (filled ? _mm_set_epi64x(UINT64_C(0x000000000003FFFF), UINT64_C(0x7FFFFFFFFFFFFFFF)) : _mm_setzero_si128());
    }

    /** Bitboard operators (SIMD parallelization) **/
    Bitboard& operator<<=(int shift)
    {
        p[1] = (p[1] << shift) | (p[0] >> (63 - shift));
        p[0] <<= shift;
        return *this;
    }

    Bitboard& operator>>=(int shift)
    {
        p[0] = (p[0] >> shift) | (p[1] << (63 - shift));
        p[1] >>= shift;
        return *this;
    }

    Bitboard operator<<(int shift) const
    {
        Bitboard result(*this);
        result <<= shift;
        return result;
    }

    Bitboard operator>>(int shift) const
    {
        Bitboard result(*this);
        result >>= shift;
        return result;
    }

    Bitboard& operator|=(const Bitboard& b1)
    {
        m = _mm_or_si128(m, b1.m);
        return *this;
    }

    Bitboard& operator&=(const Bitboard& b1)
    {
        m = _mm_and_si128(m, b1.m);
        return *this;
    }

    Bitboard& operator^=(const Bitboard& b1)
    {
        m = _mm_xor_si128(m, b1.m);
        return *this;
    }

    Bitboard& operator+=(const Bitboard& b1)
    {
        m = _mm_add_epi64(m, b1.m);
        return *this;
    }

    Bitboard& operator-=(const Bitboard& b1)
    {
        m = _mm_sub_epi64(m, b1.m);
        return *this;
    }

    Bitboard operator&(const Bitboard& rhs) const
    {
        Bitboard result;
        result.m = _mm_and_si128(m, rhs.m);
        return result;
    }

    Bitboard operator|(const Bitboard& rhs) const
    {
        Bitboard result;
        result.m = _mm_or_si128(m, rhs.m);
        return result;
    }

    Bitboard operator^(const Bitboard& rhs) const
    {
        Bitboard result;
        result.m = _mm_xor_si128(m, rhs.m);
        return result;
    }

    Bitboard operator+(const Bitboard& rhs) const
    {
        Bitboard result;
        result.m = _mm_add_epi64(m, rhs.m);
        return result;
    }

    Bitboard operator-(const Bitboard& rhs) const
    {
        Bitboard result;
        result.m = _mm_sub_epi64(m, rhs.m);
        return result;
    }

    Bitboard operator~() const
    {
        Bitboard result;
        result.m = _mm_andnot_si128(m, _mm_set1_epi64x(static_cast<int64_t>(0xFFFFFFFFFFFFFFFF)));
        return result;
    }

    bool operator==(const Bitboard& rhs) const
    {
        return (this->p[0] == rhs.p[0]) && (this->p[1] == rhs.p[1]);
    }

    bool operator!=(const Bitboard& rhs) const
    {
        return (this->p[0] != rhs.p[0]) || (this->p[1] != rhs.p[1]);
    }

    operator bool() const
    {
        return (p[0] | p[1]) != 0;
    }

    /** Bitboard operations **/
    int BSF() const
    {
        return (p[0] ? __builtin_ctzll(p[0]) : 63 + __builtin_ctzll(p[1]));
    }

    inline int count() const
    {
        return __builtin_popcountll(p[0]) + __builtin_popcountll(p[1]);
    }

    void set(const uint64_t p0, const uint64_t p1)
    {
        m = _mm_set_epi64x(p1, p0);
    }

    uint64_t merge() const
    {
        return p[0] | p[1];
    }

    void removeLSB()
    {
        if (p[0] != 0)
        {
            p[0] &= (p[0] - 1);
        }
        else
        {
            p[1] &= (p[1] - 1);
        }
    }


    /** Auxiliary functions **/
	void print() const;
};

/** Constants **/
const Bitboard squareMask[82] = {
    Bitboard(UINT64_C(1) <<  0,                 0), // a9
    Bitboard(UINT64_C(1) <<  1,                 0), // a8
    Bitboard(UINT64_C(1) <<  2,                 0), // a7
    Bitboard(UINT64_C(1) <<  3,                 0), // a6
    Bitboard(UINT64_C(1) <<  4,                 0), // a5
    Bitboard(UINT64_C(1) <<  5,                 0), // a4
    Bitboard(UINT64_C(1) <<  6,                 0), // a3
    Bitboard(UINT64_C(1) <<  7,                 0), // a2
    Bitboard(UINT64_C(1) <<  8,                 0), // a1
    Bitboard(UINT64_C(1) <<  9,                 0), // b9
    Bitboard(UINT64_C(1) << 10,                 0), // b8
    Bitboard(UINT64_C(1) << 11,                 0), // b7
    Bitboard(UINT64_C(1) << 12,                 0), // b6
    Bitboard(UINT64_C(1) << 13,                 0), // b5
    Bitboard(UINT64_C(1) << 14,                 0), // b4
    Bitboard(UINT64_C(1) << 15,                 0), // b3
    Bitboard(UINT64_C(1) << 16,                 0), // b2
    Bitboard(UINT64_C(1) << 17,                 0), // b1
    Bitboard(UINT64_C(1) << 18,                 0), // c9
    Bitboard(UINT64_C(1) << 19,                 0), // c8
    Bitboard(UINT64_C(1) << 20,                 0), // c7
    Bitboard(UINT64_C(1) << 21,                 0), // c6
    Bitboard(UINT64_C(1) << 22,                 0), // c5
    Bitboard(UINT64_C(1) << 23,                 0), // c4
    Bitboard(UINT64_C(1) << 24,                 0), // c3
    Bitboard(UINT64_C(1) << 25,                 0), // c2
    Bitboard(UINT64_C(1) << 26,                 0), // c1
    Bitboard(UINT64_C(1) << 27,                 0), // d9
    Bitboard(UINT64_C(1) << 28,                 0), // d8
    Bitboard(UINT64_C(1) << 29,                 0), // d7
    Bitboard(UINT64_C(1) << 30,                 0), // d6
    Bitboard(UINT64_C(1) << 31,                 0), // d5
    Bitboard(UINT64_C(1) << 32,                 0), // d4
    Bitboard(UINT64_C(1) << 33,                 0), // d3
    Bitboard(UINT64_C(1) << 34,                 0), // d2
    Bitboard(UINT64_C(1) << 35,                 0), // d1
    Bitboard(UINT64_C(1) << 36,                 0), // e9
    Bitboard(UINT64_C(1) << 37,                 0), // e8
    Bitboard(UINT64_C(1) << 38,                 0), // e7
    Bitboard(UINT64_C(1) << 39,                 0), // e6
    Bitboard(UINT64_C(1) << 40,                 0), // e5
    Bitboard(UINT64_C(1) << 41,                 0), // e4
    Bitboard(UINT64_C(1) << 42,                 0), // e3
    Bitboard(UINT64_C(1) << 43,                 0), // e2
    Bitboard(UINT64_C(1) << 44,                 0), // e1
    Bitboard(UINT64_C(1) << 45,                 0), // f9
    Bitboard(UINT64_C(1) << 46,                 0), // f8
    Bitboard(UINT64_C(1) << 47,                 0), // f7
    Bitboard(UINT64_C(1) << 48,                 0), // f6
    Bitboard(UINT64_C(1) << 49,                 0), // f5
    Bitboard(UINT64_C(1) << 50,                 0), // f4
    Bitboard(UINT64_C(1) << 51,                 0), // f3
    Bitboard(UINT64_C(1) << 52,                 0), // f2
    Bitboard(UINT64_C(1) << 53,                 0), // f1
    Bitboard(UINT64_C(1) << 54,                 0), // g9
    Bitboard(UINT64_C(1) << 55,                 0), // g8
    Bitboard(UINT64_C(1) << 56,                 0), // g7
    Bitboard(UINT64_C(1) << 57,                 0), // g6
    Bitboard(UINT64_C(1) << 58,                 0), // g5
    Bitboard(UINT64_C(1) << 59,                 0), // g4
    Bitboard(UINT64_C(1) << 60,                 0), // g3
    Bitboard(UINT64_C(1) << 61,                 0), // g2
    Bitboard(UINT64_C(1) << 62,                 0), // g1
    Bitboard(                0, UINT64_C(1) <<  0), // h9
    Bitboard(                0, UINT64_C(1) <<  1), // h8
    Bitboard(                0, UINT64_C(1) <<  2), // h7
    Bitboard(                0, UINT64_C(1) <<  3), // h6
    Bitboard(                0, UINT64_C(1) <<  4), // h5
    Bitboard(                0, UINT64_C(1) <<  5), // h4
    Bitboard(                0, UINT64_C(1) <<  6), // h3
    Bitboard(                0, UINT64_C(1) <<  7), // h2
    Bitboard(                0, UINT64_C(1) <<  8), // h1
    Bitboard(                0, UINT64_C(1) <<  9), // i9
    Bitboard(                0, UINT64_C(1) << 10), // i8
    Bitboard(                0, UINT64_C(1) << 11), // i7
    Bitboard(                0, UINT64_C(1) << 12), // i6
    Bitboard(                0, UINT64_C(1) << 13), // i5
    Bitboard(                0, UINT64_C(1) << 14), // i4
    Bitboard(                0, UINT64_C(1) << 15), // i3
    Bitboard(                0, UINT64_C(1) << 16), // i2
    Bitboard(                0, UINT64_C(1) << 17), // i1
    Bitboard(                0,                 0)  // Out of bounds drop square
};

const Bitboard columnMask[9] = {
    Bitboard(0x0040201008040201, 0x0000000000000201),
    Bitboard(0x0080402010080402, 0x0000000000000402),
    Bitboard(0x0100804020100804, 0x0000000000000804),
    Bitboard(0x0201008040201008, 0x0000000000001008),
    Bitboard(0x0402010080402010, 0x0000000000002010),
    Bitboard(0x0804020100804020, 0x0000000000004020),
    Bitboard(0x1008040201008040, 0x0000000000008040),
    Bitboard(0x2010080402010080, 0x0000000000010080),
    Bitboard(0x4020100804020100, 0x0000000000020100)
};

const Bitboard rowMask[9] = {
    Bitboard(0x00000000000001FF, 0),
    Bitboard(0x000000000003FE00, 0),
    Bitboard(0x0000000007FC0000, 0),
    Bitboard(0x0000000FF8000000, 0),
    Bitboard(0x00001FF000000000, 0),
    Bitboard(0x003FE00000000000, 0),
    Bitboard(0x7FC0000000000000, 0),
    Bitboard(0, 0x00000000000001FF),
    Bitboard(0, 0x000000000003FE00)
};

const Bitboard promotionZone[2] = {
    Bitboard(0x7FC0000000000000, 0x000000000003FFFF),
    Bitboard(0x0000000007FFFFFF, 0)
};

void initialiseBitboards();

/** Source: https://github.com/HiraokaTakuya/apery/tree/master/src **/

extern const uint64_t rookMagic[81];
extern const int rookShiftBits[81];
extern int rookLookupIndex[81];
extern Bitboard rookSlide[512000];
extern Bitboard rookMask[81];
extern Bitboard dragonMask[81];

extern const uint64_t bishopMagic[81];
extern const int bishopShiftBits[81];
extern int bishopLookupIndex[81];
extern Bitboard bishopSlide[20224];
extern Bitboard bishopMask[81];
extern Bitboard horseMask[81];

extern Bitboard kingMask[81];
extern Bitboard goldMask[2][81];
extern Bitboard silverMask[2][81];
extern Bitboard knightMask[2][81];
extern Bitboard lanceMask[2][81];


inline Bitboard kingAttack(int square)
{
    return kingMask[square];
}

inline Bitboard rookAttack(int square, const Bitboard& empty)
{
    return rookSlide[rookLookupIndex[square] +
                     ((((empty.p[0] & rookMask[square].p[0]) |
                        (empty.p[1] & rookMask[square].p[1]))
                                    * rookMagic[square]) >> rookShiftBits[square])];
}

inline Bitboard bishopAttack(int square, const Bitboard& empty)
{
    return bishopSlide[bishopLookupIndex[square] +
                     ((((empty.p[0] & bishopMask[square].p[0]) |
                        (empty.p[1] & bishopMask[square].p[1]))
                                    * bishopMagic[square]) >> bishopShiftBits[square])];
}

inline Bitboard goldAttack(int square, bool firstMover)
{
    return goldMask[firstMover][square];
}

inline Bitboard silverAttack(int square, bool firstMover)
{
    return silverMask[firstMover][square];
}

inline Bitboard knightAttack(int square, bool firstMover)
{
    return knightMask[firstMover][square];
}

inline Bitboard lanceAttack(int square, const Bitboard& empty, bool firstMover)
{
    return rookSlide[rookLookupIndex[square] +
                     ((((empty.p[0] & rookMask[square].p[0]) |
                        (empty.p[1] & rookMask[square].p[1]))
                                    * rookMagic[square]) >> rookShiftBits[square])] & lanceMask[firstMover][square];
}

inline Bitboard pawnAttack(int square, bool firstMover)
{
    return squareMask[(firstMover ? square - 9 : square + 9)];
}

inline Bitboard promotedRookAttack(int square, const Bitboard& empty)
{
    return rookSlide[rookLookupIndex[square] +
                     ((((empty.p[0] & rookMask[square].p[0]) |
                        (empty.p[1] & rookMask[square].p[1]))
                                    * rookMagic[square]) >> rookShiftBits[square])] | kingMask[square];
}

inline Bitboard promotedBishopAttack(int square, const Bitboard& empty)
{
    return bishopSlide[bishopLookupIndex[square] +
                     ((((empty.p[0] & bishopMask[square].p[0]) |
                        (empty.p[1] & bishopMask[square].p[1]))
                                    * bishopMagic[square]) >> bishopShiftBits[square])] | kingMask[square];
}

inline Bitboard attackMap(int piece, int square, const Bitboard& empty, bool firstMover) {
    switch (piece) {
        case KING:
            return kingAttack(square);

        case ROOK:
            return rookAttack(square, empty);

        case BISHOP:
            return bishopAttack(square, empty);

        case GOLD_GENERAL:
            return goldAttack(square, firstMover);

        case SILVER_GENERAL:
            return silverAttack(square, firstMover);

        case KNIGHT:
            return knightAttack(square, firstMover);

        case LANCE:
            return lanceAttack(square, empty, firstMover);

        case PAWN:
            return pawnAttack(square, firstMover);

        case PROMOTED_ROOK:
            return promotedRookAttack(square, empty);

        case PROMOTED_BISHOP:
            return promotedBishopAttack(square, empty);

        case PROMOTED_SILVER_GENERAL:
            return goldAttack(square, firstMover);

        case PROMOTED_KNIGHT:
            return goldAttack(square, firstMover);

        case PROMOTED_LANCE:
            return goldAttack(square, firstMover);

        case PROMOTED_PAWN:
            return goldAttack(square, firstMover);

        default:
            std::cout << "Piece " << piece << " is unknown!\n";
            return Bitboard(false);
    }
}

#endif // BITBOARD_H_INCLUDED
