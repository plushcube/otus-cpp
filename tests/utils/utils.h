#pragma once

#include <config.h>
#include <filelist.h>

#include <filesystem>
#include <set>
#include <string>
#include <vector>

namespace test_utils {

// Конфигурация по умолчанию: без масок, без ограничения глубины,
// блок 1024 байта, минимальный размер 1 байт, хэш md5.
Config make_config(std::vector<std::string> dirs, std::vector<std::string> exclude = {});

// Все файлы, найденные FileList, полными путями.
std::vector<std::filesystem::path> collect_files(const Config &cfg);

// Имена файлов (filename()), найденных FileList.
std::set<std::string> file_names(const Config &cfg);

// Независимый от FileList рекурсивный сбор регулярных файлов
// размером >= min_size — для проверки поведения утилиты «со стороны».
std::vector<std::filesystem::path> collect_regular_files(const std::filesystem::path &root,
                                                         size_t min_size = 1);

// Корень статичной фикстуры tests/fixtures (см. fixtures/tree).
std::filesystem::path fixture_dir(const std::string &name = "tree");

// Временная директория с уникальным именем; удаляется в деструкторе.
class TempDir {
public:
  TempDir();
  ~TempDir();

  TempDir(const TempDir &) = delete;
  TempDir &operator=(const TempDir &) = delete;

  const std::filesystem::path &path() const { return m_path; }

private:
  std::filesystem::path m_path;
};

} // namespace test_utils
