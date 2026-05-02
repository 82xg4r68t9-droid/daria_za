#pragma once
#include <string>

#include "image.h"
namespace image_processor {
Image ReadBMP(
    const std::string& path);  // читает и возвращает картинку,Image - тип возвращаемого значения
void WriteBMP(const Image& image,
              const std::string& path);  // ничего не возвращет,записывает картинку на диск

}  // namespace image_processor
