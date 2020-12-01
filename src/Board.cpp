#include "../header/Board.hpp"
#include "../header/Piece.hpp"
#include "../header/Coord.hpp"

#include <algorithm>
#include <array>
#include <curses.h>
#include <iostream>
#include <assert.h>

// Graphics

static bool isBlackSquare(const Coord &c) {
    return ((c.y + c.x) % 2) == 0;
}

#ifndef NO_GRAPHICS
#define NUM_SQUARES 8
#define SQUAREW 5
#define SQUAREH 3

static WINDOW* wsquares[NUM_SQUARES*NUM_SQUARES] = {nullptr};

static WINDOW* getSquare(int i, int j) {
    return wsquares[i*NUM_SQUARES + j];
}

static WINDOW* getSquare(const Coord &c) {
    return getSquare(c.y, c.x);
}
#endif

static void refreshAllSquares() {
#ifndef NO_GRAPHICS
    for (auto square: wsquares) {
        wnoutrefresh(square);
        wrefresh(square);
    }
    doupdate();
#endif
}

static void eraseBorder(const Coord &c) {
#ifndef NO_GRAPHICS
    wborder(getSquare(c), ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
#endif
}

// Don't call this function directly, call drawSquare to ensure that
// that the Piece's color at this location is not messed up
static void drawBorder(const Coord &c, bool isCursor) {
#ifndef NO_GRAPHICS
    if (isCursor) {
        WINDOW* w = getSquare(c);
        if (isBlackSquare(c))
            wbkgd(w, COLOR_PAIR(1));
        else
            wbkgd(w, COLOR_PAIR(2));
        box(w, '|', '-');
    } else {
        WINDOW* w = getSquare(c);
        if (isBlackSquare(c))
            wbkgd(w, COLOR_PAIR(3));
        else
            wbkgd(w, COLOR_PAIR(4));
        box(w, '|', '-');
    }
#endif
}

static void drawPiece(Piece *p, Coord loc) {
#ifndef NO_GRAPHICS
    WINDOW* w = getSquare(loc);
    // erase current printed symbol
    wmove(w, SQUAREH/2, 1);
    for (int i = 0; i < SQUAREW-2; ++i)
        waddch(w, ' ');
    // insert new symbol
    if (p) {
        if (p->getTeam() == WHITE) {
            if (isBlackSquare(loc)) wattron(w, COLOR_PAIR(1));
            else wattron(w, COLOR_PAIR(2));
        }
        else {
            if (isBlackSquare(loc)) wattron(w, COLOR_PAIR(3));
            else wattron(w, COLOR_PAIR(4));
        }
        mvwaddnstr(w,
                   SQUAREH/2,
                   (SQUAREW-p->getSymbol().size())/2,
                   p->getSymbol().c_str(),
                   p->getSymbol().size());
    }
#endif
}

static void drawSquare(Piece *p, const Coord &c, enum BorderType border) {
#ifndef NO_GRAPHICS
    if (border != NONE) drawBorder(c, border==CURSOR);
    //else eraseBorder(c);
    if (p) drawPiece(p, c);
#endif
}

void Board::initNCurses() {
#ifndef NO_GRAPHICS
    initscr();
    //cbreak(); // disable line buffering
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();

    assert(has_colors() == TRUE);
    assert(init_pair(1, COLOR_RED, COLOR_BLACK) != ERR); // white on black square
    assert(init_pair(2, COLOR_RED, COLOR_WHITE) != ERR); // white on white square
    assert(init_pair(3, COLOR_GREEN, COLOR_BLACK) != ERR); // black on black square
    assert(init_pair(4, COLOR_GREEN, COLOR_WHITE) != ERR); // black on white square

    refresh();
    for (int i = 0; i < sizeof(wsquares)/sizeof(wsquares[0]); ++i) {
        wsquares[i] =
            newwin(SQUAREH, SQUAREW, i/NUM_SQUARES*SQUAREH, i%NUM_SQUARES*SQUAREW);
        wbkgd(wsquares[i], COLOR_PAIR((i+i/NUM_SQUARES)%2 + 1));
    }
    drawCursor();
    refreshAllSquares();
#endif
}

static void cleanupNCurses() {
#ifndef NO_GRAPHICS
    for (auto square: wsquares)
        delwin(square);
    endwin();
#endif
}

void Board::drawTick() {
#ifndef NO_GRAPHICS
    refreshAllSquares();
#endif
}

void Board::drawSquare(const Coord &c, enum BorderType border) {
    ::drawSquare(piece(c), c, border);
}

void Board::eraseCursor() {
    eraseBorder(cursor_);
    if (std::find(highlightedSquares_.begin(),
                  highlightedSquares_.end(),
                  cursor_)
        != highlightedSquares_.end())
        // The square with the cursor is highlighted. Make sure to restore its highlighting.
        drawSquare(cursor_, HIGHLIGHTED);
}

void Board::drawCursor() {
    drawSquare(cursor_, CURSOR);
}

// Board

void Board::reset() {
    Piece **b = (Piece**)board;
    for (int i = 0; i < 8*8; ++i)
        if (b[i]) delete b[i];
    bzero(b, 8*8*sizeof(*b));
    cursor_ = Coord(0,0);
    highlightedSquares_.clear();
    cleanupNCurses();
}

// sets all pieces on the board to nullptrs.
void Board::clearBoard() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            delete piece(Coord(j, i));
            placePiece(nullptr, Coord(j, i));
        }
    }
}

void Board::moveCursor(enum Direction dir) {
    eraseCursor();
    switch(dir) {
    case UP: cursor_.addWrapped(Coord(0,-1)); break;
    case DOWN: cursor_.addWrapped(Coord(0,1)); break;
    case LEFT: cursor_.addWrapped(Coord(-1,0)); break;
    case RIGHT: cursor_.addWrapped(Coord(1,0)); break;
    }
    drawCursor();
}

Coord Board::cursor() {
    return cursor_;
}

void Board::highlightedSquares(std::vector<Coord> v) {
    for (auto c: highlightedSquares_)
        eraseBorder(c);
    highlightedSquares_ = std::move(v);
    for (auto c: highlightedSquares_)
        drawSquare(c, HIGHLIGHTED);
    // cursor may have been erased or covered up; redraw
    drawCursor();
}

const std::vector<Coord>& Board::highlightedSquares() {
    return highlightedSquares_;
}

Board& Board::get() {
    static Board board;
    return board;
}

void Board::placePiece(Piece *p, Coord loc) {
    if (p != nullptr) {
        p->updateLocation(loc.x, loc.y);
    }
    board[loc.y][loc.x] = p;
    drawPiece(p, loc);
}

Piece* Board::piece(Coord loc) {
    return board[loc.y][loc.x];
}
