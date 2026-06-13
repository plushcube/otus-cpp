#pragma once

#include <iostream>
#include <list>
#include <tuple>
#include <vector>

namespace details {
template <typename T> struct is_container : std::false_type {};
template <typename T, typename Alloc> struct is_container<std::vector<T, Alloc>> : std::true_type {};
template <typename T, typename Alloc> struct is_container<std::list<T, Alloc>> : std::true_type {};
template <typename T> inline constexpr bool is_container_v = is_container<T>::value;

template <typename T> struct is_string : std::false_type {};
template <> struct is_string<std::string> : std::true_type {};
template <typename T> inline constexpr bool is_string_v = is_string<T>::value;

template <typename T> struct is_tuple : std::false_type {};
template <typename T> inline constexpr bool is_tuple_v = is_tuple<T>::value;
template <typename... Args> struct is_tuple<std::tuple<Args...>> : std::true_type {};
template <typename... Args>
static constexpr bool all_types_same_v = (... && std::is_same_v<Args, std::tuple_element_t<0, std::tuple<Args...>>>);
} // namespace details

template <class T, std::enable_if_t<details::is_container_v<T>, int> = 0>
std::ostream &operator<<(std::ostream &os, const T &container) {
  if (!container.empty()) {
    os << *container.begin();
    std::for_each(std::next(container.begin()), container.end(), [&os](auto &value) { os << "." << value; });
  }
  return os;
}

template <typename... Args, std::enable_if_t<details::is_tuple_v<std::tuple<Args...>>, int> = 0>
std::ostream &operator<<(std::ostream &os, std::tuple<Args...> const &value) {
  static_assert(details::all_types_same_v<Args...>, "tuple must contain elements of the same type");
  std::apply(
      [&os](Args const &...args) {
        std::size_t n{0};
        ((os << args << (++n != sizeof...(Args) ? "." : "")), ...);
      },
      value);
  return os;
}

template <class T,
          std::enable_if_t<details::is_container_v<T> || details::is_string_v<T> || details::is_tuple_v<T>, int> = 0>
void print_ip(const T &ip, std::ostream &os = std::cout) {
  os << ip << std::endl;
}

template <class T,
          std::enable_if_t<!details::is_container_v<T> && !details::is_tuple_v<T> && !details::is_string_v<T>, int> = 0>
void print_ip(const T &ip, std::ostream &os = std::cout) {
  const unsigned char *bytes = reinterpret_cast<const unsigned char *>(&ip);
  for (int i = sizeof(T) - 1; i >= 0; --i) {
    os << static_cast<int>(bytes[i]);
    if (i == 0) {
      os << std::endl;
    } else {
      os << ".";
    }
  }
}
