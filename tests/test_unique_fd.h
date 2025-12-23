#pragma once

#include "../src/ScopeExit.h"
#include "../src/unique_fd.h"

#include <cassert>
#include <cerrno>
#include <cstdio>  // for ::open
#include <fcntl.h> // for ::fcntl
#include <stdexcept>
#include <vector>

static bool is_valid_fd(int fd) {
  errno = 0;
  int r = ::fcntl(fd, F_GETFD);
  if (r != -1)
    return true;
  return errno != EBADF;
}
void write_record(); 
void read_record();
void mainTest();
