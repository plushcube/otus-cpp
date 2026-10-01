#pragma once

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#include <vector>
#endif

namespace app {

namespace fs = std::filesystem;

namespace detail {

inline fs::path executable_path() {
#if defined(_WIN32)
  std::wstring buf(MAX_PATH, L'\0');
  for (;;) {
    const DWORD n = ::GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
    if (n == 0) {
      return {};
    }
    if (n < buf.size()) {
      buf.resize(n);
      break;
    }
    buf.resize(buf.size() * 2); // буфера не хватило — растём и пробуем снова
  }
  return fs::path{buf};
#elif defined(__APPLE__)
  uint32_t size = 0;
  ::_NSGetExecutablePath(nullptr, &size); // первый вызов — только узнаём нужный размер
  std::string buf(size, '\0');
  if (::_NSGetExecutablePath(buf.data(), &size) != 0) {
    return {};
  }
  buf.resize(std::strlen(buf.c_str()));
  return fs::path{buf};
#else
  std::vector<char> buf(1024);
  for (;;) {
    const ssize_t n = ::readlink("/proc/self/exe", buf.data(), buf.size());
    if (n < 0) {
      return {};
    }
    if (static_cast<size_t>(n) < buf.size()) {
      return fs::path{std::string{buf.data(), static_cast<size_t>(n)}};
    }
    buf.resize(buf.size() * 2);
  }
#endif
}

} // namespace detail

inline fs::path executable_dir() {
  const auto p = detail::executable_path();
  if (p.empty()) {
    return {};
  }
  std::error_code ec;
  auto abs = fs::weakly_canonical(p, ec);
  if (ec) {
    abs = fs::absolute(p, ec);
  }
  return abs.parent_path();
}

inline fs::path resource_path(const std::string &name) {
  const fs::path p{name};
  if (p.is_absolute()) {
    return p;
  }
  if (const char *home = std::getenv("FASHIO_MNIST_HOME"); home != nullptr && *home != '\0') {
    return fs::path{home} / p;
  }
  return executable_dir() / p;
}

} // namespace app
