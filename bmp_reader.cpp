#include "bmp_reader.h"

#include <fstream>     //для работы с файлами
#include <stdexcept>   //для ошибок
#pragma pack(push, 1)  // чтобы читать данные из файла правильно, без лишних байтов
// первые 14 байт
struct BMPFileHeader {
    uint16_t signature = 0x4D42;  // BMP
    uint32_t file_size;
    uint32_t reserved = 0;
    uint32_t data_offest = 54;  // где начинаются пиксели
};
// 40 байт,информация о картинке
struct BMPInfoHeader {
    uint32_t size = 40;  // размер заголовка
    int32_t width;
    int32_t height;
    uint16_t planes = 1;           // кол-во плоскостей
    uint16_t bits_per_pixel = 24;  // 24 бита на пиксель
    uint32_t compression = 0;      // сжатие
    uint32_t data_size = 0;        // размер данных пикселей
    int32_t x_ppm = 0;             // разрешение по x
    int32_t y_ppm = 0;             // разрешение по y
    uint32_t colors_used = 0;      // сколько цветов в палитре
    uint32_t colors_important;
};
#pragma pack(pop)  // возвращение к обычному выравниванию

// чтение
namespace image_processor {
Image ReadBMP(const std::string& path) {
    std::ifstream file(path,
                       std::ios::binary);  // открываем файл в бинарном режиме,информация
                                           // считывается или записывается байт в байт
    if (!file.is_open()) {
        throw std::runtime_error("невозможно открыть файл" + path);
    }
    // читаем заголовки
    BMPFileHeader file_header;
    BMPInfoHeader info_header;

    file.read(reinterpret_cast<char*>(&file_header),
              sizeof(file_header));  // чтение из файла заголовков BMP и помещение их в структуры
                                     // file_header, info_header
    file.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    if (file_header.signature != 0x4D42) {
        throw std::runtime_error("файл не является BMP!");
    }
    if (info_header.bits_per_pixel != 24) {
        throw std::runtime_error("поддерживается только 24-битный размер!");
    }
    int width = info_header.width;
    int height = std::abs(info_header.height);
    bool flipped =
        info_header.height >
        0;  //> 0 картинка хранится снизу вверх ,< 0 картинка хранится сверху вниз как в BMP
    Image image(width, height);               // создаем пустую картинку
    int padding = (4 - (width * 3) % 4) % 4;  // каждая строчка должана быть кратна 4
    // читаем пиксели
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t b, g, r;
            file.read(reinterpret_cast<char*>(&b), 1);
            file.read(reinterpret_cast<char*>(&g), 1);
            file.read(reinterpret_cast<char*>(&r), 1);
            int actual_y;
            if (flipped) {
                actual_y = height - 1 - y;  // считываем с конца если картинка перевернута

            } else {
                actual_y = y;
            }
            image.GetPixel(x, actual_y) = Color(r, g, b);
        }
        file.ignore(padding);
    }

    return image;
}
// записываем в файл
void WriteBMP(const Image& image, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("невозможно открыть файл" + path);
    }
    int width = image.GetWidth();
    int height = image.GetHeight();
    int padding = (4 - (width * 3) % 4) % 4;
    BMPFileHeader file_header;
    BMPInfoHeader info_header;
    info_header.width = width;
    info_header.height = height;
    file_header.file_size = 14 + 40 + height * (width * 3 + padding);
    // запись заголовков
    file.write(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    file.write(reinterpret_cast<char*>(&info_header), sizeof(info_header));
    // запись пикселей снизу вверх
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            Color c = image.GetPixel(x, y);
            file.put(c.b);
            file.put(c.g);
            file.put(c.r);
        }
        for (int i = 0; i < padding; ++i) {
            file.put(0);  // выравнивание
        }
    }
}

}  // namespace image_processor
