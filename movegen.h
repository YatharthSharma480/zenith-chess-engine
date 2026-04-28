#pragma once
#include "types.h"
#include "board.h"
#include <vector>

std::vector<Move> generatePseudoLegalMoves(Board& b);
bool isSquareAttacked(Board& b, int square, bool byWhite);
std::vector<Move> generateLegalMoves(Board& b);
