// Copyright 2022, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Axel Lehmann <lehmann@cs.uni-freiburg.de>,
//         Claudius Korzen <korzen@cs.uni-freiburg.de>.
//         Johannes Kalmbach <kalmbach@cs.uni-freiburg.de>.
//
// Changes made by Henrik Roth
//
#ifndef TERMINALMANAGER_H_
#define TERMINALMANAGER_H_

#include <cstddef>

// Class for the input from the user.
class UserInput {
 public:
  // The code of the key pressed.
  int keycode_;
  // Was the event a mousecklick.
  bool isMouseclick_;
  // If the event was a mousecklick, then the coordinates
  // of the mouseclick are stored here.
  int mouseX_ = -1;
  int mouseY_ = -1;
};

// A class managing the input and output via the terminal, using ncurses.
class TerminalManager {
 public:
  // Constructor: initialize the terminal for use with ncurses.
  TerminalManager();

  // Destructor: Clean up the screen.
  ~TerminalManager();

  // Get input from the user.
  UserInput getUserInput();

  // Draw a "pixel" at the given position with the given color.
  // 1 = White, 2 = Green, 3 = Magenta.
  void drawPixel(int row, int col, bool inverse, int color);

  // Draw a string at the given position and with the given color.
  // 1 = White, 2 = Green, 3 = Magenta.
  void drawString(int row, int col, const char* output, int color,
                                                  bool bold = true);

  // Draw a single char at the given position
  // and with the given color. 1 = White, 2 = Green, 3 = Magenta.
  void drawChar(int row, int col, const char*, int color,
                                                bool bold = true);

  // Draw a "box" at given location with given color.
  void drawBox(int row, int col, int color);

  // Refresh the screen.
  void refresh();

  // Get the dimensions of the screen.
  int numRows() const { return numRows_; }
  int numCols() const { return numCols_; }

 private:
  // The number of "logical" rows and columns of the screen.
  int numRows_;
  int numCols_;
};

#endif  // TERMINALMANAGER_H_

