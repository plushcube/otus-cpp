#pragma once

#include "../models/action.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

class ToolbarView {
public:
  struct Button {
    const UI_Action type;
    const std::string icon;
    const std::function<void()> action;
  };

  ToolbarView() {}

  void show() const noexcept { std::cout << "show tools panel" << std::endl; }
  void add_button(const Button &btn) { m_buttons.push_back(btn); }

  Button button_at(const size_t &idx) const { return m_buttons.at(idx); }
  size_t item_count() const noexcept { return m_buttons.size(); }

private:
  std::vector<Button> m_buttons{};
};
