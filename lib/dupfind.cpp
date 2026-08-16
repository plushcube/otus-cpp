#include <dupfind.h>

#include <filelist.h>

#include <iostream>

void DupFinder::run() {
  for (const auto &entry : FileList{m_config}) {
    // TODO: блочное чтение (m_config.block), хеширование (m_config.hash),
    // группировка дубликатов и вывод результата.
    std::cout << "file: " << entry << std::endl;
  }
}
