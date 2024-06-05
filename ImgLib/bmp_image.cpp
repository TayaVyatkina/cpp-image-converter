#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {
    
    PACKED_STRUCT_BEGIN BitmapFileHeader{
    uint8_t sig[2]{'B', 'M'};
    uint32_t header_and_data_size;
    uint32_t reserved = 0;
    uint32_t padding = 54;
    }
    PACKED_STRUCT_END

    PACKED_STRUCT_BEGIN BitmapInfoHeader{
        uint32_t header_size = 40;
        uint32_t width;
        uint32_t height;
        uint16_t plate = 1;
        uint16_t bit_per_pixel = 24;
        uint32_t compress_type = 0;
        uint32_t data_byte_count;
        uint32_t hor_per = 11811;
        uint32_t vert_per = 11811;
        uint32_t color_counter = 0;
        uint32_t main_color = 0x1000000;
    }
    PACKED_STRUCT_END

        // функция вычисления отступа по ширине
    static int GetBMPStride(int w) {
        return 4 * ((w * 3 + 3) / 4);
    }

    bool SaveBMP(const Path& file, const Image& image) {
        // некорректное изображение
        if (!image.operator bool()) {
            return false;
        }
        ofstream out(file, ios::out | ios::binary);

        BitmapFileHeader file_header;
        BitmapInfoHeader info_header;

        file_header.header_and_data_size = sizeof(file_header) + sizeof(info_header) + GetBMPStride(image.GetWidth()) * image.GetHeight();

        info_header.width = image.GetWidth();
        info_header.height = image.GetHeight();
        info_header.data_byte_count = GetBMPStride(image.GetWidth()) * image.GetHeight();

        out.write(reinterpret_cast<const char*>(&file_header), sizeof(BitmapFileHeader));
        out.write(reinterpret_cast<const char*>(&info_header), sizeof(BitmapInfoHeader));

        std::vector<char> buffer(image.GetWidth() * 3);

        for (int y = image.GetHeight() - 1; y >= 0; --y) {
            const Color* line = image.GetLine(y);

            for (int x = 0; x < image.GetWidth(); ++x) {

                buffer[x * 3 + 0] = (static_cast<char>(line[x].b));
                buffer[x * 3 + 1] = (static_cast<char>(line[x].g));
                buffer[x * 3 + 2] = (static_cast<char>(line[x].r));
            }
            out.write(buffer.data(), GetBMPStride(image.GetWidth()));

            if (out.fail()) {
                return false;
            }
        }

        return true;
    }

    Image LoadBMP(const Path& file) {
        ifstream ifs(file, ios::binary);

        BitmapFileHeader file_header;
        BitmapInfoHeader info_header;
        ifs.read(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
        ifs.read(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));

        Image result(info_header.width, info_header.height, Color::Black());
        std::vector<char> buff(info_header.width * 3);

        for (int y = info_header.height - 1; y >= 0; --y) {
            Color* line = result.GetLine(y);
            ifs.read(buff.data(), GetBMPStride(info_header.width));

            for (int x = 0; x < info_header.width; ++x) {
                line[x].b = static_cast<byte>(buff[x * 3 + 0]);
                line[x].g = static_cast<byte>(buff[x * 3 + 1]);
                line[x].r = static_cast<byte>(buff[x * 3 + 2]);
            }
        }

        return result;
    }
}  // namespace img_lib
   