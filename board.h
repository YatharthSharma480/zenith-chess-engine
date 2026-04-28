#pragma once
#include "types.h"

struct Board {
    Bitboard pieces[12];
    bool whiteToMove;

    void initBoard();
    int  getPieceAt(int square);
    void makeMove(Move m);
};
