#pragma once

#include <cassert>
#include <cerrno>
#include <cstdio>  // for ::open
#include <fcntl.h> // for ::fcntl

#include "../src/ScopeExit.h"

static bool is_valid_fd(int fd) {
  errno = 0;
  int r = ::fcntl(fd, F_GETFD);
  if (r != -1)
    return true;
  return errno != EBADF;
}
void recordHandler(); 
void mainTest();
