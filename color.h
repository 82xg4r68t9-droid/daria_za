#pragma once
#include <algorithm>  //для ограничения чисел (std::clamp)
#include <cstdint>

namespace image_processor {
struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    Color() : r(0), g(0), b(0) {}  // конструктор по умолчанию
    Color(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}
    Color operator*(float value) const {  // не изменяет текущий обьект
        return Color(static_cast<uint8_t>(std::clamp(
                         r * value, 0.0f, 255.0f)),  // если <- - осталяем 0,если >255- берем 255
                     static_cast<uint8_t>(std::clamp(g * value, 0.0f, 255.0f)),
                     static_cast<uint8_t>(std::clamp(b * value, 0.0f, 255.0f)));
    }
    Color operator+(const Color& other)
        const {  // только смотрим на обьект other не меняя его,только прочитав его компоненты
        return Color(static_cast<uint8_t>(std::clamp(r + other.r, 0, 255)),
                     static_cast<uint8_t>(std::clamp(g + other.g, 0, 255)),
                     static_cast<uint8_t>(std::clamp(b + other.b, 0, 255)));
    }
};

}  // namespace image_processor