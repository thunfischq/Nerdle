// Copyright 2022 Henrik Roth

#include "./TerminalManager.h"
#include "./Nerdle.h"


int main() {
  TerminalManager tm;
  bool run = true;
  while (run) {
    Nerdle nerdle;
    run = nerdle.play(&tm);
  }
}

