#include "filters.h"
#include <algorithm>
#include <cmath>

namespace image_processor {
// черно-белый фильтр
void GrayscaleFilter::Apply(Image& image) {
    int width = image.GetWidth();
    int height = image.GetHeight();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color& pixel = image.GetPixel(x, y);
            float gray_value = 0.299f * pixel.r + 0.587f * pixel.g + 0.114f * pixel.b;
            uint8_t gray = static_cast<uint8_t>(gray_value);  // превращаем в целое число
            pixel.r = gray;
            pixel.g = gray;
            pixel.b = gray;
        }
    }
}
// фильтр негатива (инвертирует цвета)
void NegativeFilter::Apply(Image& image) {
    int width = image.GetWidth();
    int height = image.GetHeight();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color& pixel = image.GetPixel(x, y);
            pixel.r = 255 - pixel.r;
            pixel.g = 255 - pixel.g;
            pixel.b = 255 - pixel.b;
        }
    }
}
// конструктор пинимает ширину и высоту,сохраняя их в приватные поля
CropFilter::CropFilter(int width, int height) : width_(width), height_(height) {}

void CropFilter::Apply(Image& image) {
    int new_width = std::min(width_, image.GetWidth());
    int new_height = std::min(height_, image.GetHeight());
    image.Resize(new_width, new_height);
}
// фильтр повышения резкости
void SharpeningFilter::Apply(Image& image) {
    int width = image.GetWidth();
    int height = image.GetHeight();
    Image original = image;  // сохранем оригинал изображения

    int matrix[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = 0;
            int g = 0;
            int b = 0;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int px = x + kx;
                    int py = y + ky;
                    // ищем ближайший пиксель на границе
                    if (px < 0) px = 0;
                    if (px >= width) px = width - 1;
                    if (py < 0) py = 0;
                    if (py >= height) py = height - 1;
                    // берем цвет соседнего пикселя
                    const Color& pixel = original.GetPixel(px, py);
                    // вычисляем коэффициент из матрицы
                    int coeff = matrix[ky + 1][kx + 1];
                    r += coeff * pixel.r;
                    g += coeff * pixel.g;
                    b += coeff * pixel.b;
                }
            }
            // ограничение значений
            if (r < 0) r = 0;
            if (r > 255) r = 255;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            if (b < 0) b = 0;
            if (b > 255) b = 255;

            image.GetPixel(x, y) = Color(r, g, b);  // записываем новый цвет в изображение
        }
    }
}
// конструктор фильтра выделения границ,делвет изображение более четким
EdgeDetectionFilter::EdgeDetectionFilter(double threshold) : threshold_(threshold) {}
void EdgeDetectionFilter::Apply(Image& image) {
    // делаем изображение в оттенках серого
    GrayscaleFilter().Apply(image);
    // получаем размеры
    int width = image.GetWidth();
    int height = image.GetHeight();
    // сохраняем оригинал изображения
    Image original = image;
    int matrix[3][3] = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}}; //сумма всех коэф 1,поэтому япкость в целом сильно не мняется
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sum = 0;
            // применяем матрицу 3*3, ky и kx : -1,0,1 - смещения от центра
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int px = x + kx;
                    int py = y + ky;
                    // обработка краев
                    if (px < 0) px = 0;
                    if (px >= width) px = width - 1;
                    if (py < 0) py = 0;
                    if (py >= height) py = height - 1;
                    // берем значение пикселя,тк чб r=g=b
                    int pixel_value = original.GetPixel(px, py).r;
                    // берем коэффициент из матрицы
                    int coeff = matrix[ky + 1][kx + 1];
                    sum += coeff * pixel_value;
                }
            }
            // берем модуль границы
            if (sum < 0) {
                sum = -sum;
            }
            uint8_t new_color;
            // 1020 тк макс значение это 255*4=1020
            if (sum > threshold_ * 1020) {
                new_color = 255;  // белый-граница
            } else {
                new_color = 0;  // черный-фон
            }
            image.GetPixel(x, y) = Color(new_color, new_color, new_color);
        }
    }
}
// фильтр гауссово размытие, параметр - сигма
GaussianBlurFilter::GaussianBlurFilter(double sigma) : sigma_(sigma) {}
void GaussianBlurFilter::Apply(Image& image) {
    int width = image.GetWidth();
    int height = image.GetHeight();
    // определяем размер окна размытия
    int size = 3;
    if (sigma_ > 1.0) size = 5;
    if (sigma_ > 2.0) size = 7;
    // радиус окна
    int r = size / 2;
    Image copy = image;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int rr = 0, gg = 0, bb = 0;
            int count = 0;

            for (int ky = -r; ky <= r; ++ky) {
                for (int kx = -r; kx <= r; ++kx) {
                    // координаты соседнего пикселя:текущий пиксель+смещение
                    int px = x + kx;
                    int py = y + ky;

                    if (px < 0 || px >= width || py < 0 || py >= height) continue;

                    Color& p = copy.GetPixel(px, py);
                    rr += p.r;
                    gg += p.g;
                    bb += p.b;
                    count++;
                }
            }

            if (count > 0) {
                rr /= count;
                gg /= count;
                bb /= count;
            }
            // записываем новый цвет в изображение(среднее щначение всех пикселей)
            image.GetPixel(x, y) = Color(rr, gg, bb);
        }
    }
}

// фильтр извлечения канала
ExtractFilter::ExtractFilter(char channel) : channel_(channel) {}

void ExtractFilter::Apply(Image& image) {
    int width = image.GetWidth();
    int height = image.GetHeight();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color& pixel = image.GetPixel(x, y);
            
            if (channel_ == 'R') {
                pixel = Color(pixel.r, 0, 0);
            } else if (channel_ == 'G') {
                pixel = Color(0, pixel.g, 0);
            } else if (channel_ == 'B') {
                pixel = Color(0, 0, pixel.b);
            }
        }
    }
}

}  // namespace image_processor