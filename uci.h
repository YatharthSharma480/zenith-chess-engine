#pragma once
#include "board.h"
#include "types.h"
#include <string>

std::string squareToAlgebraic(int sq);
Move        algebraicToMove(const std::string& str, Board& b);
void        uciLoop(Board& b);
