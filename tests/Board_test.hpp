#include "gtest/gtest.h"

#include "../header/Coord.hpp"
#include "../header/Board.hpp"
#include "../header/Pawn.hpp"

TEST(BoardTest, Cursor) {
    Board &b = Board::get();
    Coord expected = b.cursor();
    expected.addWrapped(Coord(0,-1));
    b.moveCursor(UP);
    EXPECT_EQ(expected, b.cursor());
    expected = b.cursor();
    expected.addWrapped(Coord(0,1));
    b.moveCursor(DOWN);
    EXPECT_EQ(expected, b.cursor());
    expected = b.cursor();
    expected.addWrapped(Coord(-1,0));
    b.moveCursor(LEFT);
    EXPECT_EQ(expected, b.cursor());
    expected = b.cursor();
    expected.addWrapped(Coord(1,0));
    b.moveCursor(RIGHT);
    EXPECT_EQ(expected, b.cursor());
}

TEST(BoardTest, HighlightedSquares) {
    Board &b = Board::get();
    Coord a (0,1);
    std::vector<Coord> v = {a, Coord(5,6)};
    b.highlightedSquares(v);
    for (int i = 0; i < v.size(); ++i)
        EXPECT_EQ(b.highlightedSquares()[i], v[i]);

    // test that Board creates its own vector
    v[0] = Coord(7,7);
    EXPECT_EQ(b.highlightedSquares()[0], a);
}

TEST(BoardTest, Pieces) {
    Board &b = Board::get();
    Coord a (0,1);
    Pawn p;
    std::vector<Coord> v;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            v.push_back(Coord(i,j));

    // board should start empty
    for (auto c: v)
        EXPECT_EQ(b.piece(c), nullptr);
    Coord c = v.back();
    v.pop_back();
    // place a piece
    b.placePiece(&p, c);
    EXPECT_EQ(b.piece(c), &p);
    for (auto c: v)
        EXPECT_EQ(b.piece(c), nullptr);
}