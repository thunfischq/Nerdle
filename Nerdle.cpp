// Copyright 2022 Henrik Roth

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <locale>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "./Nerdle.h"
#include "./TerminalManager.h"


// ____________________________________________________________________________
Nerdle::Nerdle() {
  equation_ = generateEquation();
  for (int i = 0; i < equation_.length(); ++i) {
    symbolInEquation_[equation_[i]] = symbolInEquation_[equation_[i]] + 1;
  }
  cursor_ = 0;
  round_ = 0;
  userGuess_ = "????????";
  userGuessHighlight_ = "411111111";  // cursor at the left of the first row
  timer_ = 1;
}

// ____________________________________________________________________________
bool Nerdle::play(TerminalManager* tm) {
  if ((*tm).numRows() < 36 || (*tm).numCols() < 39) {
    (*tm).~TerminalManager();
    std::cout << "Terminal must be at least 36 rows and 76 columns of size!"
                                                                  << std::endl;
    return false;  // terminal to small to fit the game
  }
  upperLeftRow_ = ((*tm).numRows() - 36) / 2;
  upperLeftCol_ = ((*tm).numCols() - 39) / 2;
  drawBoard(tm);
  drawRow(tm);
  bool terminate = false;
  while (!terminate) {
    terminate = processUserInput(tm);
    timer_ = std::max(-1, timer_ - 1);
    if (timer_ == 0) {  // draw over message on screen
      for (int i = 2; i < 37; ++i) {
        (*tm).drawPixel(upperLeftRow_ + 33, upperLeftCol_ + i, false, 1);
        (*tm).drawPixel(upperLeftRow_ + 34, upperLeftCol_ + i, false, 4);
        (*tm).drawPixel(upperLeftRow_ + 35, upperLeftCol_ + i, false, 1);
      }
    }
    usleep(50'000);  // 50 ms
  }
  if (round_ <= 5 && userGuessHighlight_ != "22222222") {
    return false;  // game was quit via 'q'
  }
  (*tm).drawString(upperLeftRow_ + 35, upperLeftCol_ + 8,
                        "Press q to quit or ENTER to play another round.", 1);
  while (true) {
    UserInput ui = (*tm).getUserInput();
    if (ui.keycode_ == 113) {
      return false;  // quit
    } else if (ui.keycode_ == 10) {
      return true;  // play another round
    }
  }
}

// ____________________________________________________________________________
const bool Nerdle::isEquationSyntactic(const std::string* eq) const {
  if ((*eq).length() != 8) { return false; }  // wrong length
  bool symbolAllowed = false;
  bool numberAllowed = true;
  int numEq = 0;
  // Iterating over string, each iteration determines which symbols will
  // be legal in the next iteration via symbolAllowed, numberAllowed and numEq.
  for (int i = 0; i < (*eq).length(); ++i) {
    if (!std::isdigit((*eq)[i])) {
      if (!symbolAllowed) {
        // no arithmetic symbol is allowed because of prior character
        return false;
      } else if (numEq > 0) {
        // arithmetic symbol not allowed after an equal sign has appeared
        return false;
      } else if ((*eq)[i] == '=') {
        // arithmetic symbol "=" is allowed, next character must be 0-9
        ++numEq;
        symbolAllowed = false;
        numberAllowed = true;
      } else if ((*eq)[i] == '+' || (*eq)[i] == '-' || (*eq)[i] == '*'
                                                        || (*eq)[i] == '/') {
        // arithmetic symbol is allowed, next character must be 0-9
        symbolAllowed = false;
        numberAllowed = true;
      } else {
        // non-number that isn't an arithmetic symbol is not allowed
        return false;
      }
    } else {
      if (!numberAllowed) {
        return false;
      } else if ((*eq)[i] == '0') {
        // zero allowed, next symbol may be an arithmetic one
        symbolAllowed = true;
        // number after zero is only allowerd when char in front of zero
        // is also a number, else only arithmetic symbols allowed
        if (i > 0) {
          if (std::isdigit((*eq)[i - 1])) {
            numberAllowed = true;
          } else {
            numberAllowed = false;
          }
        } else {
          numberAllowed = false;
        }
      } else {
        // 1-9 allowed, 0 and arithmetic symbol allowed afterwards
        symbolAllowed = true;
        numberAllowed = true;
      }
    }
  }
  if (numEq == 1) {
    return true;
  } else {
    return false;
  }
}

// ____________________________________________________________________________
const std::pair<std::string, int>
                          Nerdle::splitEquation(const std::string* eq) const {
  int eqPos = (*eq).find('=');
  std::string leftSide = (*eq).substr(0, eqPos);
  int rightSide = std::stoi((*eq).substr(eqPos + 1, (*eq).length() - eqPos));
  return std::pair(leftSide, rightSide);
}

// ____________________________________________________________________________
const std::vector<int> Nerdle::parseEquation(const std::string* eq) const {
  std::vector<int> revEquation;
  int numCount = 0;
  int inputLength = (*eq).length();
  // each iteration, arithmetic symbols will be appended to vector as negativ
  // numbers and numbers will be eather appended (if numCount is 0) or added
  // to the last element of the vector as multiples of powers of ten.
  for (int i = 0; i < inputLength; ++i) {
    int accessValue = inputLength - i - 1;
    if (std::isdigit((*eq)[accessValue])) {
      if (numCount == 0) {
        revEquation.push_back(std::stoi((*eq).substr(accessValue, 1)));
        ++numCount;
      } else {
        revEquation.back() = revEquation.back() +
                (std::stoi((*eq).substr(accessValue, 1)) * pow(10, numCount));
        ++numCount;
      }
    } else {
      numCount = 0;
      if ((*eq)[accessValue] == '+') { revEquation.push_back(PLUS); }
      if ((*eq)[accessValue] == '-') { revEquation.push_back(MINUS); }
      if ((*eq)[accessValue] == '*') { revEquation.push_back(TIMES); }
      if ((*eq)[accessValue] == '/') { revEquation.push_back(DIVIDED); }
    }
  }
  // Equation must be inverted so that it is the correct output.
  std::vector<int> equation;
  for (int j = 0; j < revEquation.size(); ++j) {
    equation.push_back(revEquation[revEquation.size() - j - 1]);
  }
  return equation;
}

// ____________________________________________________________________________
const int Nerdle::computeEquation(const std::vector<int>* eq) const {
  std::vector<int> simpleEquation;  // equation will only contain number, +, -
  for (int i = 0; i < (*eq).size(); ++i) {
    // number or +, - appeared so no computation
    if ((*eq)[i] > -3) {
      simpleEquation.push_back((*eq)[i]);
    } else {
      if ((*eq)[i] == -3) {
        // * found
        // x * 0 or 0 * x not allowed
        if ((*eq)[i-1] == 0 || (*eq)[i+1] == 0) { return -1; }
        simpleEquation.back() = simpleEquation.back() * (*eq)[i+1];
        ++i;
      } else {
        // / found
        // x / 0 or 0 / x not allowed
        if ((*eq)[i-1] == 0 || (*eq)[i+1] == 0) { return -1; }
        float value = 1.0 * simpleEquation.back() / (*eq)[i+1];
        int roundValue = value;
        // calculations leading to non-integer results not allowed
        if (value != roundValue) { return -1; }
        simpleEquation.back() = roundValue;
        ++i;
      }
    }
  }
  int result = 0;  // result after all arithmetic operations were carried out
  for (int j = 0; j < simpleEquation.size(); ++j) {
    // number appeared so no computation
    if (simpleEquation[j] > -1) {
      result = simpleEquation[j];
    } else {
      // + found
      if (simpleEquation[j] == -1) {
        result = result + simpleEquation[j+1];
        ++j;
      } else {
        // - found
        result = result - simpleEquation[j+1];
        ++j;
      }
    }
  }
  return result;
}

// ____________________________________________________________________________
const bool Nerdle::isEquationCorrect(const std::string* eq) const {
  const std::pair<std::string, int> splitEq = splitEquation(eq);
  const std::vector<int> parsedEq = parseEquation(&((splitEq).first));
  const int computedEq = computeEquation(&parsedEq);
  return computedEq == splitEq.second;
}

// ____________________________________________________________________________
const std::string Nerdle::generateEquation() const {
  std::string equation = "";
  bool operationAppeared = false;
  bool lastIsOperation = true;
  bool penultimateIsOperation = false;
  bool lastIsZero = false;
  unsigned int currtime = (unsigned int)(time(NULL));
  int randNum;
  // Generate new random legal symbol or number each iteration. Legal meaning
  // that the generated char must be compatible with the last generated char.
  // So for example, after an * hast benn appended, the next iterations must
  // not produce an arithmetic symbol.
  while (true) {
    randNum = rand_r(&currtime);
    if (lastIsZero && penultimateIsOperation) {
      int alsoRandNum = (randNum % 4) - 4;
      if (alsoRandNum == PLUS) {
        equation += "+";
      } else if (alsoRandNum == MINUS) {
        equation += "-";
      } else if (alsoRandNum == TIMES) {
        equation += "*";
      } else {
        equation += "/";
      }
      operationAppeared = true;
      lastIsOperation = true;
      penultimateIsOperation = false;
      lastIsZero = false;
    }
    if (lastIsOperation) {
      int alsoRandNum = randNum % 10;
      equation += std::to_string(alsoRandNum);
      lastIsOperation = false;
      penultimateIsOperation = true;
      if (alsoRandNum == 0) {
        lastIsZero = true;
      } else {
        lastIsZero = false;
      }
    } else {
      int alsoRandNum = (randNum % 14) - 4;
      if (alsoRandNum >= 0) {
        equation += std::to_string(alsoRandNum);
        lastIsOperation = false;
        penultimateIsOperation = false;
        if (alsoRandNum == 0) {
          lastIsZero = true;
        } else {
          lastIsZero = false;
        }
      } else if (alsoRandNum == PLUS) {
        equation += "+";
        operationAppeared = true;
        lastIsOperation = true;
        penultimateIsOperation = false;
        lastIsZero = false;
      } else if (alsoRandNum == MINUS) {
        equation += "-";
        operationAppeared = true;
        lastIsOperation = true;
        penultimateIsOperation = false;
        lastIsZero = false;
      } else if (alsoRandNum == TIMES) {
        equation += "*";
        operationAppeared = true;
        lastIsOperation = true;
        penultimateIsOperation = false;
        lastIsZero = false;
      } else {
        equation += "/";
        operationAppeared = true;
        lastIsOperation = true;
        penultimateIsOperation = false;
        lastIsZero = false;
      }
    }
    // Check wether generated equation merged with "=" and the result
    // fits the length boundaries, if so return that equation, if it is
    // to short, add new symbols / numbers and if its to long, discard
    // the equation and beginn anew.
    if (operationAppeared && std::isdigit(equation.back())) {
      const std::vector<int> parsedEq = parseEquation(&equation);
      const int result = computeEquation(&parsedEq);
      std::string finalEquation = equation + "=" + std::to_string(result);
      if (finalEquation.length() == 8 && result >= 0
                                     && isEquationSyntactic(&finalEquation)) {
        return finalEquation;
      } else if (finalEquation.length() >= 8) {
        equation = "";
        operationAppeared = false;
        lastIsOperation = true;
      }
    }
  }
}

// ____________________________________________________________________________
const std::string Nerdle::compareUserGuess(const std::string* guess) const {
  std::unordered_map<char, int> symbolInEquationCopy = symbolInEquation_;
  std::string userGuessHighlight = "11111111";
  for (int i = 0; i < (*guess).length(); ++i) {
    if ((*guess)[i] == equation_[i]) {
      userGuessHighlight[i] = '2';  // green; symbol at right locatiom
      symbolInEquationCopy[(*guess)[i]] -= 1;
    }
  }
  for (int i = 0; i < (*guess).length(); ++i) {
    if (userGuessHighlight[i] != '2') {
      if (symbolInEquationCopy[(*guess)[i]] > 0) {
        symbolInEquationCopy[(*guess)[i]] -= 1;
        userGuessHighlight[i] = '3';  // magenta; symbol at wrong location
      } else {
        userGuessHighlight[i] = '1';  // black; wrong symbol
      }
    }
  }
  return userGuessHighlight;
}

// ____________________________________________________________________________
void Nerdle::drawRow(TerminalManager* tm) {
  std::string symbol;
  for (int i = 0; i < 8; ++i) {
    int color = userGuessHighlight_[i] - '0';
    if (userGuess_[i] != '?') {
      symbol = userGuess_.substr(i, 1);
    } else {
      symbol = " ";
    }
    (*tm).drawBox(upperLeftRow_ + 5 + 5 * round_,
                                              upperLeftCol_ + 5 + 4 *i, color);
    (*tm).drawChar(upperLeftRow_ + 5 + 5 * round_,
                              upperLeftCol_ + 5 + 4 *i, symbol.c_str(), color);
  }
  (*tm).refresh();
}

// ____________________________________________________________________________
void Nerdle::drawBoard(TerminalManager* tm) {
  for (int row = upperLeftRow_; row < upperLeftRow_ + 36; ++row) {
    for (int col = upperLeftCol_; col < upperLeftCol_ + 39; ++col) {
      if (row == upperLeftRow_ + 1 || row == upperLeftRow_ + 34
                   || col == upperLeftCol_ + 1 || col == upperLeftCol_ + 37) {
        (*tm).drawPixel(row, col, false, 4);
      }
    }
  }
  for (int row = upperLeftRow_ + 3; row < upperLeftRow_ + 33; ++row) {
    for (int col = upperLeftCol_ + 3; col < upperLeftCol_ + 36; ++col) {
      (*tm).drawPixel(row, col, true, 1);
    }
  }
  (*tm).drawString(upperLeftRow_, upperLeftCol_ + 18, "Nerdle", 1);
  (*tm).drawString(upperLeftRow_ + 1, upperLeftCol_ + 19, "by", 4);
  (*tm).drawString(upperLeftRow_ + 2, upperLeftCol_ + 18, "Henrik", 1);
  // (*tm).drawString(upperLeftRow_ + 33, upperLeftCol_ + 17,
  //                                                    equation_.c_str(), 1);
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 8; ++j) {
      (*tm).drawBox(upperLeftRow_ + 5 + 5 * i, upperLeftCol_ + 5 + 4 * j, 1);
    }
  }
  (*tm).refresh();
}

// ____________________________________________________________________________
void Nerdle::drawWinnerBoard(TerminalManager* tm) {
  for (int row = upperLeftRow_; row < upperLeftRow_ + 36; ++row) {
    for (int col = upperLeftCol_; col < upperLeftCol_ + 39; ++col) {
      if (row == upperLeftRow_ + 1 || row == upperLeftRow_ + 34
                   || col == upperLeftCol_ + 1 || col == upperLeftCol_ + 37) {
        (*tm).drawPixel(row, col, false, 2);
      }
    }
  }
  (*tm).drawString(upperLeftRow_ + 1, upperLeftCol_ + 19, "by", 2);
  (*tm).refresh();
}

// ____________________________________________________________________________
void Nerdle::drawLoserBoard(TerminalManager* tm) {
  for (int row = upperLeftRow_; row < upperLeftRow_ + 36; ++row) {
    for (int col = upperLeftCol_; col < upperLeftCol_ + 39; ++col) {
      if (row == upperLeftRow_ + 1 || row == upperLeftRow_ + 34
                   || col == upperLeftCol_ + 1 || col == upperLeftCol_ + 37) {
        (*tm).drawPixel(row, col, false, 3);
      }
    }
  }
  (*tm).drawString(upperLeftRow_ + 1, upperLeftCol_ + 19, "by", 3);
  (*tm).refresh();
}

// ____________________________________________________________________________
bool Nerdle::processUserInput(TerminalManager* tm) {
  UserInput ui = (*tm).getUserInput();
  int key;
  if (ui.keycode_ != -1) {
    key = ui.keycode_;
  } else {
    return false;
  }
  if (key == 260) {  // Left-Arrow
    userGuessHighlight_[cursor_] = 49;  // = 1 + '0'
    cursor_ = std::max(0, cursor_ - 1);
    userGuessHighlight_[cursor_] = 52;  // = 4 + '0'
    drawRow(tm);
  } else if (key == 261) {  // Right-Arrow
    userGuessHighlight_[cursor_] = 49;
    cursor_ = std::min(cursor_ + 1, 7);
    userGuessHighlight_[cursor_] = 52;
    drawRow(tm);
  } else if (47 < key && key < 58) {  // number 0-9
    userGuess_[cursor_] = key;
    userGuessHighlight_[cursor_] = 49;
    cursor_ = std::min(cursor_ + 1, 7);
    userGuessHighlight_[cursor_] = 52;
    drawRow(tm);
  } else if (key == 42) {  // arithmetic symbol *
    userGuess_[cursor_] = '*';
    userGuessHighlight_[cursor_] = 49;
    cursor_ = std::min(cursor_ + 1, 7);
    userGuessHighlight_[cursor_] = 52;
    drawRow(tm);
  } else if (key == 43) {  // arithmetic symbol +
    userGuess_[cursor_] = '+';
    userGuessHighlight_[cursor_] = 49;
    cursor_ = std::min(cursor_ + 1, 7);
    userGuessHighlight_[cursor_] = 52;
    drawRow(tm);
  } else if (key == 45) {  // arithmetic symbol -
    userGuess_[cursor_] = '-';
    userGuessHighlight_[cursor_] = 49;
    cursor_ = std::min(cursor_ + 1, 7);
    userGuessHighlight_[cursor_] = 52;
    drawRow(tm);
  } else if (key == 47) {  // arithmetic symbol /
    userGuess_[cursor_] = '/';
    userGuessHighlight_[cursor_] = 49;
    cursor_ = std::min(cursor_ + 1, 7);
    userGuessHighlight_[cursor_] = 52;
    drawRow(tm);
  } else if (key == 61) {  // arithmetic symbol =
    userGuess_[cursor_] = '=';
    userGuessHighlight_[cursor_] = 49;
    cursor_ = std::min(cursor_ + 1, 7);
    userGuessHighlight_[cursor_] = 52;
    drawRow(tm);
  } else if (key == 113) {  // q: quit
    for (int i = 2; i < 37; ++i) {
      (*tm).drawPixel(upperLeftRow_ + 34, upperLeftCol_ + i, false, 4);
    }
    (*tm).drawString(upperLeftRow_ + 34, upperLeftCol_ + 10,
                                  "Are you sure you want to quit?  [y/n]", 4);
    timer_ = 1;
    while (true) {
      UserInput userInput = (*tm).getUserInput();
      if (userInput.keycode_ == 121) {
        return true;
      } else if (userInput.keycode_ == 110) { break; }
    }
  } else if (key == 263) {  // Backspace
    if (userGuess_[cursor_] != '?') {
      userGuess_[cursor_] = '?';
    } else {
      userGuessHighlight_[cursor_] = 49;
      cursor_ = std::max(0, cursor_ - 1);
      userGuessHighlight_[cursor_] = 52;
      if (userGuess_[cursor_] != '?') { userGuess_[cursor_] = '?'; }
    }
    drawRow(tm);
  } else if (key == 10) {  // Enter
    if (isEquationSyntactic(&userGuess_)) {
      if (isEquationCorrect(&userGuess_)) {
        userGuessHighlight_ = compareUserGuess(&userGuess_);
        drawRow(tm);
        if (userGuessHighlight_ == "22222222") {  // correct equation found
          drawWinnerBoard(tm);
          (*tm).drawString(upperLeftRow_ + 34, upperLeftCol_ + 15,
                                    "Congratz! You won!", 2);
          return true;
        }
        cursor_ = 0;
        ++round_;
        if (round_ == 6) {  // game was lost
          drawLoserBoard(tm);
          (*tm).drawString(upperLeftRow_ + 34, upperLeftCol_ + 16,
                                            "Oops. You lost...", 3);
          (*tm).drawString(upperLeftRow_ + 33, upperLeftCol_ + 13,
                                              "Right equation:", 1);
          (*tm).drawString(upperLeftRow_ + 33, upperLeftCol_ + 22,
                                              equation_.c_str(), 1);
          return true;
        }
        userGuess_ = "????????";
        userGuessHighlight_ = "411111111";
        drawRow(tm);
      } else {  // equation is not correct content-wise
        for (int i = 2; i < 37; ++i) {
          (*tm).drawPixel(upperLeftRow_ + 34, upperLeftCol_ + i, false, 4);
        }
        (*tm).drawString(upperLeftRow_ + 34, upperLeftCol_ + 13,
                                    "That guess doesn't compute!", 4);
        timer_ = 100;
      }
    } else {  // equation is not syntactic
      for (int i = 2; i < 37; ++i) {
        (*tm).drawPixel(upperLeftRow_ + 34, upperLeftCol_ + i, false, 4);
      }
      (*tm).drawString(upperLeftRow_ + 34, upperLeftCol_ + 13,
                                    "That guess doesn't compute!", 4);
      timer_ = 100;
    }
  } else {  // no valid key was pressed
    for (int i = 12; i < 28; ++i) {
      (*tm).drawPixel(upperLeftRow_ + 34, upperLeftCol_ + i, false, 4);
    }
    (*tm).drawString(upperLeftRow_ + 34, upperLeftCol_ + 13,
                                    "Please press a valid key.", 4);
      timer_ = 100;
  }
  return false;
}

