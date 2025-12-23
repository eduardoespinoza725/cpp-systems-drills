#include "test_unique_fd.h"
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <vector>

void write_record(const std::vector<char> &data) {
  // Open file with write-only mode and fail if file exists
  int fd = ::open("/dev/null", O_WRONLY);
  assert(fd >= 0);
  assert(is_valid_fd(fd));

  UniqueFd f(fd);
  if (!f) {
    throw std::runtime_error("Open Failed");
  }

  auto close_file = ScopeExit{[&] { ::close(f.get()); }};

  // rollback guard for close(fd) covers all exits
  bool wrote_ok = false;
  auto rollback = ScopeExit{[&] {
    if (!wrote_ok) {
      ::unlink("/dev/null");
    }
  }};

  // write record, throw if failed
  if (::write(f.get(), data.data(), data.size()) != data.size()) {
    throw std::runtime_error("Write Failed");
  }

  // mark rollback as successful
  wrote_ok = true;
  rollback.release();
}

void read_record(std::vector<char> &data) {
  // Open file with read-only mode and fail if file exists
  int fd = ::open("/dev/null", O_RDONLY);
  assert(fd >= 0);
  assert(is_valid_fd(fd));

  UniqueFd f(fd);
  if (!f) {
    throw std::runtime_error("Open Failed");
  }

  auto close_file = ScopeExit{[&] { ::close(f.get()); }};

  // rollback guard for close(fd) covers all exits
  bool read_ok = false;
  auto rollback = ScopeExit{[&] {
    if (!read_ok) {
      ::unlink("/dev/null");
    }
  }};

  // read record, should return 0 if success, throw if failed
  if (::read(f.get(), data.data(), sizeof(data.size())) != 0) {
    throw std::runtime_error("Read Failed");
  }
  // mark rollback as successful
  read_ok = true;
  rollback.release();
}

void mainTest() {
  int fd1 = ::open("/dev/null", O_RDONLY);
  assert(fd1 >= 0);
  assert(is_valid_fd(fd1));

  UniqueFd a(fd1); // a owns fd1
  assert(a.get() == fd1);

  UniqueFd b(std::move(a)); // b now owns fd1, a is empty
  assert(!a);
  assert(b.get() == fd1);
  assert(is_valid_fd(b.get()));

  int fd2 = ::open("/dev/null", O_RDONLY);
  assert(fd2 >= 0);
  assert(is_valid_fd(fd2));

  // reset closes fd1 and takes fd2
  b.reset(fd2); // b now owns fd2, fd1 is closed
  assert(b.get() == fd2);
  assert(is_valid_fd(fd2));
  assert(!is_valid_fd(fd1)); // fd1 should be closed now

  // release detaches without closing
  int raw = b.release(); // raw owns fd2, b is empty
  assert(raw == fd2);
  assert(!b);
  assert(is_valid_fd(raw));
  ::close(raw);
  assert(!is_valid_fd(fd2));

  // test write_record
  std::vector<char> records = {'H', 'e', 'l', 'l', 'o', '\n'};
  try {
    write_record(records);
  } catch (const std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << '\n';
    throw;
  }

  // test read_record
  try {
    read_record(records);
  } catch (const std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << '\n';
    throw;
  }
}

int main() {
  mainTest();
  return 0;
}
