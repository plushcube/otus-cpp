#pragma once

#include <cstdint>

enum class UI_Action : uint32_t {
  new_image,
  load_image,
  save_image,
  draw_rectangle,
  draw_circle,
  draw_ellipse,
  draw_line,
  remove,
  quit,
};

enum class UI_Mouse : uint8_t {
  left_btn_down,
  left_btn_up,
  right_btn_down,
  right_btn_up,
  mouse_move,
};
