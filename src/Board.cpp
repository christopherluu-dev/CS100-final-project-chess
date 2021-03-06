#include "../header/Board.hpp"
#include "../header/Coord.hpp"
#include "../header/Draw.hpp"
#include "../header/Piece.hpp"

#include "../header/Pawn.hpp"
#include "../header/Rook.hpp"
#include "../header/Knight.hpp"
#include "../header/Bishop.hpp"
#include "../header/Queen.hpp"
#include "../header/King.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>

#include <strings.h>

// Graphics

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

// Iterator

Board::iterator Board::begin() { return (Piece**) board; }
Board::iterator Board::end() { return ((Piece**) board) + (8*8); }

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

void Board::placeInitialPieces() {
    King *wk = new King(WHITE, Coord(4, 7));
    King *bk = new King(BLACK, Coord(4, 0));
    vector<Piece*> pieces = {
        new Rook(WHITE, Coord(0, 7)),
        new Rook(WHITE, Coord(7, 7)),
        new Knight(WHITE, Coord(1, 7)),
        new Knight(WHITE, Coord(6, 7)),
        new Bishop(WHITE, Coord(2, 7)),
        new Bishop(WHITE, Coord(5, 7)),
        new Queen(WHITE, Coord(3, 7)),
        wk,

        new Rook(BLACK, Coord(0, 0)),
        new Rook(BLACK, Coord(7, 0)),
        new Knight(BLACK, Coord(1, 0)),
        new Knight(BLACK, Coord(6, 0)),
        new Bishop(BLACK, Coord(2, 0)),
        new Bishop(BLACK, Coord(5, 0)),
        new Queen(BLACK, Coord(3, 0)),
        bk,
    };
    for (int i = 0; i < 8; ++i) {
        pieces.push_back(new Pawn(WHITE, Coord(i, 6)));
        pieces.push_back(new Pawn(BLACK, Coord(i, 1)));
    }

    for (auto p: pieces)
        placePiece(p, p->getLocation());
    setKings(wk, bk);
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
    // validate coordinates of each coord
    for (auto c: highlightedSquares_)
        assert(c.isInBounds());

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

Piece** Board::pieceSlot(Coord c) {
    assert(c.isInBounds());
    return &(board[c.y][c.x]);
}

Piece* Board::piece(Coord c) {
    Piece *p = *pieceSlot(c);
    assert((!p) || (p->getLocation() == c));
    return p;
}

void Board::maybeRemovePiece(Coord c) {
    Piece **p = pieceSlot(c);
    if (*p) {
        delete *p;
        *p = nullptr;
    }
    drawPiece(nullptr, c);
}

void Board::placePiece(Piece *p, Coord c) {
    Piece **slot = pieceSlot(c);
    if (p != nullptr) {
        p->updateLocation(c.x, c.y);
    }
    *slot = p;
    drawPiece(p, c);
}

void Board::setKings(Piece *whiteKing, Piece *blackKing) {
    this->whiteKing = whiteKing;
    this->blackKing = blackKing;
}

static bool toplevel = true;

bool Board::isInCheck(enum Team team) {
    // hacky way to determine if possibleMoves should check for
    // isInCheck. We let it check anyways but any nested call to
    // isInCheck will just return false.
    if (toplevel && whiteKing && blackKing) {
        toplevel = false;
        Coord kingCoord = (team == WHITE) ? whiteKing->getLocation() : blackKing->getLocation();
        for (auto pslot: *this) {
            if (pslot && (pslot->getTeam() != team)) {
                vector<Coord> dangerousSquares = pslot->possibleMoves();
                if (std::find(dangerousSquares.begin(),
                              dangerousSquares.end(),
                              kingCoord)
                    != dangerousSquares.end()) {
                    toplevel = true;
                    return true;
                }
            }
        }
        toplevel = true;
    }
    return false;
}

bool Board::canMakeMove(enum Team team) {
    for (auto p: *this)
        if (p && (p->getTeam() == team) && !(p->possibleMoves().empty()))
            return true;
    return false;
}
