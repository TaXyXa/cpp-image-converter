#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <iostream>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char b_ = 'B';
    char m_ = 'M';
    int32_t size_ = 54;
    int32_t reserve_ = 0;
    uint32_t step_ = 54;

}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t info_size_ = 40;
    int32_t width_;
    int32_t height_;
    uint16_t plane_ = 1;
    uint16_t btp_ = 24;
    uint32_t compress = 0;
    uint32_t bite_number;
    int32_t horizontal_scale = 11811;
    int32_t vertical_scale = 11811;
    int32_t color_number = 0;
    int32_t value_colors = 0x1000000;

}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);

    const int w = image.GetWidth();
    const int h = image.GetHeight();
    const int s = GetBMPStride(w);

    BitmapFileHeader file_header;
    file_header.size_ += s * h;
    BitmapInfoHeader info_header;
    info_header.width_ = w;
    info_header.height_ = h;
    info_header.bite_number = s * h;

    out.write(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
    out.write(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));

    std::vector<char> buff(w * 3);

    for (int y = h-1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), s);
    }

    return out.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    assert(ifs.is_open());
    int w, h;
    // читаем заголовок: он содержит формат, размеры изображения
    // и максимальное значение цвета
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    ifs.read((char*)&file_header, sizeof(BitmapFileHeader));
    ifs.read((char*)&info_header, sizeof(BitmapInfoHeader));

    assert(file_header.b_ == 'B' && file_header.m_ == 'M' );
    assert(info_header.width_ >= 0 && info_header.height_ >= 0 );
    //std::cout << file_header << " " << info_header << std::endl;

    w = info_header.width_;
    h = info_header.height_;

    Image result(w, h, Color::Black());
    std::vector<char> buff(GetBMPStride(w));

    for (int y = h - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), GetBMPStride(w));

        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib