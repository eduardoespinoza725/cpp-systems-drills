#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "../src/unique_fd.h"
#include "test_unique_fd.h"

void Tester::recordHandler(char data[], const int n) {
  // Open file with read-only mode
  UniqueFd in(::open("../tests/start.txt", O_RDONLY));
  if (!in) {
    throw std::runtime_error("Open start.txt failed");
  }

  const char *out_path = "../tests/end.txt";

  // Open file with write-only mode and overwrite/truncate if exists
  UniqueFd out(::open(out_path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
  if (!out) {
    throw std::runtime_error("Open end.txt failed");
  }

  // rollback guard. Rollback output file unless we commit
  auto rollback = ScopeExit{[&] { ::unlink(out_path); }};

  int count = 0;
  // I/O loop: must distinguish EOF (0) from error (-1)
  for (;;) {
    const ssize_t rd = ::read(in.get(), data, 1); 
    if(rd == 0) break;
    if (rd < 0) throw std::runtime_error("Read failed");

    // to write the 1st byte of the input file in the output file bool
    bool isFirstByte = count < n;
    if (isFirstByte) {
      // SEEK_CUR specifies that offset provided is relative to current file position.
      if (::lseek(in.get(), n, SEEK_CUR) == -1) {
        throw std::runtime_error("Lseek failed");
      }
      if (::write(out.get(), data, 1) != 1) {
        throw std::runtime_error("Write failed");
      }

      count = n;
    } else {
      // After nth byte (now taking alternate nth byte).  
      count = 2*n;
      if (::lseek(in.get(), count, SEEK_CUR) == -1) {
        throw std::runtime_error("Lseek failed");
      }
      if (::write(out.get(), data, 1) != 1) {
        throw std::runtime_error("Write failed");
      }
    }
  }

  // Commit: Keep output file
  rollback.release();
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

  // DO NOT release; restoration is the point.
}

void Tester::correctOrdering() {
  auto a = ScopeExit{[] { std::cout << "a" << std::endl; }};
  auto b = ScopeExit{[] { std::cout << "b" << std::endl; }};
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
  Tester::mainTest();
  Tester::restoreTempStateTest(flg);
  Tester::correctOrdering();
  
  return 0;
}
