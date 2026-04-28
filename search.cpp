#include "search.h"
#include "movegen.h"
#include "eval.h"
#include <vector>

int alphaBeta(Board& b, int depth, int alpha, int beta) {
    if (depth == 0) {
        return evaluateBoard(b);
    }

    std::vector<Move> moves = generateLegalMoves(b);

    if (moves.empty()) {
        int myKing = b.whiteToMove ? WHITE_KING : BLACK_KING;
        bool byWhite = !b.whiteToMove;
        int kingSq = -1;
        for (int sq = 0; sq < 64; sq++) {
            if (b.pieces[myKing] & (1ULL << sq)) { kingSq = sq; break; }
        }
        if (kingSq != -1 && isSquareAttacked(b, kingSq, byWhite)) {
            return b.whiteToMove ? -INF : INF;
        }
        return 0;
    }

    if (b.whiteToMove) {
        int best = -INF;
        for (Move& m : moves) {
            Board nextBoard = b;
            nextBoard.makeMove(m);
            int score = alphaBeta(nextBoard, depth - 1, alpha, beta);
            if (score > best) best = score;
            if (score > alpha) alpha = score;
            if (beta <= alpha) break;
        }
        return best;
    } else {
        int best = INF;
        for (Move& m : moves) {
            Board nextBoard = b;
            nextBoard.makeMove(m);
            int score = alphaBeta(nextBoard, depth - 1, alpha, beta);
            if (score < best) best = score;
            if (score < beta) beta = score;
            if (beta <= alpha) break;
        }
        return best;
    }
}

Move getBestMove(Board& b, int depth) {
    std::vector<Move> moves = generateLegalMoves(b);

    Move bestMove = moves[0]; // fallback to first move
    int bestScore = b.whiteToMove ? -INF : INF;

    for (Move& m : moves) {
        Board nextBoard = b;
        nextBoard.makeMove(m);
        int score = alphaBeta(nextBoard, depth - 1, -INF, INF);

        if (b.whiteToMove && score > bestScore) {
            bestScore = score;
            bestMove  = m;
        }
        if (!b.whiteToMove && score < bestScore) {
            bestScore = score;
            bestMove  = m;
        }
    }

    return bestMove;
}
