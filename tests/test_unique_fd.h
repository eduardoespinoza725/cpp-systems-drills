#pragma once

#include <cassert>
#include <cerrno>
#include <cstdio>  // for ::open
#include <fcntl.h> // for ::fcntl

#include "../src/ScopeExit.h"

class Tester {
public:
  static bool is_valid_fd(int fd) {
    errno = 0;
    int r = ::fcntl(fd, F_GETFD);
    if (r != -1)
      return true;
    return errno != EBADF;
  }

  struct Flags { int value; };

  static void mainTest();
  static void recordHandler(char data[], const int n);
  static void restoreTempStateTest(Flags& flags);
  static void correctOrdering();
};
