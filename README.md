# Zenith Chess Engine

A from-scratch chess engine written in **C++17**, built around a pure bitboard architecture. Zenith communicates with chess GUIs using the **UCI protocol** and currently searches at a fixed depth using alpha-beta pruning.

---

## Architecture Overview

### Bitboard Representation (`types.h`, `board.h`, `board.cpp`)

The board is stored as an array of **12 `uint64_t` bitboards** — one per piece type. Each bit in a `uint64_t` corresponds to one of the 64 squares; a set bit means that piece occupies that square.

```
pieces[0]  = WHITE_PAWN   bitboard
pieces[1]  = WHITE_KNIGHT bitboard
...
pieces[11] = BLACK_KING   bitboard
```

Piece placement is manipulated with three bitwise macros:

| Macro | Operation |
|---|---|
| `SET_BIT(bb, sq)` | Place a piece on a square (`\|=`) |
| `CLEAR_BIT(bb, sq)` | Remove a piece from a square (`&= ~`) |
| `CHECK_BIT(bb, sq)` | Test whether a square is occupied (`&`) |

**`Board::makeMove`** executes a move in-place. It handles:
- Standard quiet moves and captures
- Pawn promotion (replaces the pawn bit with the promoted piece bit)
- Castling (moves both king and rook atomically)

---

### Move Generation (`movegen.h`, `movegen.cpp`)

Move generation is split into two stages:

#### 1. Pseudo-legal generation (`generatePseudoLegalMoves`)

Iterates every square, identifies the piece, and appends candidate `Move` structs to a `std::vector<Move>`. Each piece type uses a dedicated strategy:

| Piece | Strategy |
|---|---|
| Pawns | Direction-based push (+8 / -8), double push from starting rank, diagonal captures, promotion branches |
| Knights | 8-offset table with file-delta wrap guard (`abs(Δfile) > 2`) |
| Bishops | Diagonal ray sliding (`±9, ±7`) via `addSlidingMoves` |
| Rooks | Orthogonal ray sliding (`±8, ±1`) via `addSlidingMoves` |
| Queens | All 8 directions via `addSlidingMoves` |
| Kings | 8-offset step moves + castling availability check |

`addSlidingMoves` is a shared ray-casting helper that walks a direction until it hits the board edge, a friendly piece (blocked), or an enemy piece (captured, then stopped).

#### 2. Legality filter (`generateLegalMoves`)

For each pseudo-legal move, a **copy of the board** is made, the move is applied, and `isSquareAttacked` checks whether our king is in check in the resulting position. Moves that leave the king in check are discarded.

#### Attack Detection (`isSquareAttacked`)

Reverse-traces attacks from the target square outward using each piece's movement pattern:
- Knights: 8-offset reverse lookup
- Pawns: reverse diagonal from the target square
- Kings: 8-step adjacency check
- Rooks/Queens: orthogonal ray scan
- Bishops/Queens: diagonal ray scan

---

### Evaluation (`eval.h`, `eval.cpp`)

Material-only evaluation using standard centipawn values:

| Piece | Value |
|---|---|
| Pawn | ±100 |
| Knight | ±320 |
| Bishop | ±330 |
| Rook | ±500 |
| Queen | ±900 |
| King | ±20000 |

White pieces are positive, black pieces are negative. The score is computed by iterating all 12 bitboards and summing `popcount(bb) × pieceValue` using **Brian Kernighan's bit-trick** (`bb &= bb - 1`) for efficient population counting.

---

### Search (`search.h`, `search.cpp`)

**Alpha-Beta Minimax** with negation-based pruning:

```
alphaBeta(board, depth, alpha, beta)
  if depth == 0 → return evaluateBoard()
  for each legal move:
    make move on a board copy
    score = alphaBeta(copy, depth-1, alpha, beta)
    prune if beta ≤ alpha
```

- White is the **maximising** player, black is the **minimising** player.
- Checkmate is detected when `generateLegalMoves` returns an empty list and the king is in check (returns `±INF`).
- Stalemate returns `0`.
- `getBestMove` is the root call; it runs alpha-beta at the top level and tracks the best `Move` object to return to the UCI layer.

---

### UCI Protocol (`uci.h`, `uci.cpp`)

Implements a minimal UCI loop compatible with any standard chess GUI (e.g. Cute Chess, Arena, Lichess bot):

| Command | Behaviour |
|---|---|
| `uci` | Identifies the engine |
| `isready` | Responds `readyok` |
| `ucinewgame` | Resets the board to the start position |
| `position startpos moves …` | Replays the move list from the start position |
| `go depth N` | Searches to depth N and outputs `bestmove` |
| `quit` | Exits the loop |

Move strings are in **long algebraic notation** (`e2e4`, `e7e8q`). `algebraicToMove` parses the string into a `Move` struct by looking up the piece on the board and detecting castling via a king moving ±2 files.

---

## Build

Requires **g++** with C++17 support (MinGW on Windows, g++ on Linux/macOS).

```bash
cd chess_engine
make
```

This produces `engine.exe` (Windows) / `engine` (Linux/macOS).

To clean build artifacts:
```bash
make clean
```

---

## Project Structure

```
chess_engine/
├── chess_engine/          ← source code
│   ├── types.h            ← Bitboard typedef, piece constants, Move struct
│   ├── board.h / .cpp     ← Board state, initBoard, makeMove
│   ├── movegen.h / .cpp   ← Pseudo-legal & legal move generation, attack detection
│   ├── eval.h / .cpp      ← Material evaluation
│   ├── search.h / .cpp    ← Alpha-beta search, getBestMove
│   ├── uci.h / .cpp       ← UCI protocol loop
│   ├── main.cpp           ← Entry point
│   ├── Makefile
│   └── zenith_test.exe    ← Current test build
├── engine.exe
├── zenith_new.exe
└── zenith_v2.exe
```
