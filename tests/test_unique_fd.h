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

  static void recordHandler(char data[], const int n);
  static void mainTest();
};
