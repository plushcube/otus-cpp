#pragma once

struct Coord {
  float x;
  float y;

  Coord operator - (const Coord &other) const noexcept {
    return { x - other.x, y - other.y };
  }

  Coord operator + (const Coord &other) const noexcept {
    return { x + other.x, y + other.y };
  }
};

struct Size {
  float width;
  float height;
};

struct Rect {
  Coord origin;
  Size size;

  Coord get_center() const noexcept { return { origin.x + size.width / 2.0f, origin.y + size.height / 2.0f }; }
  bool point_inside(const Coord &p) const noexcept {
    return p.x >= origin.x && p.x <= origin.x + size.width && p.y >= origin.y && p.y <= size.height;
  }
};
