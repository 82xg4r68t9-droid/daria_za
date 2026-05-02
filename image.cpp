#include "image.h"

#include <algorithm>

namespace image_processor {
Image::Image(int width, int height)
    : width_(width), height_(height) {  // функция Image внутри класса
                                        // Image(конструктор)
    pixels_.resize(width * height);     // выделяем место в памяти под новые пиксели
}
Color& Image::GetPixel(int x, int y) {
    return pixels_[Getindex(
        x, y)];  // возвращаем ссылку на цвет,а не копию,можем менять,возвращает пиксель из массива
}
const Color& Image::GetPixel(int x, int y) const {
    return pixels_[Getindex(x, y)];  // константная версия метода
}
const Color& Image::GetPixelClamped(int x, int y) const {  // ограничение пикселя по границам
    x = std::clamp(x, 0, width_ - 1);
    y = std::clamp(y, 0, height_ - 1);
    return GetPixel(x, y);
}
void Image::Resize(int new_width, int new_height) {
    Image new_image(new_width, new_height);  // создаем новую пустую картинку

    int copy_w = std::min(width_, new_width);  // сколько копировать
    int copy_h = std::min(height_, new_height);

    for (int y = 0; y < copy_h; ++y) {                  // строка
        for (int x = 0; x < copy_w; ++x) {              // столбец
            new_image.GetPixel(x, y) = GetPixel(x, y);  // копируем пиксель
        }
    }
    *this = new_image;  // заменяем старую картинку на новую
}
}  // namespace image_processor
