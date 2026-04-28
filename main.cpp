#include "board.h"
#include "uci.h"

int main() {
    Board b;
    b.initBoard();
    uciLoop(b);
    return 0;
}
