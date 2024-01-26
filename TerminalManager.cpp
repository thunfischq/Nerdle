// Copyright 2022, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Axel Lehmann <lehmann@cs.uni-freiburg.de>,
//         Claudius Korzen <korzen@cs.uni-freiburg.de>.
//         Johannes Kalmbach <kalmbach@cs.uni-freiburg.de>.
//
// Changes made by Henrik Roth
//
#include "./TerminalManager.h"
#include <ncurses.h>

#include <algorithm>
#include <cmath>

#define COLOR_GREY  COLOR_BLUE


// ____________________________________________________________________________
TerminalManager::TerminalManager() {
  initscr();
  start_color();
  cbreak();
  noecho();
  curs_set(false);
  nodelay(stdscr, true);
  keypad(stdscr, true);
  // Catch mouse events
  mousemask(ALL_MOUSE_EVENTS, NULL);
  mouseinterval(0);
  start_color();
  init_color(COLOR_BLUE, 300, 300, 300);
  init_pair(1, COLOR_WHITE, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_GREEN);
  init_pair(3, COLOR_WHITE, COLOR_MAGENTA);
  init_pair(4, COLOR_WHITE, COLOR_GREY);
  numRows_ = LINES;
  numCols_ = COLS / 2;
}

// ____________________________________________________________________________
TerminalManager::~TerminalManager() { endwin();}

// ____________________________________________________________________________
void TerminalManager::drawPixel(int row, int col, bool inverse, int color) {
  if (inverse) attron(A_REVERSE);
  attron(COLOR_PAIR(color));
  mvprintw(row, 2 * col, "  ");
  attroff(COLOR_PAIR(color));
  if (inverse) attroff(A_REVERSE);
}

// ____________________________________________________________________________
void TerminalManager::refresh() {
  ::refresh();
}

// ___________________________________________________________________________
void TerminalManager::drawString(int row, int col, const char* output,
                                 int color, bool bold) {
  attron(COLOR_PAIR(color));
  if (bold) { attron(A_BOLD); }
  mvaddstr(row, 2 * col, output);
  if (bold) { attroff(A_BOLD); }
  attroff(COLOR_PAIR(color));
}

// ___________________________________________________________________________
void TerminalManager::drawChar(int row, int col, const char* output,
                                              int color, bool bold) {
  attron(COLOR_PAIR(color));
  if (bold) { attron(A_BOLD); }
  mvprintw(row, 2 * col, output);
  if (bold) { attroff(A_BOLD); }
  attroff(COLOR_PAIR(color));
}

// ___________________________________________________________________________
void TerminalManager::drawBox(int row, int col, int color) {
  drawPixel(row - 1, col - 1, false, color);
  drawPixel(row - 1, col, false, color);
  drawPixel(row - 1, col + 1, false, color);
  drawPixel(row, col - 1, false, color);
  drawPixel(row, col, false, color);
  drawPixel(row, col + 1, false, color);
  drawPixel(row + 1, col - 1, false, color);
  drawPixel(row + 1, col, false, color);
  drawPixel(row + 1, col + 1, false, color);
}

// ___________________________________________________________________________
UserInput TerminalManager::getUserInput() {
  UserInput userInput;
  userInput.keycode_ = getch();
  userInput.isMouseclick_ = false;
  if (userInput.keycode_ == KEY_MOUSE) {
    MEVENT mouseEvent;
    if (getmouse(&mouseEvent) == OK) {
      if (mouseEvent.bstate & BUTTON1_PRESSED) {
        userInput.isMouseclick_ = true;
        userInput.mouseX_ = mouseEvent.x / 2;
        userInput.mouseY_ = mouseEvent.y;
      }
    }
  }
  return userInput;
}

