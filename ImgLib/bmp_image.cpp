#include "bmp_image.h"
#include "img_lib.h"
#include "pack_defines.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <string_view>

using namespace std;

const int BYTE_IN_PIXEL = 3;
const int ALIGMENT = 4;

namespace img_lib {

    PACKED_STRUCT_BEGIN BitmapFileHeader{
        char sign[2];
        uint32_t title_size_and_data;
        uint32_t reserve_data;
        uint32_t ident;
    }
        PACKED_STRUCT_END

    PACKED_STRUCT_BEGIN BitmapInfoHeader{
        uint32_t title_size;
        int32_t width;
        int32_t height;
        uint16_t planes_count;
        uint16_t bit_per_pixel;
        uint32_t compress_type;
        uint32_t byte_count;
        int32_t h_dpi;
        int32_t v_dpi;
        int32_t color_count;
        int32_t important_count;
    }
    PACKED_STRUCT_END

    // функция вычисления отступа по ширине
    static int GetBMPStride(int w) {
        return ALIGMENT * ((w * BYTE_IN_PIXEL + BYTE_IN_PIXEL) / ALIGMENT);
    }

    // напишите эту функцию
    bool SaveBMP(const Path& file, const Image& image) {
        std::ofstream ofs(file, std::ios::binary);
        if (!ofs.is_open()) {
            return false;
        }
        int w = image.GetWidth();
        int h = image.GetHeight();
        int stride = GetBMPStride(w);
        BitmapFileHeader file_header;
        file_header.sign[0] = 'B';
        file_header.sign[1] = 'M';
        file_header.title_size_and_data = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + stride * h;
        file_header.reserve_data = 0;
        file_header.ident = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

        BitmapInfoHeader info_header;
        info_header.title_size = sizeof(BitmapInfoHeader);
        info_header.width = w;
        info_header.height = h;
        info_header.planes_count = 1;
        info_header.bit_per_pixel = 24;
        info_header.compress_type = 0;
        info_header.byte_count = stride * h;
        info_header.h_dpi = 11811;
        info_header.v_dpi = 11811;
        info_header.color_count = 0;
        info_header.important_count = 0x1000000;

        ofs.write(reinterpret_cast<char*>(&file_header), sizeof(file_header));
        ofs.write(reinterpret_cast<char*>(&info_header), sizeof(info_header));

        std::vector<char> buffer(stride, 0);
        
        for (int y = h - 1; y >= 0; --y) {
            const img_lib::Color* line = image.GetLine(y);
            for (int x = 0; x < w; ++x) {
                buffer[x * 3 + 0] = static_cast<char>(line[x].b);
                buffer[x * 3 + 1] = static_cast<char>(line[x].g);
                buffer[x * 3 + 2] = static_cast<char>(line[x].r);
            }
            ofs.write(buffer.data(), stride);
        }

        return ofs.good();
    }

    // напишите эту функцию
    Image LoadBMP(const Path& file) {
        std::ifstream ifs(file, std::ios::binary);
        if (!ifs.is_open()) {
            return {};
        }
        BitmapFileHeader file_header;
        ifs.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
        if(!ifs.good()){
            return {};
        }
        if (file_header.sign[0] != 'B' || file_header.sign[1] != 'M') {
            return {};
        }
        BitmapInfoHeader info_header;
        ifs.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
        if(!ifs.good()){
            return {};
        }

        int w = info_header.width;
        int h = info_header.height;
        int stride = GetBMPStride(w);
        img_lib::Image result(w, h, Color::Black());
        std::vector<char> buffer(stride);

        for (int y = h - 1; y >= 0; --y) {
            img_lib::Color* line = result.GetLine(y);
            ifs.read(buffer.data(), stride);
            if(!ifs.good()){
                return {};
            }

            for (int x = 0; x < w; ++x) {
                line[x].b = static_cast<std::byte>(buffer[x * 3 + 0]);
                line[x].g = static_cast<std::byte>(buffer[x * 3 + 1]);
                line[x].r = static_cast<std::byte>(buffer[x * 3 + 2]);
            }
        }

        return result;
    }

}  // namespace img_lib