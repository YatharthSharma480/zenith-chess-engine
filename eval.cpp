#include "eval.h"

// Standard piece values in centipawns
static const int pieceValues[12] = {
    100,  // WHITE_PAWN
    320,  // WHITE_KNIGHT
    330,  // WHITE_BISHOP
    500,  // WHITE_ROOK
    900,  // WHITE_QUEEN
    20000,// WHITE_KING
    -100, // BLACK_PAWN
    -320, // BLACK_KNIGHT
    -330, // BLACK_BISHOP
    -500, // BLACK_ROOK
    -900, // BLACK_QUEEN
    -20000// BLACK_KING
};

// Brian Kernighan's trick - count the set bits in a bitboard
static int countBits(uint64_t bb) {
    int count = 0;
    while (bb) {
        bb &= (bb - 1); // clears the lowest set bit
        count++;
    }
    return count;
}

int evaluateBoard(Board& b) {
    int score = 0;
    for (int i = 0; i < 12; i++) {
        score += countBits(b.pieces[i]) * pieceValues[i];
    }
    // Positive = good for white, negative = good for black
    return score;
}
