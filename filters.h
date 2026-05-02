#pragma once
#include "image.h"

namespace image_processor {
// базовый класс для всех фильтров

class Filter {
   public:
    virtual ~Filter() = default;  // виртуальный деструктор,может быть переопределен в
                                  // наследниках,когда удаляешь объект через указатель на базовый
                                  // класс, сначала вызови деструктор наследника, потом мой
    virtual void Apply(Image& image) = 0;  // каждый наследник сам реализует этот метод
};

class GrayscaleFilter : public Filter {  // черно-белый
   public:
    void Apply(Image& image) override;  // проверяет, что сигнатура метода совпадает с базовой
};
// инвертирует цвета
class NegativeFilter : public Filter {
   public:
    void Apply(Image& image) override;
};
// обрезает изображение,оставляя левый верхний угол
class CropFilter : public Filter {
   public:
    CropFilter(int w, int h);
    void Apply(Image& image) override;

   private:       // запоминает новую ширину и высоту
    int width_;   // новая ширина
    int height_;  // новая высота
};
// делает изображение более четким
class SharpeningFilter : public Filter {
   public:
    void Apply(Image& image) override;
};
// фильтр выделения границ - на выходе чб картинка,белый- граница, черный-фон
class EdgeDetectionFilter : public Filter {
   public:
    EdgeDetectionFilter(double threshold);  // порог отсекает только сильные изменения в цвете
    void Apply(Image& image) override;

   private:
    double threshold_;  // double-тип с плавабщей точкой,private чтобы никто не мог изменитт порог
};
class GaussianBlurFilter : public Filter {
   public:
    GaussianBlurFilter(double sigma);  // сила размытия цвета
    void Apply(Image& image) override;

   private:
    double sigma_;
};
class ExtractFilter : public Filter {
public:
    ExtractFilter(char channel);
    void Apply(Image& image) override;

private:
    char channel_;
};


}  // namespace image_processor
