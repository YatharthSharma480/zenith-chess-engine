#include "movegen.h"
#include <vector>
#include <cstdlib>

#define CHECK_BIT(bb, sq) ((bb) & (1ULL << (sq)))

static Move makeM(int from, int to, int piece, int captured, int promotedTo = EMPTY, bool castle = false) {
    Move m;
    m.from       = from;
    m.to         = to;
    m.piece      = piece;
    m.captured   = captured;
    m.promotedTo = promotedTo;
    m.isCastling = castle;
    return m;
}

static bool isWhitePiece(int piece) { return piece >= WHITE_PAWN && piece <= WHITE_KING; }
static bool isBlackPiece(int piece) { return piece >= BLACK_PAWN && piece <= BLACK_KING; }

static uint64_t whitePieces(Board& b) {
    uint64_t occ = 0;
    for (int i = WHITE_PAWN; i <= WHITE_KING; i++) occ |= b.pieces[i];
    return occ;
}
static uint64_t blackPieces(Board& b) {
    uint64_t occ = 0;
    for (int i = BLACK_PAWN; i <= BLACK_KING; i++) occ |= b.pieces[i];
    return occ;
}
static uint64_t allPieces(Board& b) { return whitePieces(b) | blackPieces(b); }

static void addSlidingMoves(Board& b, int from, int piece, const int* dirs, int numDirs,
                             uint64_t friendly, uint64_t enemy, std::vector<Move>& moves) {
    for (int d = 0; d < numDirs; d++) {
        int sq = from;
        while (true) {
            int prevFile = sq % 8;
            sq += dirs[d];
            if (sq < 0 || sq > 63) break;
            int newFile = sq % 8;
            // Prevent wrapping around the board edges
            if (abs(newFile - prevFile) > 1) break;
            if (CHECK_BIT(friendly, sq)) break;
            int cap = b.getPieceAt(sq);
            moves.push_back(makeM(from, sq, piece, CHECK_BIT(enemy, sq) ? cap : EMPTY));
            if (CHECK_BIT(enemy, sq)) break;
        }
    }
}

std::vector<Move> generatePseudoLegalMoves(Board& b) {
    std::vector<Move> moves;
    uint64_t friendly = b.whiteToMove ? whitePieces(b) : blackPieces(b);
    uint64_t enemy    = b.whiteToMove ? blackPieces(b) : whitePieces(b);
    uint64_t occ      = allPieces(b);

    int myPawn   = b.whiteToMove ? WHITE_PAWN   : BLACK_PAWN;
    int myKnight = b.whiteToMove ? WHITE_KNIGHT : BLACK_KNIGHT;
    int myBishop = b.whiteToMove ? WHITE_BISHOP : BLACK_BISHOP;
    int myRook   = b.whiteToMove ? WHITE_ROOK   : BLACK_ROOK;
    int myQueen  = b.whiteToMove ? WHITE_QUEEN  : BLACK_QUEEN;
    int myKing   = b.whiteToMove ? WHITE_KING   : BLACK_KING;

    int dir    = b.whiteToMove ? 8  : -8;  // pawn push direction
    int start  = b.whiteToMove ? 1  : 6;   // starting rank index
    int promR  = b.whiteToMove ? 7  : 0;   // promotion rank index

    for (int sq = 0; sq < 64; sq++) {
        int piece = b.getPieceAt(sq);
        if (piece == EMPTY) continue;
        if (b.whiteToMove && !isWhitePiece(piece)) continue;
        if (!b.whiteToMove && !isBlackPiece(piece)) continue;

        int file = sq % 8;
        int rank = sq / 8;

        // ---- PAWNS ----
        if (piece == myPawn) {
            int one = sq + dir;
            if (one >= 0 && one < 64 && !CHECK_BIT(occ, one)) {
                if (one / 8 == promR) {
                    moves.push_back(makeM(sq, one, piece, EMPTY, myQueen));
                    moves.push_back(makeM(sq, one, piece, EMPTY, myRook));
                    moves.push_back(makeM(sq, one, piece, EMPTY, myBishop));
                    moves.push_back(makeM(sq, one, piece, EMPTY, myKnight));
                } else {
                    moves.push_back(makeM(sq, one, piece, EMPTY));
                }
                if (rank == start) {
                    int two = sq + dir * 2;
                    if (!CHECK_BIT(occ, two)) {
                        moves.push_back(makeM(sq, two, piece, EMPTY));
                    }
                }
            }
            // Captures
            int captureFiles[2] = { file - 1, file + 1 };
            for (int cf : captureFiles) {
                if (cf < 0 || cf > 7) continue;
                int capSq = (rank + (b.whiteToMove ? 1 : -1)) * 8 + cf;
                if (capSq < 0 || capSq > 63) continue;
                if (CHECK_BIT(enemy, capSq)) {
                    int cap = b.getPieceAt(capSq);
                    if (capSq / 8 == promR) {
                        moves.push_back(makeM(sq, capSq, piece, cap, myQueen));
                        moves.push_back(makeM(sq, capSq, piece, cap, myRook));
                        moves.push_back(makeM(sq, capSq, piece, cap, myBishop));
                        moves.push_back(makeM(sq, capSq, piece, cap, myKnight));
                    } else {
                        moves.push_back(makeM(sq, capSq, piece, cap));
                    }
                }
            }
        }

        else if (piece == myKnight) {
            int offsets[8] = { 17, 15, 10, 6, -6, -10, -15, -17 };
            for (int off : offsets) {
                int dest = sq + off;
                if (dest < 0 || dest > 63) continue;
                // Prevent wrap-around
                if (abs((dest % 8) - file) > 2) continue;
                if (CHECK_BIT(friendly, dest)) continue;
                int cap = CHECK_BIT(enemy, dest) ? b.getPieceAt(dest) : EMPTY;
                moves.push_back(makeM(sq, dest, piece, cap));
            }
        }

        else if (piece == myBishop) {
            int dirs[4] = { 9, 7, -7, -9 };
            addSlidingMoves(b, sq, piece, dirs, 4, friendly, enemy, moves);
        }

        else if (piece == myRook) {
            int dirs[4] = { 8, -8, 1, -1 };
            addSlidingMoves(b, sq, piece, dirs, 4, friendly, enemy, moves);
        }

        else if (piece == myQueen) {
            int dirs[8] = { 8, -8, 1, -1, 9, 7, -7, -9 };
            addSlidingMoves(b, sq, piece, dirs, 8, friendly, enemy, moves);
        }

        else if (piece == myKing) {
            int offsets[8] = { 8, -8, 1, -1, 9, 7, -7, -9 };
            for (int off : offsets) {
                int dest = sq + off;
                if (dest < 0 || dest > 63) continue;
                if (abs((dest % 8) - file) > 1) continue;
                if (CHECK_BIT(friendly, dest)) continue;
                int cap = CHECK_BIT(enemy, dest) ? b.getPieceAt(dest) : EMPTY;
                moves.push_back(makeM(sq, dest, piece, cap));
            }

            if (b.whiteToMove && sq == 4) {
                // Kingside
                if (!CHECK_BIT(occ, 5) && !CHECK_BIT(occ, 6) && CHECK_BIT(b.pieces[WHITE_ROOK], 7))
                    moves.push_back(makeM(4, 6, WHITE_KING, EMPTY, EMPTY, true));
                // Queenside
                if (!CHECK_BIT(occ, 3) && !CHECK_BIT(occ, 2) && !CHECK_BIT(occ, 1) && CHECK_BIT(b.pieces[WHITE_ROOK], 0))
                    moves.push_back(makeM(4, 2, WHITE_KING, EMPTY, EMPTY, true));
            }
            if (!b.whiteToMove && sq == 60) {
                if (!CHECK_BIT(occ, 61) && !CHECK_BIT(occ, 62) && CHECK_BIT(b.pieces[BLACK_ROOK], 63))
                    moves.push_back(makeM(60, 62, BLACK_KING, EMPTY, EMPTY, true));
                if (!CHECK_BIT(occ, 59) && !CHECK_BIT(occ, 58) && !CHECK_BIT(occ, 57) && CHECK_BIT(b.pieces[BLACK_ROOK], 56))
                    moves.push_back(makeM(60, 58, BLACK_KING, EMPTY, EMPTY, true));
            }
        }
    }

    return moves;
}

bool isSquareAttacked(Board& b, int square, bool byWhite) {

    int knightPiece = byWhite ? WHITE_KNIGHT : BLACK_KNIGHT;
    int kOffsets[8] = { 17, 15, 10, 6, -6, -10, -15, -17 };
    for (int off : kOffsets) {
        int src = square - off;
        if (src < 0 || src > 63) continue;
        if (abs((src % 8) - (square % 8)) > 2) continue;
        if (CHECK_BIT(b.pieces[knightPiece], src)) return true;
    }

    int pawnPiece = byWhite ? WHITE_PAWN : BLACK_PAWN;
    int pDir = byWhite ? -1 : 1;
    int sqFile = square % 8;
    int sqRank = square / 8;
    for (int fd : {-1, 1}) {
        int srcFile = sqFile + fd;
        int srcRank = sqRank + pDir;
        if (srcFile < 0 || srcFile > 7 || srcRank < 0 || srcRank > 7) continue;
        int src = srcRank * 8 + srcFile;
        if (CHECK_BIT(b.pieces[pawnPiece], src)) return true;
    }

    int kingPiece = byWhite ? WHITE_KING : BLACK_KING;
    int kgOffsets[8] = { 8, -8, 1, -1, 9, 7, -7, -9 };
    for (int off : kgOffsets) {
        int src = square + off;
        if (src < 0 || src > 63) continue;
        if (abs((src % 8) - sqFile) > 1) continue;
        if (CHECK_BIT(b.pieces[kingPiece], src)) return true;
    }

    uint64_t occ = allPieces(b);

    int rookPiece  = byWhite ? WHITE_ROOK  : BLACK_ROOK;
    int queenPiece = byWhite ? WHITE_QUEEN : BLACK_QUEEN;
    int straightDirs[4] = { 8, -8, 1, -1 };
    for (int d : straightDirs) {
        int sq = square;
        while (true) {
            int prevFile = sq % 8;
            sq += d;
            if (sq < 0 || sq > 63) break;
            if (abs((sq % 8) - prevFile) > 1 && (d == 1 || d == -1)) break;
            if (CHECK_BIT(occ, sq)) {
                if (CHECK_BIT(b.pieces[rookPiece], sq) || CHECK_BIT(b.pieces[queenPiece], sq)) return true;
                break;
            }
        }
    }

    int bishopPiece = byWhite ? WHITE_BISHOP : BLACK_BISHOP;
    int diagDirs[4] = { 9, 7, -7, -9 };
    for (int d : diagDirs) {
        int sq = square;
        while (true) {
            int prevFile = sq % 8;
            sq += d;
            if (sq < 0 || sq > 63) break;
            if (abs((sq % 8) - prevFile) != 1) break;
            if (CHECK_BIT(occ, sq)) {
                if (CHECK_BIT(b.pieces[bishopPiece], sq) || CHECK_BIT(b.pieces[queenPiece], sq)) return true;
                break;
            }
        }
    }

    return false;
}

std::vector<Move> generateLegalMoves(Board& b) {
    std::vector<Move> pseudo = generatePseudoLegalMoves(b);
    std::vector<Move> legal;

    int myKing = b.whiteToMove ? WHITE_KING : BLACK_KING;
    bool attackedByWhite = !b.whiteToMove;

    for (Move& m : pseudo) {
        Board nextBoard = b;
        nextBoard.makeMove(m);

        // Find where our king ended up in the new position
        int kingSq = -1;
        for (int sq = 0; sq < 64; sq++) {
            if (CHECK_BIT(nextBoard.pieces[myKing], sq)) { kingSq = sq; break; }
        }

        if (kingSq != -1 && !isSquareAttacked(nextBoard, kingSq, attackedByWhite)) {
            legal.push_back(m);
        }
    }

    return legal;
}
