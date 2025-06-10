#include <iostream>
#include <vector>
#include "bitboard.h"

// functions exclusively for generation
uint64_t _pdep_u64(uint64_t mask, uint64_t val);
int L_1_norm(int sq1, int sq2);
Bitboard raysAttackMap(const Bitboard field, const int startSquare, const std::vector<int>& rays);

void Bitboard::print() const
{
    std::cout << "Bitboard:\n";
    for (int i = 0; i < 81; ++i)
    {
        bool isSet = (*this & squareMask[i]);
        std::cout << (isSet ? " X" : " -") << ((i % 9 == 8) ? "\n" : "");
    }
    std::cout << "\n";
}

// Parallel deposit is not present on all CPU's so a simple implementation is used
uint64_t _pdep_u64(uint64_t mask, uint64_t val) {
    uint64_t res = 0;
    for (uint64_t bb = 1; mask; bb += bb) {
        if (val & bb)
            res |= mask & -mask;
        mask &= mask - 1;
    }
    return res;
}

/** Source: https://github.com/HiraokaTakuya/apery/tree/master/src **/

const uint64_t rookMagic[81] = {
    UINT64_C(0x140000400809300),  UINT64_C(0x1320000902000240), UINT64_C(0x8001910c008180),
    UINT64_C(0x40020004401040),   UINT64_C(0x40010000d01120),   UINT64_C(0x80048020084050),
    UINT64_C(0x40004000080228),   UINT64_C(0x400440000a2a0a),   UINT64_C(0x40003101010102),
    UINT64_C(0x80c4200012108100), UINT64_C(0x4010c00204000c01), UINT64_C(0x220400103250002),
    UINT64_C(0x2600200004001),    UINT64_C(0x40200052400020),   UINT64_C(0xc00100020020008),
    UINT64_C(0x9080201000200004), UINT64_C(0x2200201000080004), UINT64_C(0x80804c0020200191),
    UINT64_C(0x45383000009100),   UINT64_C(0x30002800020040),   UINT64_C(0x40104000988084),
    UINT64_C(0x108001000800415),  UINT64_C(0x14005000400009),   UINT64_C(0xd21001001c00045),
    UINT64_C(0xc0003000200024),   UINT64_C(0x40003000280004),   UINT64_C(0x40021000091102),
    UINT64_C(0x2008a20408000d00), UINT64_C(0x2000100084010040), UINT64_C(0x144080008008001),
    UINT64_C(0x50102400100026a2), UINT64_C(0x1040020008001010), UINT64_C(0x1200200028005010),
    UINT64_C(0x4280030030020898), UINT64_C(0x480081410011004),  UINT64_C(0x34000040800110a),
    UINT64_C(0x101000010c0021),   UINT64_C(0x9210800080082),    UINT64_C(0x6100002000400a7),
    UINT64_C(0xa2240800900800c0), UINT64_C(0x9220082001000801), UINT64_C(0x1040008001140030),
    UINT64_C(0x40002220040008),   UINT64_C(0x28000124008010c),  UINT64_C(0x40008404940002),
    UINT64_C(0x40040800010200),   UINT64_C(0x90000809002100),   UINT64_C(0x2800080001000201),
    UINT64_C(0x1400020001000201), UINT64_C(0x180081014018004),  UINT64_C(0x1100008000400201),
    UINT64_C(0x80004000200201),   UINT64_C(0x420800010000201),  UINT64_C(0x2841c00080200209),
    UINT64_C(0x120002401040001),  UINT64_C(0x14510000101000b),  UINT64_C(0x40080000808001),
    UINT64_C(0x834000188048001),  UINT64_C(0x4001210000800205), UINT64_C(0x4889a8007400201),
    UINT64_C(0x2080044080200062), UINT64_C(0x80004002861002),   UINT64_C(0xc00842049024),
    UINT64_C(0x8040000202020011), UINT64_C(0x400404002c0100),   UINT64_C(0x2080028202000102),
    UINT64_C(0x8100040800590224), UINT64_C(0x2040009004800010), UINT64_C(0x40045000400408),
    UINT64_C(0x2200240020802008), UINT64_C(0x4080042002200204), UINT64_C(0x4000b0000a00a2),
    UINT64_C(0xa600000810100),    UINT64_C(0x1410000d001180),   UINT64_C(0x2200101001080),
    UINT64_C(0x100020014104e120), UINT64_C(0x2407200100004810), UINT64_C(0x80144000a0845050),
    UINT64_C(0x1000200060030c18), UINT64_C(0x4004200020010102), UINT64_C(0x140600021010302)
};

const int rookShiftBits[81] = {
    50, 51, 51, 51, 51, 51, 51, 51, 50,
    51, 52, 52, 52, 52, 52, 52, 52, 50, // [17]: 51 -> 50
    51, 52, 52, 52, 52, 52, 52, 52, 51,
    51, 52, 52, 52, 52, 52, 52, 52, 51,
    51, 52, 52, 52, 52, 52, 52, 52, 51,
    51, 52, 52, 52, 52, 52, 52, 52, 50, // [53]: 51 -> 50
    51, 52, 52, 52, 52, 52, 52, 52, 51,
    51, 52, 52, 52, 52, 52, 52, 52, 51,
    50, 51, 51, 51, 51, 51, 51, 51, 50
};

int rookLookupIndex[81];
Bitboard rookSlide[512000];
Bitboard rookMask[81];
Bitboard dragonMask[81];

const uint64_t bishopMagic[81] = {
    UINT64_C(0x20101042c8200428), UINT64_C(0x840240380102),     UINT64_C(0x800800c018108251),
    UINT64_C(0x82428010301000),   UINT64_C(0x481008201000040),  UINT64_C(0x8081020420880800),
    UINT64_C(0x804222110000),     UINT64_C(0xe28301400850),     UINT64_C(0x2010221420800810),
    UINT64_C(0x2600010028801824), UINT64_C(0x8048102102002),    UINT64_C(0x4000248100240402),
    UINT64_C(0x49200200428a2108), UINT64_C(0x460904020844),     UINT64_C(0x2001401020830200),
    UINT64_C(0x1009008120),       UINT64_C(0x4804064008208004), UINT64_C(0x4406000240300ca0),
    UINT64_C(0x222001400803220),  UINT64_C(0x226068400182094),  UINT64_C(0x95208402010d0104),
    UINT64_C(0x4000807500108102), UINT64_C(0xc000200080500500), UINT64_C(0x5211000304038020),
    UINT64_C(0x1108100180400820), UINT64_C(0x10001280a8a21040), UINT64_C(0x100004809408a210),
    UINT64_C(0x202300002041112),  UINT64_C(0x4040a8000460408),  UINT64_C(0x204020021040201),
    UINT64_C(0x8120013180404),    UINT64_C(0xa28400800d020104), UINT64_C(0x200c201000604080),
    UINT64_C(0x1082004000109408), UINT64_C(0x100021c00c410408), UINT64_C(0x880820905004c801),
    UINT64_C(0x1054064080004120), UINT64_C(0x30c0a0224001030),  UINT64_C(0x300060100040821),
    UINT64_C(0x51200801020c006),  UINT64_C(0x2100040042802801), UINT64_C(0x481000820401002),
    UINT64_C(0x40408a0450000801), UINT64_C(0x810104200000a2),   UINT64_C(0x281102102108408),
    UINT64_C(0x804020040280021),  UINT64_C(0x2420401200220040), UINT64_C(0x80010144080c402),
    UINT64_C(0x80104400800002),   UINT64_C(0x1009048080400081), UINT64_C(0x100082000201008c),
    UINT64_C(0x10001008080009),   UINT64_C(0x2a5006b80080004),  UINT64_C(0xc6288018200c2884),
    UINT64_C(0x108100104200a000), UINT64_C(0x141002030814048),  UINT64_C(0x200204080010808),
    UINT64_C(0x200004013922002),  UINT64_C(0x2200000020050815), UINT64_C(0x2011010400040800),
    UINT64_C(0x1020040004220200), UINT64_C(0x944020104840081),  UINT64_C(0x6080a080801c044a),
    UINT64_C(0x2088400811008020), UINT64_C(0xc40aa04208070),    UINT64_C(0x4100800440900220),
    UINT64_C(0x48112050),         UINT64_C(0x818200d062012a10), UINT64_C(0x402008404508302),
    UINT64_C(0x100020101002),     UINT64_C(0x20040420504912),   UINT64_C(0x2004008118814),
    UINT64_C(0x1000810650084024), UINT64_C(0x1002a03002408804), UINT64_C(0x2104294801181420),
    UINT64_C(0x841080240500812),  UINT64_C(0x4406009000004884), UINT64_C(0x80082004012412),
    UINT64_C(0x80090880808183),   UINT64_C(0x300120020400410),  UINT64_C(0x21a090100822002)
};

const int bishopShiftBits[81] = {
    57, 58, 58, 58, 58, 58, 58, 58, 57,
    58, 58, 58, 58, 58, 58, 58, 58, 58,
    58, 58, 56, 56, 56, 56, 56, 58, 58,
    58, 58, 56, 54, 54, 54, 56, 58, 58,
    58, 58, 56, 54, 52, 54, 56, 58, 58,
    58, 58, 56, 54, 54, 54, 56, 58, 58,
    58, 58, 56, 56, 56, 56, 56, 58, 58,
    58, 58, 58, 58, 58, 58, 58, 58, 58,
    57, 58, 58, 58, 58, 58, 58, 58, 57
};

int bishopLookupIndex[81];
Bitboard bishopSlide[20224];
Bitboard bishopMask[81];
Bitboard horseMask[81];

Bitboard kingMask[81];
Bitboard goldMask[2][81];
Bitboard silverMask[2][81];
Bitboard knightMask[2][81];
Bitboard lanceMask[2][81];

void initialiseBitboards()
{
    std::cout << "Initialising bitboards for move generation:";
    /** Rook and Bishop initialization **/
    /*  Mask and index computation  */
    const Bitboard interior(UINT64_C(0x3F9FCFE7F3F9FC00), UINT64_C(0x00000000000000FE));
    const Bitboard rookBorder[2] =
    {
        Bitboard(UINT64_C(0x7FFFFFFFFFFFFE00), UINT64_C(0x00000000000001FF)),
        Bitboard(UINT64_C(0x3F9FCFE7F3F9FCFE), UINT64_C(0x000000000001FCFE))
    };
    const Bitboard filledBitboard(true);
    std::vector<int> rookRays = {-1, 1, -9, 9};
    std::vector<int> bishopRays = {-10, 10, -8, 8};


    int rookIndex = 0;
    int bishopIndex = 0;
    const int rookDirections[4] = {-1, 1, -9, 9};
    for (int i = 0; i < 81; ++i)
    {

        rookLookupIndex[i] = rookIndex;
        rookIndex += (1 << (64 - rookShiftBits[i]));
        rookMask[i].set(0, 0);
        for (int direction : rookDirections)
        {
            int step = 1;
            while (0 <= i + step * direction && i + step * direction <= 80
                   && (squareMask[i + step * direction] & rookBorder[std::abs(direction) == 1]))
            {
                rookMask[i] |= squareMask[i + step * direction];
                ++step;
            }
        }

        bishopLookupIndex[i] = bishopIndex;
        bishopIndex += (1 << (64 - bishopShiftBits[i]));
        bishopMask[i] = raysAttackMap(filledBitboard, i, bishopRays) & interior;
    }
    /*  Sliding piece filler  */
    int rightBits,leftBits;
    uint64_t rightMask, leftMask, rightResult, leftResult;
    Bitboard result(false);
    Bitboard attacks(false);
    for (int s = 0; s < 81; ++s)
    {
        // get masks and bit counts
        rightBits = __builtin_popcountll(bishopMask[s].p[0]);
        leftBits  = __builtin_popcountll(bishopMask[s].p[1]);
        rightMask = bishopMask[s].p[0];
        leftMask  = bishopMask[s].p[1];
        for (int i = 0; i < (1 << rightBits); ++i)
        {
            for (int j = 0; j < (1 << leftBits); ++j)
            {
                // set occupancy
                rightResult = _pdep_u64(rightMask, i);
                leftResult = _pdep_u64(leftMask, j);
                result.set(rightResult, leftResult);
                // compute partial index and fill table
                uint64_t index = ((result.p[0] | result.p[1]) * bishopMagic[s]) >> bishopShiftBits[s];
                bishopSlide[bishopLookupIndex[s] + index] = raysAttackMap(result, s, bishopRays);
            }
        }

        // get masks and bit counts
        rightBits = __builtin_popcountll(rookMask[s].p[0]);
        leftBits = __builtin_popcountll(rookMask[s].p[1]);
        rightMask = rookMask[s].p[0];
        leftMask = rookMask[s].p[1];
        for (int i = 0; i < (1 << rightBits); ++i)
        {
            for (int j = 0; j < (1 << leftBits); ++j)
            {
                // set occupancy
                rightResult = _pdep_u64(rightMask, i);
                leftResult = _pdep_u64(leftMask, j);
                result.set(rightResult, leftResult);
                // compute partial index and fill table
                uint64_t index = ((result.p[0] | result.p[1]) * rookMagic[s]) >> rookShiftBits[s];
                rookSlide[rookLookupIndex[s] + index] = raysAttackMap(result, s, rookRays);
            }
        }
    }


    /** King, Gold, Silver, Knight, Lance, extra Horse and Dragon mask initialization **/
    std::vector<int> firstLanceRay = {-9};
    std::vector<int> secondLanceRay = {9};
    const int dir[10] = {-1, 1, 9, -9, -10, -8, 10, 8, -17, -19};
    for (int s = 0; s < 81; ++s)
    {
        kingMask[s].set(0, 0);
        for (int d = 0; d < 8; ++d)
        {
            if (0 <= s + dir[d] && s + dir[d] <= 80 && L_1_norm(s, s + dir[d]) <= 2)
                kingMask[s] |= squareMask[s + dir[d]];
        }

        goldMask[1][s].set(0, 0);
        goldMask[0][s].set(0, 0);
        for (int d = 0; d < 6; ++d)
        {
            if (0 <= s + dir[d] && s + dir[d] <= 80 && L_1_norm(s, s + dir[d]) <= 2)
                goldMask[1][s] |= squareMask[s + dir[d]];
            if (0 <= s - dir[d] && s - dir[d] <= 80 && L_1_norm(s, s - dir[d]) <= 2)
                goldMask[0][s] |= squareMask[s - dir[d]];
        }

        silverMask[1][s].set(0, 0);
        silverMask[0][s].set(0, 0);
        for (int d = 3; d < 8; ++d)
        {
            if (0 <= s + dir[d] && s + dir[d] <= 80 && L_1_norm(s, s + dir[d]) <= 2)
                silverMask[1][s] |= squareMask[s + dir[d]];
            if (0 <= s - dir[d] && s - dir[d] <= 80 && L_1_norm(s, s - dir[d]) <= 2)
                silverMask[0][s] |= squareMask[s - dir[d]];
        }

        knightMask[1][s].set(0, 0);
        knightMask[0][s].set(0, 0);
        for (int d = 8; d < 10; ++d)
        {
            if (0 <= s + dir[d] && s + dir[d] <= 80 && L_1_norm(s, s + dir[d]) == 3)
                knightMask[1][s] |= squareMask[s + dir[d]];
            if (0 <= s - dir[d] && s - dir[d] <= 80 && L_1_norm(s, s - dir[d]) == 3)
                knightMask[0][s] |= squareMask[s - dir[d]];
        }

        lanceMask[0][s] = raysAttackMap(Bitboard(true), s, secondLanceRay);
        lanceMask[1][s] = raysAttackMap(Bitboard(true), s, firstLanceRay);

        dragonMask[s] = silverMask[0][s] & silverMask[1][s];
        horseMask[s] = goldMask[0][s] & goldMask[1][s];
    }
    std::cout << " Completed\n";
}

Bitboard raysAttackMap(const Bitboard field, const int startSquare, const std::vector<int>& rays)
{
    Bitboard result(false);
    int x_0 = startSquare % 9;
    int y_0 = startSquare / 9;

    for (int ray : rays)
    {
        // Small (unelegant) work around to map rays to deltas, 8 does not cooperate.
        int del_x = std::abs(ray) == 8 ? - ray / 8 : ray % 9;
        int del_y = std::abs(ray) == 8 ?   ray / 8 : ray / 9;
        int step = 0;
        // obtain all empty squares that can be seen
        while (0 <= x_0 + (step + 1) * del_x &&
               0 <= y_0 + (step + 1) * del_y &&
               x_0 + (step + 1) * del_x <= 8 &&
               y_0 + (step + 1) * del_y <= 8 &&
               (field & squareMask[startSquare + (step + 1) * ray]))
        {
            ++step;
            result |= squareMask[startSquare + step * ray];

        }
        // extend once more to get attacked squares
        if (0 <= x_0 + (step + 1) * del_x &&
            0 <= y_0 + (step + 1) * del_y &&
            x_0 + (step + 1) * del_x <= 8 &&
            y_0 + (step + 1) * del_y <= 8)
        {
            result |= squareMask[startSquare + (step + 1) * ray];
        }
    }
    return result;
}

int L_1_norm(int sq1, int sq2)
{
    int x1 = sq1 / 9;
    int y1 = sq1 % 9;
    int x2 = sq2 / 9;
    int y2 = sq2 % 9;
    return std::abs(x1 - x2) + std::abs(y1 - y2);
}



