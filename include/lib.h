#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

typedef std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> ip4_t;

std::vector<ip4_t> filter(const std::vector<ip4_t> &ip_pool, std::function<bool(const ip4_t &)> pred);
ip4_t read_ip(const std::string &line, const char delimeter = '\t');
void sort_rev(std::vector<ip4_t> &ip_pool);
