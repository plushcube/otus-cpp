#include <config.h>

#include <algorithm>
#include <cstddef>
#include <sstream>

#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace std;
namespace po = boost::program_options;

template <class T> string str(const T &x) {
  stringstream e;
  e << x;
  return e.str();
}

expected<Config, string> Config::make_config(int ac, char **av) {
  vector<string> i, e, f;
  int d;
  size_t m, s;
  string h;

  // clang-format off
  po::options_description visible("Allowed options");
  visible.add_options()
    ("help", "produce help message")
    ("exclude,E", po::value<vector<string>>()->multitoken()->composing(), "exclude directories")
    ("depth,D", po::value<int>(&d)->default_value(INT_MAX), "search depth")
    ("min-size,F", po::value<size_t>(&m)->default_value(1), "minimum file size")
    ("file-masks,M", po::value<vector<string>>()->multitoken()->composing(), "file matching masks")
    ("block-size,S", po::value<size_t>(&s), "block size")
    ("hash,H", po::value<string>(&h), "hash algorithm")
  ;

  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("dirs,I", po::value<vector<string>>(), "search directories")
  ;
  // clang-format on

  po::positional_options_description p;
  p.add("dirs", -1);

  po::options_description full("Allowed options");
  full.add(visible).add(hidden);

  // clang-format off
  po::variables_map vm;
  try {
    po::store(
      po::command_line_parser(ac, av)
        .options(full)
        .positional(p)
        .run(),
      vm);
    po::notify(vm);
  } catch (const po::error &e) {
    return unexpected(string("Command line error: ") + e.what());
  }
  // clang-format on

  if (vm.count("help")) {
    return unexpected(str(visible));
  }

  if (vm.count("dirs")) {
    i = vm["dirs"].as<vector<string>>();
  } else {
    return unexpected("No input directories specified!");
  }

  if (vm.count("block-size")) {
    s = vm["block-size"].as<size_t>();
    if (s < 1) {
      return unexpected("Wrong block size specified!");
    }
  } else {
    return unexpected("No block size specified!");
  }

  if (vm.count("exclude")) {
    e = vm["exclude"].as<vector<string>>();
  }

  if (vm.count("file-masks")) {
    f = vm["file-masks"].as<vector<string>>();
  }

  if (vm.count("depth")) {
    d = vm["depth"].as<int>();
    if (d < 0) {
      d = INT_MAX;
    }
  }

  if (vm.count("min-size")) {
    m = vm["min-size"].as<size_t>();
    if (m < 1) {
      m = 1;
    }
  }

  Hash hash;
  if (!vm.count("hash")) {
    return unexpected("No hash algorithm specified");
  }
  if (h == "crc32") {
    hash = Hash::crc32;
  } else if (h == "md5") {
    hash = Hash::md5;
  } else if (h == "sha1") {
    hash = Hash::sha1;
  } else {
    return unexpected("Unknown hash function '" + h + "'!");
  }

  for (auto &mask : f) {
    transform(mask.begin(), mask.end(), mask.begin(), [](auto c) { return ::tolower(static_cast<unsigned char>(c)); });
  }

  return Config{i, e, f, static_cast<size_t>(d), s, m, hash};
}
