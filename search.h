#pragma once
#include "board.h"
#include "types.h"

const int INF = 1000000;

int  alphaBeta(Board& b, int depth, int alpha, int beta);
Move getBestMove(Board& b, int depth);
