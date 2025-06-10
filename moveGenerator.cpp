#include <iostream>
#include <chrono>
#include "bitboard.h"
#include "position.h"
#include "moveGenerator.h"

void kingMoves(const Position& pos, moveList& moves);
void dropMoves(const Position& pos, moveList& moves, const Bitboard& restriction);
void checkDropMoves(const Position& pos, moveList& moves);
Bitboard pinnedPieceMoves(const Position& pos, moveList& moves, const Bitboard& moveRestriction);
void addMovesAll(const Position& pos, moveList& moves, const Bitboard& empty, const int from, const Bitboard& to, const int movedPiece);
void addMovesLogical(const Position& pos, moveList& moves, const Bitboard& empty, const int from, const Bitboard& to, const int movedPiece);
void addDropMoves(moveList& moves, Bitboard& dropSquares, const int droppedPiece);
void pieceMoves(const Position& pos, moveList& moves, const Bitboard& moveRestriction);

void (*addMoves)(const Position& pos, moveList& moves, const Bitboard& empty, const int from, const Bitboard& to, const int movedPiece) = addMovesLogical;

void kingMoves(const Position& pos, moveList& moves)
{
    // This function can be optimized by not considering all non-sliding pieces, but only those that attack a square the king can move too
    // Auxiliary bitboards
    const Bitboard occupied = pos.pieceMaps[0] | pos.pieceMaps[1] | pos.pieceMaps[2] | pos.pieceMaps[3] |
                              pos.pieceMaps[4] | pos.pieceMaps[5] | pos.pieceMaps[6] | pos.pieceMaps[7];
    const Bitboard ownPieces = (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & occupied;
    Bitboard enemyPieces = ~ownPieces & occupied;
    const Bitboard emptyOrKing = ~(occupied ^ (ownPieces & pos.pieceMaps[KING])) & Bitboard(true);
    // Determine attacked squares
    Bitboard attackedSquares(false);

    while (enemyPieces)
    {
        int square = enemyPieces.BSF();
        enemyPieces.removeLSB();
        uint8_t piece = pos.mailbox[square];
        attackedSquares |= attackMap(piece, square, emptyOrKing, !pos.playerOne);
    }

    int kingSquare = (pos.pieceMaps[KING] & ownPieces).BSF();
    Bitboard kingMoves = ~attackedSquares & (~ownPieces) & kingAttack(kingSquare);
    addMoves(pos, moves, emptyOrKing, kingSquare, kingMoves, KING);
}


void dropMoves(const Position& pos, moveList& moves, const Bitboard& restriction)
{
    const Bitboard occupied = pos.pieceMaps[0] | pos.pieceMaps[1] | pos.pieceMaps[2] | pos.pieceMaps[3] |
                              pos.pieceMaps[4] | pos.pieceMaps[5] | pos.pieceMaps[6] | pos.pieceMaps[7];
    const Bitboard empty = ~occupied & Bitboard(true);
    const Bitboard ownPieces = (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & occupied;

    Hand currentHand = pos.hand[pos.playerOne];

    if (hand_count(currentHand, ROOK))
    {
        Bitboard dropSquares = empty & restriction;
        addDropMoves(moves, dropSquares, ROOK);
    }
    if (hand_count(currentHand, BISHOP))
    {
        Bitboard dropSquares = empty & restriction;
        addDropMoves(moves, dropSquares, BISHOP);
    }
    if (hand_count(currentHand, GOLD_GENERAL))
    {
        Bitboard dropSquares = empty & restriction;
        addDropMoves(moves, dropSquares, GOLD_GENERAL);
    }
    if (hand_count(currentHand, SILVER_GENERAL))
    {
        Bitboard dropSquares = empty & restriction;
        addDropMoves(moves, dropSquares, SILVER_GENERAL);
    }
    if (hand_count(currentHand, LANCE))
    {
        Bitboard lanceDrops = (Bitboard(true) ^ rowMask[(pos.playerOne ? 0 : 8)]) & empty & restriction;
        addDropMoves(moves, lanceDrops, LANCE);
    }
    if (hand_count(currentHand, KNIGHT))
    {
        Bitboard knightDrops = (Bitboard(true) ^ rowMask[(pos.playerOne ? 0 : 8)] ^ rowMask[(pos.playerOne ? 1 : 7)]) & empty & restriction;
        addDropMoves(moves, knightDrops, KNIGHT);
    }
    if (hand_count(currentHand, PAWN))
    {
        Bitboard pawns = pos.pieceMaps[PAWN] & (~pos.pieceMaps[9]) & ownPieces;
        Bitboard pawnDrops = rowMask[(pos.playerOne ? 0 : 8)];
        while (pawns)
        {
            int index = pawns.BSF();
            pawns.removeLSB();
            pawnDrops |= columnMask[index % 9];
        }
        pawnDrops = (~pawnDrops) & empty & restriction;
        addDropMoves(moves, pawnDrops, PAWN);
    }
}


void checkDropMoves(const Position& pos, moveList& moves)
{
    const Bitboard occupied = pos.pieceMaps[0] | pos.pieceMaps[1] | pos.pieceMaps[2] | pos.pieceMaps[3] |
                              pos.pieceMaps[4] | pos.pieceMaps[5] | pos.pieceMaps[6] | pos.pieceMaps[7];
    const Bitboard empty = ~occupied & Bitboard(true);
    const Bitboard ownPieces = (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & occupied;


    int enemyKingSquare = (pos.pieceMaps[KING] & ~ownPieces).BSF();

    Hand currentHand = pos.hand[pos.playerOne];

    if (hand_count(currentHand, ROOK))
    {
        Bitboard dropSquares = empty & rookAttack(enemyKingSquare, empty);
        addDropMoves(moves, dropSquares, ROOK);
    }
    if (hand_count(currentHand, BISHOP))
    {
        Bitboard dropSquares = empty & bishopAttack(enemyKingSquare, empty);
        addDropMoves(moves, dropSquares, BISHOP);
    }
    if (hand_count(currentHand, GOLD_GENERAL))
    {
        Bitboard dropSquares = empty & goldAttack(enemyKingSquare, !pos.playerOne);
        addDropMoves(moves, dropSquares, GOLD_GENERAL);
    }
    if (hand_count(currentHand, SILVER_GENERAL))
    {
        Bitboard dropSquares = empty & silverAttack(enemyKingSquare, !pos.playerOne);
        addDropMoves(moves, dropSquares, SILVER_GENERAL);
    }
    if (hand_count(currentHand, LANCE))
    {
        Bitboard lanceDrops = (Bitboard(true) ^ rowMask[(pos.playerOne ? 0 : 8)]) & empty & lanceAttack(enemyKingSquare, empty, !pos.playerOne);
        addDropMoves(moves, lanceDrops, LANCE);
    }
    if (hand_count(currentHand, KNIGHT))
    {
        Bitboard knightDrops = (Bitboard(true) ^ rowMask[(pos.playerOne ? 0 : 8)] ^ rowMask[(pos.playerOne ? 1 : 7)]) & empty & knightAttack(enemyKingSquare, !pos.playerOne);
        addDropMoves(moves, knightDrops, KNIGHT);
    }
    if (hand_count(currentHand, PAWN))
    {
        Bitboard pawns = pos.pieceMaps[PAWN] & (~pos.pieceMaps[9]) & ownPieces;
        Bitboard pawnDrops = rowMask[(pos.playerOne ? 0 : 8)];
        while (pawns)
        {
            int index = pawns.BSF();
            pawns.removeLSB();
            pawnDrops |= columnMask[index % 9];
        }

        const Bitboard enemyNotPawns = ~ownPieces & occupied & (~pos.pieceMaps[PAWN]);
        // Pawn drops in front of other pieces
        pawnDrops = (~pawnDrops) & empty & (pos.playerOne ? enemyNotPawns << 9 : enemyNotPawns >> 9);
        addDropMoves(moves, pawnDrops, PAWN);
    }
}



void addMovesAll(const Position& pos, moveList& moves, const Bitboard& empty,
                 const int from, const Bitboard& to, const int movedPiece)
{

    Bitboard nonPromoting(false);
    Bitboard promoting(false);
    if (movedPiece != KING && movedPiece != GOLD_GENERAL && movedPiece <= 7) // promotable piece
    {
        nonPromoting = to & unforcedPromotion[movedPiece][pos.playerOne];
        promoting = (promotionZone[pos.playerOne] & squareMask[from] ? to : to & promotionZone[pos.playerOne]);
    }
    else // Unpromotable piece
    {
        nonPromoting = to;
    }

    while (nonPromoting)
    {
        int destination = nonPromoting.BSF();
        nonPromoting.removeLSB();
        int capturedPiece = pos.mailbox[destination];
        moves.addMove(from, destination, false, capturedPiece != 0, movedPiece, capturedPiece);
    }
    while (promoting)
    {
        int destination = promoting.BSF();
        promoting.removeLSB();
        int capturedPiece = pos.mailbox[destination];
        moves.addMove(from, destination, true, capturedPiece != 0, movedPiece, capturedPiece);
    }
}

void addMovesLogical(const Position& pos, moveList& moves, const Bitboard& empty,
                     const int from, const Bitboard& to, const int movedPiece)
{
    Bitboard nonPromoting(false);
    Bitboard promoting(false);
    if (movedPiece != KING && movedPiece != GOLD_GENERAL && movedPiece <= 7) // promotable piece
    {
        nonPromoting = to & unforcedPromotion[movedPiece][pos.playerOne];
        promoting = (promotionZone[pos.playerOne] & squareMask[from] ? to : to & promotionZone[pos.playerOne]);
        // Remove bad non-promotions
        nonPromoting &= (movedPiece == BISHOP || movedPiece == ROOK || movedPiece == PAWN ? ~promoting : Bitboard(true));
    }
    else // Unpromotable piece
    {
        nonPromoting = to;
    }

    while (nonPromoting)
    {
        int destination = nonPromoting.BSF();
        nonPromoting.removeLSB();
        int capturedPiece = pos.mailbox[destination];
        moves.addMove(from, destination, false, capturedPiece != 0, movedPiece, capturedPiece);
    }
    while (promoting)
    {
        int destination = promoting.BSF();
        promoting.removeLSB();
        int capturedPiece = pos.mailbox[destination];
        moves.addMove(from, destination, true, capturedPiece != 0, movedPiece, capturedPiece);
    }
}

void addDropMoves(moveList& moves, Bitboard& dropSquares, const int droppedPiece)
{
    while (dropSquares)
    {
        int dropSquare = dropSquares.BSF();
        dropSquares.removeLSB();
        moves.addMove(dropSquare, droppedPiece);
    }
}

void pieceMoves(const Position& pos, moveList& moves, const Bitboard& moveRestriction)
{
    const Bitboard occupied = pos.pieceMaps[0] | pos.pieceMaps[1] | pos.pieceMaps[2] | pos.pieceMaps[3] |
                              pos.pieceMaps[4] | pos.pieceMaps[5] | pos.pieceMaps[6] | pos.pieceMaps[7];
    const Bitboard empty = ~occupied & Bitboard(true);
    const Bitboard ownPieces = (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & occupied;
    const Bitboard enemyPieces = ~ownPieces & occupied;
    const Bitboard promoted = pos.pieceMaps[9];
    const Bitboard totalRestriction = (empty | enemyPieces) & moveRestriction;

    Bitboard pinnedPieces = pinnedPieceMoves(pos, moves, moveRestriction);
    Bitboard moveablePieces = ownPieces & (~pinnedPieces) & (~pos.pieceMaps[KING]);

    /** Iterate over all pieces **/

    Bitboard options;
    while (moveablePieces)
    {
        int square = moveablePieces.BSF();
        moveablePieces.removeLSB();
        uint8_t piece = pos.mailbox[square];
        options = attackMap(piece, square, empty, pos.playerOne) & totalRestriction;
        addMoves(pos, moves, empty, square, options, piece);
    }
}

Bitboard pinnedPieceMoves(const Position& pos, moveList& moves, const Bitboard& moveRestriction)
{
    const Bitboard occupied = pos.pieceMaps[0] | pos.pieceMaps[1] | pos.pieceMaps[2] | pos.pieceMaps[3] |
                              pos.pieceMaps[4] | pos.pieceMaps[5] | pos.pieceMaps[6] | pos.pieceMaps[7];
    const Bitboard empty = ~occupied & Bitboard(true);
    const Bitboard ownPieces = (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & occupied;
    const Bitboard enemyPieces = ~ownPieces & occupied;

    Bitboard pinnedPieces(false);


    int kingSquare = (pos.pieceMaps[KING] & ownPieces).BSF();
    /** Rook and Lance **/
    Bitboard kingRookAttacks = rookAttack(kingSquare, empty);
    Bitboard xRayKingRookAttacks = rookAttack(kingSquare, empty | (ownPieces & kingRookAttacks));
    //xRayKingRookAttacks.print();
    // Lance
    if ((kingRookAttacks & lanceMask[pos.playerOne][kingSquare] & ownPieces) && // Seeing own pieces
       ((xRayKingRookAttacks & lanceMask[pos.playerOne][kingSquare]) &
        (pos.pieceMaps[LANCE] & (~pos.pieceMaps[9]) & enemyPieces)))             // Unpromoted lance behind piece
    {
        int pinnedPieceSquare = (kingRookAttacks & lanceMask[pos.playerOne][kingSquare] & ownPieces).BSF();
        int pinnedPieceType = pos.mailbox[pinnedPieceSquare];
        Bitboard pinRestriction = xRayKingRookAttacks & lanceMask[pos.playerOne][kingSquare];
        Bitboard options = attackMap(pinnedPieceType, pinnedPieceSquare, empty, pos.playerOne) & pinRestriction & moveRestriction;
        pinnedPieces |= squareMask[pinnedPieceSquare];
        addMoves(pos, moves, empty, pinnedPieceSquare, options, pinnedPieceType);
    }
    // Rook
    Bitboard xRayedRooks = xRayKingRookAttacks & pos.pieceMaps[ROOK] & enemyPieces;
    while (xRayedRooks)
    {
        int rookIndex = xRayedRooks.BSF();
        xRayedRooks.removeLSB();
        Bitboard pinRestriction = xRayKingRookAttacks & (rookAttack(rookIndex, empty | (ownPieces & kingRookAttacks)) | squareMask[rookIndex]);
        if (pinRestriction & ownPieces)
        {
            int pinnedPieceSquare = (pinRestriction & ownPieces).BSF();
            int pinnedPieceType = pos.mailbox[pinnedPieceSquare];
            Bitboard options = attackMap(pinnedPieceType, pinnedPieceSquare, empty, pos.playerOne) & pinRestriction & moveRestriction;
            pinnedPieces |= squareMask[pinnedPieceSquare];
            addMoves(pos, moves, empty, pinnedPieceSquare, options, pinnedPieceType);
        }
    }
    // Bishop
    Bitboard kingBishopAttacks = bishopAttack(kingSquare, empty);
    Bitboard xRayKingBishopAttacks = bishopAttack(kingSquare, empty | (ownPieces & kingBishopAttacks));
    Bitboard xRayedBishops = xRayKingBishopAttacks & pos.pieceMaps[BISHOP] & enemyPieces;
    while (xRayedBishops)
    {
        int bishopIndex = xRayedBishops.BSF();
        xRayedBishops.removeLSB();
        Bitboard pinRestriction = xRayKingBishopAttacks & (bishopAttack(bishopIndex, empty | (ownPieces & kingBishopAttacks)) | squareMask[bishopIndex]);
        if (pinRestriction & ownPieces)
        {
            int pinnedPieceSquare = (pinRestriction & ownPieces).BSF();
            int pinnedPieceType = pos.mailbox[pinnedPieceSquare];
            Bitboard options = attackMap(pinnedPieceType, pinnedPieceSquare, empty, pos.playerOne) & pinRestriction & moveRestriction;
            pinnedPieces |= squareMask[pinnedPieceSquare];
            addMoves(pos, moves, empty, pinnedPieceSquare, options, pinnedPieceType);
        }
    }
    return pinnedPieces;
}


moveList generateMoves(const Position& pos, bool allMoves)
{
    if (allMoves)
    {
        addMoves = addMovesAll;
    }
    const Bitboard occupied = pos.pieceMaps[0] | pos.pieceMaps[1] | pos.pieceMaps[2] | pos.pieceMaps[3] |
                              pos.pieceMaps[4] | pos.pieceMaps[5] | pos.pieceMaps[6] | pos.pieceMaps[7];
    const Bitboard empty       = ~occupied & Bitboard(true);
    const Bitboard ownPieces   = (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & occupied;
    const Bitboard enemyPieces = ~ownPieces & occupied;
    const Bitboard promoted    = pos.pieceMaps[9];
    //kingMoves(pos);

    /** Sliding piece checks **/
    int checkers = 0;
    Bitboard checkingPieces(false);
    Bitboard blockSquares(false);
    int kingSquare = (pos.pieceMaps[KING] & ownPieces).BSF();
    // Lance & Rook
    Bitboard kingRookAttacks = rookAttack(kingSquare, empty);
    // Lance
    Bitboard attackingKing = (kingRookAttacks & lanceMask[pos.playerOne][kingSquare]) &
                             (pos.pieceMaps[LANCE] & (~promoted) & enemyPieces);
    if (attackingKing)
    {
        ++checkers;
        blockSquares   |= kingRookAttacks & lanceMask[pos.playerOne][kingSquare] & empty;
        checkingPieces |= attackingKing;
    }
    // Rook
    attackingKing = kingRookAttacks & pos.pieceMaps[ROOK] & enemyPieces;
    if (attackingKing)
    {
        ++checkers;
        blockSquares   |= kingRookAttacks & rookAttack(attackingKing.BSF(), empty);
        checkingPieces |= attackingKing;
    }
    // Bishop
    Bitboard kingBishopAttacks = bishopAttack(kingSquare, empty);
    attackingKing = kingBishopAttacks & pos.pieceMaps[BISHOP] & enemyPieces;
    if (attackingKing)
    {
        ++checkers;
        blockSquares   |= kingBishopAttacks & bishopAttack(attackingKing.BSF(), empty);
        checkingPieces |= attackingKing;
    }

    /** Non-sliding piece checks **/
    // Unpromoted knight
    attackingKing = knightAttack(kingSquare, pos.playerOne) &
                    (pos.pieceMaps[KNIGHT] & (~promoted) & enemyPieces);
    if (attackingKing)
    {
        ++checkers;
        checkingPieces |= attackingKing;
    }
    if (kingAttack(kingSquare) & enemyPieces)
    {
        // Pawn
        attackingKing = (pos.playerOne ? (pos.pieceMaps[KING] & ownPieces) >> 9 : (pos.pieceMaps[KING] & ownPieces) << 9) &
                        (pos.pieceMaps[PAWN] & (~promoted) & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }

        // Silver
        attackingKing = silverAttack(kingSquare, pos.playerOne) &
                        (pos.pieceMaps[SILVER_GENERAL] & (~promoted) & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
        // Gold, promoted silver, promoted knight, promoted pawn
        attackingKing = goldAttack(kingSquare, pos.playerOne) & enemyPieces &
                        (pos.pieceMaps[GOLD_GENERAL] | ((pos.pieceMaps[PAWN] | pos.pieceMaps[SILVER_GENERAL] | pos.pieceMaps[KNIGHT] | pos.pieceMaps[LANCE]) & promoted));
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
        // Promoted rook non-sliding attacks
        attackingKing = dragonMask[kingSquare] &
                        (pos.pieceMaps[ROOK] & promoted & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
        // Promoted bishop non-sliding attacks
        attackingKing = horseMask[kingSquare] &
                       (pos.pieceMaps[BISHOP] & promoted & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
    }
    /** Option Tree **/
    moveList legalMoves;
    legalMoves.inCheck = checkers > 0;
    if (checkers == 0)
    {
        // Not in check. Pinned pieces can move, non-pinned piece can move unrestricted, unrestricted drops
        dropMoves(pos, legalMoves, Bitboard(true));
        pieceMoves(pos, legalMoves, Bitboard(true));

    }
    if (checkers == 1)
    {
        if (blockSquares)
        {
            // Blockable check => Capture checking piece, block check with a move, block check with a drop
            // Crucially, pinned pieces cannot move
            dropMoves(pos, legalMoves, blockSquares);
            pieceMoves(pos, legalMoves, blockSquares | checkingPieces);
        }
        else
        {
            // Unblockable check => Capture checking piece
            pieceMoves(pos, legalMoves, checkingPieces);
        }
    }
    // Can always make a king move
    kingMoves(pos, legalMoves);
    if (allMoves)
    {
        addMoves = addMovesLogical;
    }
    return legalMoves;
}

void generateMoves(const Position& pos, moveList& legalMoves, bool allMoves)
{
    // Set starting pointer to zero, allows for reuse.
    legalMoves.clear();

    if (allMoves)
    {
        addMoves = addMovesAll;
    }
    const Bitboard occupied = pos.pieceMaps[0] | pos.pieceMaps[1] | pos.pieceMaps[2] | pos.pieceMaps[3] |
                              pos.pieceMaps[4] | pos.pieceMaps[5] | pos.pieceMaps[6] | pos.pieceMaps[7];
    const Bitboard empty       = ~occupied & Bitboard(true);
    const Bitboard ownPieces   = (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & occupied;
    const Bitboard enemyPieces = ~ownPieces & occupied;
    const Bitboard promoted    = pos.pieceMaps[9];
    //kingMoves(pos);

    /** Sliding piece checks **/
    int checkers = 0;
    Bitboard checkingPieces(false);
    Bitboard blockSquares(false);
    int kingSquare = (pos.pieceMaps[KING] & ownPieces).BSF();
    // Lance & Rook
    Bitboard kingRookAttacks = rookAttack(kingSquare, empty);
    // Lance
    Bitboard attackingKing = (kingRookAttacks & lanceMask[pos.playerOne][kingSquare]) &
                             (pos.pieceMaps[LANCE] & (~promoted) & enemyPieces);
    if (attackingKing)
    {
        ++checkers;
        blockSquares   |= kingRookAttacks & lanceMask[pos.playerOne][kingSquare] & empty;
        checkingPieces |= attackingKing;
    }
    // Rook
    attackingKing = kingRookAttacks & pos.pieceMaps[ROOK] & enemyPieces;
    if (attackingKing)
    {
        ++checkers;
        blockSquares   |= kingRookAttacks & rookAttack(attackingKing.BSF(), empty);
        checkingPieces |= attackingKing;
    }
    // Bishop
    Bitboard kingBishopAttacks = bishopAttack(kingSquare, empty);
    attackingKing = kingBishopAttacks & pos.pieceMaps[BISHOP] & enemyPieces;
    if (attackingKing)
    {
        ++checkers;
        blockSquares   |= kingBishopAttacks & bishopAttack(attackingKing.BSF(), empty);
        checkingPieces |= attackingKing;
    }

    /** Non-sliding piece checks **/
    // Unpromoted knight
    attackingKing = knightAttack(kingSquare, pos.playerOne) &
                    (pos.pieceMaps[KNIGHT] & (~promoted) & enemyPieces);
    if (attackingKing)
    {
        ++checkers;
        checkingPieces |= attackingKing;
    }
    if (kingAttack(kingSquare) & enemyPieces)
    {
        // Pawn
        attackingKing = (pos.playerOne ? (pos.pieceMaps[KING] & ownPieces) >> 9 : (pos.pieceMaps[KING] & ownPieces) << 9) &
                        (pos.pieceMaps[PAWN] & (~promoted) & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }

        // Silver
        attackingKing = silverAttack(kingSquare, pos.playerOne) &
                        (pos.pieceMaps[SILVER_GENERAL] & (~promoted) & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
        // Gold, promoted silver, promoted knight, promoted pawn
        attackingKing = goldAttack(kingSquare, pos.playerOne) & enemyPieces &
                        (pos.pieceMaps[GOLD_GENERAL] | ((pos.pieceMaps[PAWN] | pos.pieceMaps[SILVER_GENERAL] | pos.pieceMaps[KNIGHT] | pos.pieceMaps[LANCE]) & promoted));
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
        // Promoted rook non-sliding attacks
        attackingKing = dragonMask[kingSquare] &
                        (pos.pieceMaps[ROOK] & promoted & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
        // Promoted bishop non-sliding attacks
        attackingKing = horseMask[kingSquare] &
                       (pos.pieceMaps[BISHOP] & promoted & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
    }
    /** Option Tree **/
    legalMoves.inCheck = checkers > 0;
    if (checkers == 0)
    {
        // Not in check. Pinned pieces can move, non-pinned piece can move unrestricted, unrestricted drops
        dropMoves(pos, legalMoves, Bitboard(true));
        pieceMoves(pos, legalMoves, Bitboard(true));

    }
    if (checkers == 1)
    {
        if (blockSquares)
        {
            // Blockable check => Capture checking piece, block check with a move, block check with a drop
            // Crucially, pinned pieces cannot move
            dropMoves(pos, legalMoves, blockSquares);
            pieceMoves(pos, legalMoves, blockSquares | checkingPieces);
        }
        else
        {
            // Unblockable check => Capture checking piece
            pieceMoves(pos, legalMoves, checkingPieces);
        }
    }
    // Can always make a king move
    kingMoves(pos, legalMoves);
    if (allMoves)
    {
        addMoves = addMovesLogical;
    }
}

void generateTacticalMoves(const Position& pos, moveList& legalMoves, bool allMoves)
{
    // Definition Tactical Move:
    // if in check
    //    Generate all moves
    // if not in check
    //    (?) Generate promotions
    //    Captures (except pawns that do not attack anything)
    //    Drops that give check

    // Set starting pointer to zero, allows for reuse.
    legalMoves.clear();

    if (allMoves)
    {
        addMoves = addMovesAll;
    }
    const Bitboard occupied = pos.pieceMaps[0] | pos.pieceMaps[1] | pos.pieceMaps[2] | pos.pieceMaps[3] |
                              pos.pieceMaps[4] | pos.pieceMaps[5] | pos.pieceMaps[6] | pos.pieceMaps[7];
    const Bitboard empty       = ~occupied & Bitboard(true);
    const Bitboard ownPieces   = (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & occupied;
    const Bitboard enemyPieces = ~ownPieces & occupied;
    const Bitboard promoted    = pos.pieceMaps[9];
    //kingMoves(pos);

    /** Sliding piece checks **/
    int checkers = 0;
    Bitboard checkingPieces(false);
    Bitboard blockSquares(false);
    int kingSquare = (pos.pieceMaps[KING] & ownPieces).BSF();
    // Lance & Rook
    Bitboard kingRookAttacks = rookAttack(kingSquare, empty);
    // Lance
    Bitboard attackingKing = (kingRookAttacks & lanceMask[pos.playerOne][kingSquare]) &
                             (pos.pieceMaps[LANCE] & (~promoted) & enemyPieces);
    if (attackingKing)
    {
        ++checkers;
        blockSquares   |= kingRookAttacks & lanceMask[pos.playerOne][kingSquare] & empty;
        checkingPieces |= attackingKing;
    }
    // Rook
    attackingKing = kingRookAttacks & pos.pieceMaps[ROOK] & enemyPieces;
    if (attackingKing)
    {
        ++checkers;
        blockSquares   |= kingRookAttacks & rookAttack(attackingKing.BSF(), empty);
        checkingPieces |= attackingKing;
    }
    // Bishop
    Bitboard kingBishopAttacks = bishopAttack(kingSquare, empty);
    attackingKing = kingBishopAttacks & pos.pieceMaps[BISHOP] & enemyPieces;
    if (attackingKing)
    {
        ++checkers;
        blockSquares   |= kingBishopAttacks & bishopAttack(attackingKing.BSF(), empty);
        checkingPieces |= attackingKing;
    }

    /** Non-sliding piece checks **/
    // Unpromoted knight
    attackingKing = knightAttack(kingSquare, pos.playerOne) &
                    (pos.pieceMaps[KNIGHT] & (~promoted) & enemyPieces);
    if (attackingKing)
    {
        ++checkers;
        checkingPieces |= attackingKing;
    }
    if (kingAttack(kingSquare) & enemyPieces)
    {
        // Pawn
        attackingKing = (pos.playerOne ? (pos.pieceMaps[KING] & ownPieces) >> 9 : (pos.pieceMaps[KING] & ownPieces) << 9) &
                        (pos.pieceMaps[PAWN] & (~promoted) & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }

        // Silver
        attackingKing = silverAttack(kingSquare, pos.playerOne) &
                        (pos.pieceMaps[SILVER_GENERAL] & (~promoted) & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
        // Gold, promoted silver, promoted knight, promoted pawn
        attackingKing = goldAttack(kingSquare, pos.playerOne) & enemyPieces &
                        (pos.pieceMaps[GOLD_GENERAL] | ((pos.pieceMaps[PAWN] | pos.pieceMaps[SILVER_GENERAL] | pos.pieceMaps[KNIGHT] | pos.pieceMaps[LANCE]) & promoted));
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
        // Promoted rook non-sliding attacks
        attackingKing = dragonMask[kingSquare] &
                        (pos.pieceMaps[ROOK] & promoted & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
        // Promoted bishop non-sliding attacks
        attackingKing = horseMask[kingSquare] &
                       (pos.pieceMaps[BISHOP] & promoted & enemyPieces);
        if (attackingKing)
        {
            ++checkers;
            checkingPieces |= attackingKing;
        }
    }
    /** Option Tree **/
    legalMoves.inCheck = checkers > 0;
    if (checkers == 0)
    {
        // checkDropMoves(pos, legalMoves);
        // Generate captures, i.e, restrict moves to squares with opponent pieces
        // Exclude pawns that do not attack anything
        Bitboard worthy = enemyPieces;
        //worthy ^= ~(pos.playerOne ? ownPieces >> 9 : ownPieces << 9) & enemyPieces & pos.pieceMaps[PAWN] & ~promoted;

        pieceMoves(pos, legalMoves, worthy);

    }
    if (checkers == 1)
    {
        if (blockSquares)
        {
            // Blockable check => Capture checking piece, block check with a move, block check with a drop
            // Crucially, pinned pieces cannot move
            dropMoves(pos, legalMoves, blockSquares);
            pieceMoves(pos, legalMoves, blockSquares | checkingPieces);
        }
        else
        {
            // Unblockable check => Capture checking piece
            pieceMoves(pos, legalMoves, checkingPieces);
        }
        kingMoves(pos, legalMoves);
    }
    else
    {
        // Double check
        kingMoves(pos, legalMoves);
    }
    if (allMoves)
    {
        addMoves = addMovesLogical;
    }
}

/** Performance tests **/

uint64_t nearPerft(Position& pos, int depth)
{
    uint64_t total = 0;
    moveList moves = generateMoves(pos, true);
    if (depth == 1)
    {
        return moves.size;
    }
    for (int i = 0; i < moves.size; i++)
    {
        pos.makeMove(moves.getMove(i));
        total += nearPerft(pos, depth - 1);
        pos.undoMove(moves.getMove(i));
    }
    return total;
}


void speedTest(Position& pos, const int depth)
{
    for (int d = 1; d <= depth; d++)
    {
        auto start = std::chrono::high_resolution_clock::now();
        uint64_t nodes = nearPerft(pos, d);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::cout << "Depth: " << d <<
                     " Nodes: " << nodes <<
                     " mnps: " << (duration.count() == 0 ? 0 : (int) (((double) nodes / duration.count()) * 1000)) << "\n";
    }
}

uint64_t perft(Position& pos, int depth, Move lastMove)
{
    if (depth <= 0)
    {
        // Exclude pawn drop mate
        if (lastMove.isDrop() && lastMove.movedPiece() == PAWN &&
            pos.pieceMaps[KING] & (pos.playerOne ? pos.pieceMaps[8] : ~pos.pieceMaps[8]) & squareMask[lastMove.to() + (pos.playerOne ? 9 : -9)])
        {
            moveList checkmateCheck = generateMoves(pos, true);
            return checkmateCheck.size != 0;
        }
        else
        {
            return 1;
        }
    }
    uint64_t total = 0;
    moveList moves = generateMoves(pos, true);
    // No pawns to give mate
    if (depth == 1 && hand_count(pos.hand[pos.playerOne], PAWN) == 0)
    {
        return moves.size;
    }
    for (int i = 0; i < moves.size; i++)
    {
        pos.makeMove(moves.getMove(i));
        total += perft(pos, depth - 1, moves.getMove(i));
        pos.undoMove(moves.getMove(i));
    }
    return total;
}

void perftBreakdown(Position& pos, int depth)
{
    uint64_t total = 0;
    moveList moves = generateMoves(pos, true);
    for (int i = 0; i < moves.size; i++)
    {
        pos.makeMove(moves.getMove(i));
        uint64_t partial = perft(pos, depth - 1, moves.getMove(i));
        std::cout << moves.getMove(i) << " " << partial << "\n";
        //std::cout << moves.getMove(i).isCapture() << "\n";
        //pos.print();
        pos.undoMove(moves.getMove(i));
        total += partial;
    }
    std::cout << "Total: " << total << "\n";
}


