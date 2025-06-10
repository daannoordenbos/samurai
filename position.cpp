#include <iostream>
#include <cctype>
#include "bitboard.h"
#include "position.h"
#include "random.h"

void Position::print()
{
    char pieceMap[18] = " krbgsnlpKRBGSNLP";
    const Bitboard empty = ~(pieceMaps[0] | pieceMaps[1] | pieceMaps[2] | pieceMaps[3] |
                             pieceMaps[4] | pieceMaps[5] | pieceMaps[6] | pieceMaps[7]) & Bitboard(true);

    std::cout << "\n  9   8   7   6   5   4   3   2   1  \n";
    std::cout << "+---+---+---+---+---+---+---+---+---+\n";

    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            if (empty & squareMask[i * 9 + j])
            {
                std::cout << "|   ";
            }
            else
            {
                bool playerOnePiece = pieceMaps[8] & squareMask[i * 9 + j];
                int type = mailbox[i * 9 + j] & 7;
                std::cout << "|" << (pieceMaps[9] & squareMask[i * 9 + j] ? "+" : " ") << pieceMap[1 + type + 8 * playerOnePiece] << " ";
            }

        }
        std::cout << "| " << char(i + 'a');
        if (i == 0)
        {
            std::cout << "    Hand: " << (hand[0] ? "" : "Empty");
            for (int k = 1; k < 8; k++)
            {
                if (hand_count(hand[0], k) > 0)
                {
                    std::cout << hand_count(hand[0], k) << pieceMap[1 + k] << " ";
                }
            }
        }
        if (i == 4)
        {
            std::cout << "    Player " << (playerOne ? "one" : "two") << " to move";
        }
        if (i == 8)
        {
            std::cout << "    Hand: " << (hand[1] ? "" : "Empty");
            for (int k = 1; k < 8; k++)
            {
                if (hand_count(hand[1], k) > 0)
                {
                    std::cout << hand_count(hand[1], k) << pieceMap[9 + k] << " ";
                }
            }
        }
        std::cout << "\n+---+---+---+---+---+---+---+---+---+\n";
    }
    std::cout << "\n";
}

void Position::createSFEN()
{
    const Bitboard empty = ~(pieceMaps[0] | pieceMaps[1] | pieceMaps[2] | pieceMaps[3] |
                             pieceMaps[4] | pieceMaps[5] | pieceMaps[6] | pieceMaps[7]) & Bitboard(true);

    char pieceMap[17] = "krbgsnlpKRBGSNLP";
    int emptySquares = 0;
    std::cout << '"';
    for (int square = 0; square < 81; square++)
    {
        if (empty & squareMask[square])
        {
            emptySquares++;
        }
        else
        {
            if (emptySquares != 0)
            {
                std::cout << emptySquares;
                emptySquares = 0;
            }
            if (pieceMaps[9] & squareMask[square])
            {
                std::cout << "+";
            }
            std::cout << pieceMap[(mailbox[square] & 7) + (pieceMaps[8] & squareMask[square] ? 8 : 0)];
        }

        if (square % 9 == 8 && square < 80)
        {
            if (emptySquares != 0)
            {
                std::cout << emptySquares;
                emptySquares = 0;
            }
            std::cout << "/";
        }
    }
    std::cout << " " << (playerOne ? "b" : "w") << " ";
    if (!hand[0] & !hand[1])
    {
        std::cout << "-";
    }
    else
    {
        for (int i = 1; i < 8; i++)
        {
            if (hand_count(hand[true], i) == 1)
            {
                std::cout << pieceMap[8 + i];
            }
            if (hand_count(hand[true], i) > 1)
            {
                std::cout << hand_count(hand[true], i) << pieceMap[8 + i];
            }
        }
        for (int i = 1; i < 8; i++)
        {
            if (hand_count(hand[false], i) == 1)
            {
                std::cout << pieceMap[i];
            }
            if (hand_count(hand[false], i) > 1)
            {
                std::cout << hand_count(hand[false], i) << pieceMap[i];
            }
        }
    }
    /** Ply information is not stored yet**/
    std::cout << " 1";
    std::cout << '"' << "\n";
}


std::string Position::regularFormat() const
{
    std::string result;
    const Bitboard empty = ~(pieceMaps[0] | pieceMaps[1] | pieceMaps[2] | pieceMaps[3] |
                             pieceMaps[4] | pieceMaps[5] | pieceMaps[6] | pieceMaps[7]) & Bitboard(true);

    for (int square = 0; square < 81; square++)
    {
        if (empty & squareMask[square])
        {
            result += '-';
        }
        else
        {
            result += (char)(pieceMaps[8] & squareMask[square] ? 'A' + mailbox[square] : 'a' + mailbox[square]);
        }
    }
    result += " " + std::to_string(hand[0]) + " " + std::to_string(hand[1]) + " " + std::to_string(playerOne);

    return result;
}

void Position::loadRegularFormat(std::string& board, int handOne, int handTwo, bool onMove)
{
    playerOne = onMove;
    hand[0] = (Hand) handOne;
    hand[1] = (Hand) handTwo;
    // clean bitboards
    for (int i = 0; i < 10; i++)
    {
        pieceMaps[i] = Bitboard(false);
    }
    // set bitboards and mailbox
    for (int square = 0; square < 81; square++)
    {
        if (board[square] == '-')
        {
            mailbox[square] = 0;
        }
        else
        {
            bool upper = std::isupper(board[square]);
            int8_t value = board[square] - (upper ? 'A' : 'a');
            mailbox[square] = value;
            if (upper)
            {
                pieceMaps[8] |= squareMask[square];
            }
            if (value > 9)
            {
                pieceMaps[9] |= squareMask[square];
            }
            pieceMaps[value & 7] |= squareMask[square];
        }
    }
}

void Position::loadSFEN(const char* sfen)
{
    int pieceMap[26] = {0};
    pieceMap['k' - 'a'] = 0;
    pieceMap['r' - 'a'] = 1;
    pieceMap['b' - 'a'] = 2;
    pieceMap['g' - 'a'] = 3;
    pieceMap['s' - 'a'] = 4;
    pieceMap['n' - 'a'] = 5;
    pieceMap['l' - 'a'] = 6;
    pieceMap['p' - 'a'] = 7;

    int i = 0;

    // Set the board state
    // clear board
    for(int j = 0; j < 10; j ++)
    {
        pieceMaps[j] = Bitboard(false);
    }
    int index = 0;
    bool promoted = false;
    while (sfen[i] != ' ')
    {
        if (std::isalpha(sfen[i]))
        {
            int piece = pieceMap[std::tolower(sfen[i]) - 'a'];
            pieceMaps[piece] |= squareMask[index];
            if (std::isupper(sfen[i])) {
                pieceMaps[8] |= squareMask[index];
            }
            if (promoted) {
                pieceMaps[9] |= squareMask[index];
            }
            index += 1;
        }
        index += (std::isdigit(sfen[i])) ? sfen[i] - '0' : 0;
        promoted = (sfen[i] == '+') ? true : false;
        i++;
    }
    // Set player to move
    i++;
    playerOne = (sfen[i] == 'b') ? true : false;
    i += 2;
    // Set the hands
    hand[0] = EMPTY_HAND;
    hand[1] = EMPTY_HAND;
    int amount = 1;
    while (sfen[i] != ' ')
    {
        if (std::isalpha(sfen[i]))
        {
            int piece = pieceMap[std::tolower(sfen[i]) - 'a'];
            add_hand(hand[std::isupper(sfen[i])], piece, amount);
        }
        amount = std::isdigit(sfen[i]) ? sfen[i] - '0' : 1;
        i++;
    }

    loadMailbox();
}

void Position::loadInitial() {
    loadSFEN("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
}

void Position::loadMailbox()
{
    // clear mailbox
    for (int square = 0; square < 81; square++)
    {
        mailbox[square] = 0;
    }

    Bitboard pieceGroup;
    for (uint8_t i = 0; i < 8; i++)
    {
        pieceGroup = pieceMaps[i];
        while (pieceGroup)
        {
            int square = pieceGroup.BSF();
            pieceGroup.removeLSB();
            mailbox[square] = i;
        }
    }
    pieceGroup = pieceMaps[9];
    while (pieceGroup)
    {
        int square = pieceGroup.BSF();
        pieceGroup.removeLSB();
        mailbox[square] += 8;
    }
}

Move Position::USIToMove(const char* move)
{
    int pieceMap[26] = {0};
    pieceMap['k' - 'a'] = 0;
    pieceMap['r' - 'a'] = 1;
    pieceMap['b' - 'a'] = 2;
    pieceMap['g' - 'a'] = 3;
    pieceMap['s' - 'a'] = 4;
    pieceMap['n' - 'a'] = 5;
    pieceMap['l' - 'a'] = 6;
    pieceMap['p' - 'a'] = 7;

    Move internalMove;
    // Piece drop
    if (std::isalpha(move[0]))
    {
        internalMove.set(('9' - move[2]) + 9 * (move[3] - 'a'),
                         pieceMap[move[0] - 'A']);
    }
    else
    {
        int destinationPiece = mailbox[('9' - move[2]) + 9 * (move[3] - 'a')];
        internalMove.set(('9' - move[0]) + 9 * (move[1] - 'a'),
                         ('9' - move[2]) + 9 * (move[3] - 'a'),
                         move[4] == '+',
                         destinationPiece != 0,
                         mailbox[('9' - move[0]) + 9 * (move[1] - 'a')],
                         (destinationPiece != 0 ? destinationPiece : 0)
                         );
    }
    return internalMove;
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
    char pieceMap[9] = "KRBGSNLP";
    if (move.isDrop())
    {
        os << pieceMap[move.movedType()] << "*";
    }
    else
    {
        os << 9 - move.from() % 9 << (char) ('a' + move.from() / 9);
    }
    os << 9 - move.to() % 9 << (char) ('a' + move.to() / 9);

    if (move.isPromotion())
    {
        os << "+";
    }
    return os;
}

void Position::makeMove(Move& move)
{
    if (move.isDrop())
    {
        // Remove piece from hand, add piece to the board
        sub_hand(hand[playerOne], move.movedType());
        if (playerOne)
        {
            pieceMaps[8] |= squareMask[move.to()];
        }
        pieceMaps[move.movedType()] |= squareMask[move.to()];

        // Add dropped piece in mailbox
        mailbox[move.to()] = move.movedType();
    }
    else
    {
        // Remove piece
        if (move.isCapture())
        {
            pieceMaps[move.capturedType()] &= ~squareMask[move.to()];
            pieceMaps[8] &= ~squareMask[move.to()];
            pieceMaps[9] &= ~squareMask[move.to()];

            add_hand(hand[playerOne], move.capturedType());
        }
        // Move piece
        if (move.value & (1 << 19))
        {
            pieceMaps[9] ^= squareMask[move.from()] | squareMask[move.to()];
        }
        pieceMaps[move.movedType()] ^= squareMask[move.from()] | squareMask[move.to()];

        if (playerOne)
        {
            pieceMaps[8] ^= squareMask[move.from()] | squareMask[move.to()];
        }

        // Move piece in mailbox
        mailbox[move.from()] = 0;
        mailbox[move.to()] = move.movedPiece();
    }
    if (move.isPromotion())
    {
        pieceMaps[9] |= squareMask[move.to()];

        // Promote piece in mailbox
        mailbox[move.to()] += 8;
    }
    playerOne = !playerOne;
}

void Position::undoMove(Move& move)
{
    playerOne = !playerOne;
    if (move.isPromotion())
    {
        pieceMaps[9] ^= squareMask[move.to()];

        // Unpromote piece in mailbox
        mailbox[move.to()] -= 8;
    }

    if (move.isDrop())
    {
        // Add piece to, remove piece from the board
        add_hand(hand[playerOne], move.movedType());
        if (playerOne)
        {
            pieceMaps[8] ^= squareMask[move.to()];
        }
        pieceMaps[move.movedType()] ^= squareMask[move.to()];

        // Remove dropped piece
        mailbox[move.to()] = 0;
    }
    else
    {
        if (playerOne)
        {
            pieceMaps[8] ^= squareMask[move.from()] | squareMask[move.to()];
        }
        // Move piece
        if (move.value & (1 << 19))
        {
            pieceMaps[9] ^= squareMask[move.from()] | squareMask[move.to()];
        }
        pieceMaps[move.movedType()] ^= squareMask[move.from()] | squareMask[move.to()];

        // Move piece in mailbox
        mailbox[move.from()] = move.movedPiece();
        mailbox[move.to()] = 0;

        // Remove piece
        if (move.isCapture())
        {
            pieceMaps[move.capturedType()] |= squareMask[move.to()];
            if (move.value & (1 << 23))
            {
                pieceMaps[9] |= squareMask[move.to()];
            }
            if (!playerOne)
            {
                pieceMaps[8] |= squareMask[move.to()];
            }

            sub_hand(hand[playerOne], move.capturedType());

            mailbox[move.to()] = move.capturedPiece();
        }
    }
}

void Position::kikiBitboards(Bitboard (&out)[4]) const {
    const Bitboard occupied = pieceMaps[0] | pieceMaps[1] | pieceMaps[2] | pieceMaps[3] |
                              pieceMaps[4] | pieceMaps[5] | pieceMaps[6] | pieceMaps[7];
    const Bitboard empty = ~occupied     & Bitboard(true);
    Bitboard player_one  =  pieceMaps[8];
    Bitboard player_two  = ~pieceMaps[8] & occupied;

    Bitboard kiki;
    while (player_one)
    {
        int square = player_one.BSF();
        player_one.removeLSB();
        uint8_t piece = mailbox[square];
        kiki = attackMap(piece, square, empty, true);

        out[0] ^= kiki & ~(out[0] & out[1]);
        out[1] ^= kiki & (~out[0]) & (~out[1]);
    }
/*
    (~out[0] & ~out[1]).print();
    (out[0] & ~out[1]).print();
    (~out[0] & out[1]).print();
    (out[0] & out[1]).print();
*/
    while (player_two)
    {
        int square = player_two.BSF();
        player_two.removeLSB();
        uint8_t piece = mailbox[square];
        kiki = attackMap(piece, square, empty, false);

        out[2] ^= kiki & ~(out[2] & out[3]);
        out[3] ^= kiki & (~out[2]) & (~out[3]);
    }
/*
    (~out[2] & ~out[3]).print();
    (out[2] & ~out[3]).print();
    (~out[2] & out[3]).print();
    (out[2] & out[3]).print();*/
}
