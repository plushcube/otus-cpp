// Внешний тестовый код по разделу «Проверка» задания: подключается заголовок
// <async/async.h> установленной библиотеки и линкуется с libasync (-lasync).
// Это канонический пример задания: receive() получает порции, в которых может
// быть несколько команд (разделитель '\n'), а кусок команды может рваться
// между вызовами. Ожидаемый вывод:
//
//   bulk: 1, 2, 3, 4, 5
//   bulk: 6
//   bulk: a, b, c, d
//   bulk: 89
//   bulk: 1
//
// И по файлу bulk{ts}_{postfix}.log на каждый блок (в текущей директории).
//
// Сборка против установленной библиотеки (например, после cmake --install
// с префиксом <prefix>):
//
//   c++ -std=c++23 -I<prefix>/include main.cpp -L<prefix>/lib -lasync
//       -Wl,-rpath,<prefix>/lib -o external_async

#include <async/async.h>

#include <cstddef>
#include <iostream>

int main() {
  std::size_t bulk = 5;

  auto h = connect(bulk);
  auto h2 = connect(bulk);

  receive(h, "1", 1);
  receive(h2, "1\n", 2);
  receive(h, "\n2\n3\n4\n5\n6\n{\na\n", 15);
  receive(h, "b\nc\nd\n}\n89\n", 11);

  disconnect(h);
  disconnect(h2);

  return 0;
}
