#pragma once

#include <unistd.h> // ::close
#include <utility>  // std::exchange
#include <experimental/scope>

class UniqueFd {
public:
  UniqueFd() noexcept = default;
  explicit UniqueFd(int fd) noexcept : fd_(fd) {}

  UniqueFd(const UniqueFd &) = delete;
  UniqueFd &operator=(const UniqueFd &) = delete;

  // Copy constructor that transfers ownership
  UniqueFd(UniqueFd &&other) noexcept : fd_(other.release()) {}

  // Copy assignment that transfers ownership
  UniqueFd &operator=(UniqueFd &&other) noexcept {
    if (this != &other) {
      reset(other.release());
    }
    return *this;
  }

  ~UniqueFd() noexcept {
    if (fd_ >= 0) {
      ::close(fd_);
    }
  }

  int get() const noexcept { return fd_; }
  explicit operator bool() const noexcept { return fd_ >= 0; }

  // returns the fd and makes the object empty without closing it.
  int release() noexcept { return std::exchange(fd_, -1); }

  // closes current if owned, then takes new_fd
  void reset(int new_fd = -1) noexcept {
    if (fd_ >= 0) {
      ::close(fd_);
    }
    fd_ = new_fd;
  }

private:
  int fd_ = -1;
};
