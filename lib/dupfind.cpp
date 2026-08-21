#include "config.h"
#include <cstring>
#include <dupfind.h>
#include <filelist.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <boost/hash2/md5.hpp>
#include <boost/hash2/sha1.hpp>

using namespace std;

DupFinder::Result DupFinder::run() {
  for (const auto &entry : FileList{m_config}) {
    const size_t count = (entry.file_size() + m_config.block - 1) / m_config.block;
    m_files.push_back({entry, vector<Hasher::Hash>(count)});
  }

  if (m_files.size() < 2) {
    return {};
  }

  DupFinder::Result result{};
  DupFinder::Buffer b(m_config.block, 0);

  for (size_t i = 0; i < m_files.size() - 1; ++i) {
    auto &f1 = m_files[i];
    if (f1.is_grouped) {
      continue;
    }

    DupFinder::Doubles pack{filesystem::absolute(f1.entry.path()).string()};

    for (size_t j = i + 1; j < m_files.size(); ++j) {
      auto &f2 = m_files[j];
      if (f2.is_grouped) {
        continue;
      }
      if (match(f1, f2, b)) {
        f2.is_grouped = true;
        f2.stream.close();
        pack.push_back(filesystem::absolute(f2.entry.path()).string());
      }
    }

    if (pack.size() > 1) {
      f1.is_grouped = true;
      f1.stream.close();
      result.push_back(pack);
    }
  }
  return result;
}

bool DupFinder::match(File &f1, File &f2, DupFinder::Buffer &buf) {
  // Дубликатами могут быть только файлы одинакового размера: проверяем
  // размер, а не число блоков. Разный размер — не пара, даже если
  // хвостовые неполные блоки при дополнении нулями дают одинаковые хеши.
  if (f1.entry.file_size() != f2.entry.file_size()) {
    return false;
  }
  for (size_t i = 0; i < f1.blocks.size(); ++i) {
    if (f1.hashed == i && !read_next_block(f1, buf)) {
      return false;
    }
    if (f2.hashed == i && !read_next_block(f2, buf)) {
      return false;
    }
    if (f1.blocks[i] != f2.blocks[i]) {
      return false;
    }
  }
  return true;
}

bool DupFinder::read_next_block(File &file, DupFinder::Buffer &buf) {
  memset(buf.data(), 0, m_config.block);
  try {
    if (!file.stream.is_open()) {
      file.stream.open(file.entry.path(), ios::binary);
      if (!file.stream.is_open()) {
        throw runtime_error("Failed to open file");
      }
    }
    file.stream.read(buf.data(), static_cast<streamsize>(m_config.block));
    // Короткое чтение до EOF — штатная ситуация (хвост дополняется нулями).
    // badbit — настоящая ошибка ввода/вывода: файл в сравнении не участвует,
    // иначе по недочитанному буферу можно было бы получить ложный дубликат.
    if (file.stream.bad()) {
      throw runtime_error("I/O error while reading file");
    }
    file.blocks[file.hashed++] = m_hasher->get_hash(buf);

  } catch (const std::exception &e) {
    cerr << "Cannot read " << file.entry.path() << ": " << e.what() << '\n';
    return false;
  }
  return true;
}
