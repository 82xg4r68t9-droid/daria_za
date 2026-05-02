#pragma once
#include <vector>

#include "color.h"

namespace image_processor {
class Image {
   public:
    Image() = default;  // конструктор котрый ничего не делает (создание пустого изображения)
    Image(int width,
          int height);  // создание изображения заданного размера,выделит память под пиксели и
                        // заполнит их черны цветом
    Color& GetPixel(int x, int y);  // метод для доступа к пикселю,возвращает ссылку на объект Color
    const Color& GetPixel(int x, int y) const;  // константная версия метода
    int GetWidth() const { return width_; }     // возвращает ширину
    int GetHeight() const { return height_; }   // возвращает высоту
    void Resize(int new_width, int new_height);  // обрезает картину,ничго не возращает
    const Color& GetPixelClamped(
        int x,
        int y) const;  // возвращает крайний писель,если его координаты выходят за пределы
   private:
    int width_ = 0;
    int height_ = 0;
    std::vector<Color> pixels_;  // вектор пикселей, одномернвй массив
    int Getindex(int x, int y) const { return y * width_ + x; }
};
}  // namespace image_processor