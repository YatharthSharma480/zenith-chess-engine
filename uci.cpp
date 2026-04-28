#include "uci.h"
#include "movegen.h"
#include "search.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string squareToAlgebraic(int sq) {
    std::string result = "";
    result += (char)('a' + (sq % 8)); // file: a-h
    result += (char)('1' + (sq / 8)); // rank: 1-8
    return result;
}


Move algebraicToMove(const std::string& str, Board& b) {
    int from = (str[1] - '1') * 8 + (str[0] - 'a');
    int to   = (str[3] - '1') * 8 + (str[2] - 'a');

    int piece    = b.getPieceAt(from);
    int captured = b.getPieceAt(to);


    int promotedTo = EMPTY;
    if (str.length() == 5) {
        bool isWhite = (piece == WHITE_PAWN);
        switch (str[4]) {
            case 'q': promotedTo = isWhite ? WHITE_QUEEN  : BLACK_QUEEN;  break;
            case 'r': promotedTo = isWhite ? WHITE_ROOK   : BLACK_ROOK;   break;
            case 'b': promotedTo = isWhite ? WHITE_BISHOP : BLACK_BISHOP; break;
            case 'n': promotedTo = isWhite ? WHITE_KNIGHT : BLACK_KNIGHT; break;
        }
    }

    bool isCastling = false;
    if ((piece == WHITE_KING || piece == BLACK_KING) && abs(to - from) == 2) {
        isCastling = true;
    }

    Move m;
    m.from       = from;
    m.to         = to;
    m.piece      = piece;
    m.captured   = captured;
    m.promotedTo = promotedTo;
    m.isCastling = isCastling;
    return m;
}


static std::string moveToString(Move m) {
    std::string s = squareToAlgebraic(m.from) + squareToAlgebraic(m.to);
    if (m.promotedTo != EMPTY) {
        // Append promotion character
        int p = m.promotedTo;
        if (p == WHITE_QUEEN  || p == BLACK_QUEEN)  s += 'q';
        else if (p == WHITE_ROOK   || p == BLACK_ROOK)   s += 'r';
        else if (p == WHITE_BISHOP || p == BLACK_BISHOP) s += 'b';
        else if (p == WHITE_KNIGHT || p == BLACK_KNIGHT) s += 'n';
    }
    return s;
}

void uciLoop(Board& b) {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "uci") {
            std::cout << "id name SimpleEngine\n";
            std::cout << "id author You\n";
            std::cout << "uciok\n";
        }
        else if (line == "isready") {
            std::cout << "readyok\n";
        }
        else if (line == "ucinewgame") {
            b.initBoard();
        }
        else if (line.substr(0, 8) == "position") {
            // "position startpos moves e2e4 e7e5 ..."
            b.initBoard();
            size_t movesPos = line.find("moves");
            if (movesPos != std::string::npos) {
                std::istringstream ss(line.substr(movesPos + 6));
                std::string token;
                while (ss >> token) {
                    Move m = algebraicToMove(token, b);
                    b.makeMove(m);
                }
            }
        }
        else if (line.substr(0, 2) == "go") {
            int depth = 4;
            size_t depthPos = line.find("depth");
            if (depthPos != std::string::npos) {
                depth = std::stoi(line.substr(depthPos + 6));
            }

            Move best = getBestMove(b, depth);
            std::cout << "bestmove " << moveToString(best) << "\n";
        }
        else if (line == "quit") {
            break;
        }

        std::cout.flush();
    }
}
