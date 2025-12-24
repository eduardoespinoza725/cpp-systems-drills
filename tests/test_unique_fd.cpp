#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "../src/unique_fd.h"
#include "test_unique_fd.h"

void Tester::recordHandler(char data[], const int n) {
  // Open file with read-only mode and fail if file exists
  int write_fd = open("../tests/start.txt", O_RDONLY);
  assert(write_fd >= 0);
  assert(Tester::is_valid_fd(write_fd));

  // Open file with write-only mode and fail if file exists
  int read_fd = open("../tests/end.txt", O_WRONLY);
  assert(read_fd >= 0);
  assert(Tester::is_valid_fd(read_fd));

  UniqueFd f(write_fd);
  if (!f) {
    throw std::runtime_error("Open Failed");
  }

  UniqueFd r(read_fd);
  if (!r) {
    throw std::runtime_error("Open Failed");
  }

  // rollback guard for close(fd) covers all exits
  bool wrote_ok = false;
  auto write_rollback = ScopeExit{[&] {
    if (!wrote_ok) {
      ::unlink("../tests/start.txt");
    }
  }};

  bool read_ok = false;
  auto read_rollback = ScopeExit{[&] {
    if (!read_ok) {
      ::unlink("../tests/end.txt");
    }
  }};

  // write record, throw if failed
  int count = 0;
  while (::read(f.get(), data, 1) != 0) {
    // to write the 1st byte of the input file in the output file
    bool isFirstByte = count < n;
    if (isFirstByte) {
      // SEEK_CUR specifies that offset provided is relative to current file position.
      ::lseek(f.get(), n, SEEK_CUR);
      if (::write(r.get(), data, 1) != 1) {
        throw std::runtime_error("Write1 Failed");
      }

      count = n;
    }

    // After nth byte (now taking alternate nth byte).
    else {
      count = 2*n;
      lseek(f.get(), count, SEEK_CUR);
      if (::write(r.get(), data, 1) != 1) {
        throw std::runtime_error("Write2 Failed");
      }
    }
  }

  // mark rollback as successful
  wrote_ok = true;
  write_rollback.release();

  // mark rollback as successful
  read_ok = true;
  read_rollback.release();
}

void Tester::restoreTempStateTest(Flags &flags) {
  int old = flags.value;
  auto restore = ScopeExit{[&] { flags.value = old; }};

  flags.value |= 0x4; // temporarily enable something
  // create operations that may throw or early-return
  char arr[100];
  int n;
  n = 5;

  // Calling for the function
  recordHandler(arr, n);
}

void Tester::correctOrdering() {
  auto a = ScopeExit{[] { std::cout << "a"; }};
  auto b = ScopeExit{[] { std::cout << "b"; }};
}

void Tester::mainTest() {
  int fd1 = ::open("/dev/null", O_RDONLY);
  assert(fd1 >= 0);
  assert(Tester::is_valid_fd(fd1));

  UniqueFd a(fd1); // a owns fd1
  assert(a.get() == fd1);

  UniqueFd b(std::move(a)); // b now owns fd1, a is empty
  assert(!a);
  assert(b.get() == fd1);
  assert(Tester::is_valid_fd(b.get()));

  int fd2 = ::open("/dev/null", O_RDONLY);
  assert(fd2 >= 0);
  assert(Tester::is_valid_fd(fd2));

  // reset closes fd1 and takes fd2
  b.reset(fd2); // b now owns fd2, fd1 is closed
  assert(b.get() == fd2);
  assert(Tester::is_valid_fd(fd2));
  assert(!Tester::is_valid_fd(fd1)); // fd1 should be closed now

  // release detaches without closing
  int raw = b.release(); // raw owns fd2, b is empty
  assert(raw == fd2);
  assert(!b);
  assert(Tester::is_valid_fd(raw));
  ::close(raw);
  assert(!Tester::is_valid_fd(fd2));

  // std::vector<char> arr(100);
  char arr[100];
  int n;
  n = 5;

  // Calling for the function
  recordHandler(arr, n);
}

int main() {
  Tester::Flags flg{.value = 1};
  try {
    Tester::mainTest();
    Tester::restoreTempStateTest(flg);
    Tester::correctOrdering();
  } catch (...) {
    throw std::runtime_error("Failed");
  }
  return 0;
}
