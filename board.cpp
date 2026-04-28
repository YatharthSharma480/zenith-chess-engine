#include "board.h"
#include <cstdint>

#define SET_BIT(bb, sq)   ((bb) |=  (1ULL << (sq)))
#define CLEAR_BIT(bb, sq) ((bb) &= ~(1ULL << (sq)))
#define CHECK_BIT(bb, sq) ((bb) &   (1ULL << (sq)))

void Board::initBoard() {
    for (int i = 0; i < 12; i++) pieces[i] = 0ULL;

    pieces[WHITE_ROOK]   = (1ULL << 0) | (1ULL << 7);
    pieces[WHITE_KNIGHT] = (1ULL << 1) | (1ULL << 6);
    pieces[WHITE_BISHOP] = (1ULL << 2) | (1ULL << 5);
    pieces[WHITE_QUEEN]  = (1ULL << 3);
    pieces[WHITE_KING]   = (1ULL << 4);
    pieces[WHITE_PAWN]   = 0xFFULL << 8;

    pieces[BLACK_ROOK]   = (1ULL << 56) | (1ULL << 63);
    pieces[BLACK_KNIGHT] = (1ULL << 57) | (1ULL << 62);
    pieces[BLACK_BISHOP] = (1ULL << 58) | (1ULL << 61);
    pieces[BLACK_QUEEN]  = (1ULL << 59);
    pieces[BLACK_KING]   = (1ULL << 60);
    pieces[BLACK_PAWN]   = 0xFFULL << 48;

    whiteToMove = true;
}

int Board::getPieceAt(int square) {
    for (int i = 0; i < 12; i++) {
        if (CHECK_BIT(pieces[i], square)) return i;
    }
    return EMPTY;
}

void Board::makeMove(Move m) {
    CLEAR_BIT(pieces[m.piece], m.from);

    if (m.captured != EMPTY) {
        CLEAR_BIT(pieces[m.captured], m.to);
    }

    int landingPiece = (m.promotedTo != EMPTY) ? m.promotedTo : m.piece;
    SET_BIT(pieces[landingPiece], m.to);

    if (m.isCastling) {
        if (m.to == 6) {
            CLEAR_BIT(pieces[WHITE_ROOK], 7);
            SET_BIT(pieces[WHITE_ROOK], 5);
        } else if (m.to == 2) {
            CLEAR_BIT(pieces[WHITE_ROOK], 0);
            SET_BIT(pieces[WHITE_ROOK], 3);
        } else if (m.to == 62) {
            CLEAR_BIT(pieces[BLACK_ROOK], 63);
            SET_BIT(pieces[BLACK_ROOK], 61);
        } else if (m.to == 58) {
            CLEAR_BIT(pieces[BLACK_ROOK], 56);
            SET_BIT(pieces[BLACK_ROOK], 59);
        }
    }

    whiteToMove = !whiteToMove;
}
