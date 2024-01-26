// Copyright 2022 Henrik Roth

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "./Nerdle.h"


TEST(NerdleTest, isEquationSyntactic) {
  Nerdle testNerdle;
  std::string test0 = "42*2=84";  // too short
  std::string test1 = "187+361=548";  // too long
  std::string test2 = "12345678";  // no equal sign
  std::string test3 = "42=42=42";  // more than one equal sign
  std::string test4 = "2**8=256";  // two arithmetic symbols next to one another
  std::string test5 = "42*02=84";  // leading zero
  std::string test6 = "126=3*42";  // arithmetic symbols right of equal-sign
  std::string test7 = "+42*1=42";  // arithmetic symbol at beginning
  std::string test8 = "42*3=126";  // all good :)
  std::string test9 = "42-10=32";  // :)))
  std::string test10 = "20*5=100";  // :))))))))
  std::string test11 = "AEIOU=42";  // illegal symbols
  std::string test12 = "42+102=E";
  std::string test13 = "5A6-9=42";
  std::string test14 = "3*6-18=0";  // single standing zero should be allowed
  std::string test15 = "0+3*4=12";  // single standing zero should be allowed
  std::string test16 = "3+0+7=10";  // single standing zero should be allowed
  std::string test17 = "102-99=3";
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test0), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test1), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test2), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test3), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test4), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test5), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test6), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test7), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test8), true);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test9), true);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test10), true);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test11), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test12), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test13), false);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test14), true);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test15), true);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test16), true);
  ASSERT_EQ(testNerdle.isEquationSyntactic(&test17), true);
}

TEST(NerdleTest, splitEquation) {
  Nerdle testNerdle;
  std::string test0 = "42-10=32";
  std::string test1 = "99+1=100";
  std::string test2 = "102-99=2";
  std::string test3 = "9*9=1024";  // almost
  std::string test4 = "31=11111";  // in binary tho
  std::string test5 = "1=101010";  // 42 is true
  ASSERT_EQ(testNerdle.splitEquation(&test0).first, std::string("42-10"));
  ASSERT_EQ(testNerdle.splitEquation(&test0).second, 32);
  ASSERT_EQ(testNerdle.splitEquation(&test1).first, std::string("99+1"));
  ASSERT_EQ(testNerdle.splitEquation(&test1).second, 100);
  ASSERT_EQ(testNerdle.splitEquation(&test2).first, std::string("102-99"));
  ASSERT_EQ(testNerdle.splitEquation(&test2).second, 2);
  ASSERT_EQ(testNerdle.splitEquation(&test3).first, std::string("9*9"));
  ASSERT_EQ(testNerdle.splitEquation(&test3).second, 1024);
  ASSERT_EQ(testNerdle.splitEquation(&test4).first, std::string("31"));
  ASSERT_EQ(testNerdle.splitEquation(&test4).second, 11111);
  ASSERT_EQ(testNerdle.splitEquation(&test5).first, std::string("1"));
  ASSERT_EQ(testNerdle.splitEquation(&test5).second, 101010);
}

TEST(NerdleTest, parseEquation) {
  Nerdle testNerdle;
  std::string test0 = "42+187-420*1337/7";
  ASSERT_EQ(testNerdle.parseEquation(&test0).size(), 9);
  ASSERT_EQ(testNerdle.parseEquation(&test0)[0], 42);
  ASSERT_EQ(testNerdle.parseEquation(&test0)[1], PLUS);
  ASSERT_EQ(testNerdle.parseEquation(&test0)[2], 187);
  ASSERT_EQ(testNerdle.parseEquation(&test0)[3], MINUS);
  ASSERT_EQ(testNerdle.parseEquation(&test0)[4], 420);
  ASSERT_EQ(testNerdle.parseEquation(&test0)[5], TIMES);
  ASSERT_EQ(testNerdle.parseEquation(&test0)[6], 1337);
  ASSERT_EQ(testNerdle.parseEquation(&test0)[7], DIVIDED);
  ASSERT_EQ(testNerdle.parseEquation(&test0)[8], 7);
}

TEST(NerdleTest, computeEquation) {
  Nerdle testNerdle;
  std::string string0 = "42*3";
  std::vector<int> test0 = testNerdle.parseEquation(&string0);
  std::string string1 = "42*3/2";  // left to right
  std::vector<int> test1 = testNerdle.parseEquation(&string1);
  std::string string2 = "0*28+4";
  std::vector<int> test2 = testNerdle.parseEquation(&string2);
  std::string string3 = "4+28*0";
  std::vector<int> test3 = testNerdle.parseEquation(&string3);
  std::string string4 = "0/28-5";
  std::vector<int> test4 = testNerdle.parseEquation(&string4);
  std::string string5 = "5-28/0";
  std::vector<int> test5 = testNerdle.parseEquation(&string5);
  std::string string6 = "42+5/9";
  std::vector<int> test6 = testNerdle.parseEquation(&string6);
  std::string string7 = "42+5";
  std::vector<int> test7 = testNerdle.parseEquation(&string7);
  std::string string8 = "9-16";
  std::vector<int> test8 = testNerdle.parseEquation(&string8);
  std::string string9 = "42+2*21";  // *|/ prior to +|-
  std::vector<int> test9 = testNerdle.parseEquation(&string9);
  std::string string10 = "187-42*3+42/6+1";
  std::vector<int> test10 = testNerdle.parseEquation(&string10);
  ASSERT_EQ(testNerdle.computeEquation(&test0), 126);
  ASSERT_EQ(testNerdle.computeEquation(&test1), 63);
  ASSERT_EQ(testNerdle.computeEquation(&test2), -1);
  ASSERT_EQ(testNerdle.computeEquation(&test3), -1);
  ASSERT_EQ(testNerdle.computeEquation(&test4), -1);
  ASSERT_EQ(testNerdle.computeEquation(&test5), -1);
  ASSERT_EQ(testNerdle.computeEquation(&test6), -1);
  ASSERT_EQ(testNerdle.computeEquation(&test7), 47);
  ASSERT_EQ(testNerdle.computeEquation(&test8), -7);
  ASSERT_EQ(testNerdle.computeEquation(&test9), 84);
  ASSERT_EQ(testNerdle.computeEquation(&test10), 69);
}

TEST(NerdleTest, isEquationCorrect) {
  Nerdle testNerdle;
  std::string test0 = "32+10=42";
  std::string test1 = "42-10=32";
  std::string test2 = "20*6=120";
  std::string test3 = "120/6=20";
  std::string test4 = "5*9-3=42";
  std::string test5 = "6/1*7=42";
  std::string test6 = "42*3=420";
  std::string test7 = "420-42=9";
  std::string test8 = "0*1337=0";
  std::string test9 = "187/9=20";
  std::string test10 = "187/9=21";
  ASSERT_EQ(testNerdle.isEquationCorrect(&test0), true);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test1), true);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test2), true);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test3), true);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test4), true);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test5), true);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test6), false);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test7), false);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test8), false);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test9), false);
  ASSERT_EQ(testNerdle.isEquationCorrect(&test10), false);
}

TEST(NerdleTest, generateEquation) {
  for (int i = 0; i < 1000; ++i) {
    Nerdle testNerdle;
    ASSERT_EQ(testNerdle.isEquationSyntactic(&testNerdle.equation_), true);
    ASSERT_EQ(testNerdle.isEquationCorrect(&testNerdle.equation_), true);
  }
}

TEST(NerdleTest, compareUserGuess) {
  Nerdle testNerdle;
  std::string test0 = testNerdle.equation_;
  ASSERT_EQ(testNerdle.compareUserGuess(&test0), "22222222");
  for (int i = 0; i < 8; ++i) {
    std::string test1 = test0;
    test1[i] = 'A';
    std::string equal = "22222222";
    equal[i] = '1';
    ASSERT_EQ(testNerdle.compareUserGuess(&test1), equal);
  }
  for (int i = 0; i < 8; ++i) {
    std::string test2 = "AAAAAAAA";
    test2[i] = test0[i];
    std::string equal = "11111111";
    equal[i] = '2';
    ASSERT_EQ(testNerdle.compareUserGuess(&test2), equal);
  }
  int tmp = test0.find('=');
  for (int i = 0; i < 8; ++i) {
    std::string test3 = test0;
    if (i != tmp) {
      test3[i] = test0[tmp];
      test3[tmp] = test0[i];
      std::string equal = "22222222";
      equal[tmp] = '3';
      equal[i] = '3';
      ASSERT_EQ(testNerdle.compareUserGuess(&test3), equal);
    }
  }
}

