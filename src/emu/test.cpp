#include "../logo/ecafe_logo.hpp"
#include "../logo/test_logo.hpp"
#include "../parsers/command_machine.hpp"
#include "../parsers/flash_machine.hpp"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <png.h>
#include <sstream>
#include <zoal/gfx/renderer.hpp>
#include <zoal/ic/sh1106.hpp>

zoal::misc::flash_machine fm;

uint8_t buffer[4096];
size_t count = 0;

void fm_callback(zoal::misc::flash_machine *m, const zoal::misc::flash_cmd &cmd) {
    std::cout << std::dec << "type\t: " << (int)cmd.type << std::endl;
    std::cout << "address\t: " << cmd.address << std::endl;
    std::cout << "size\t: " << cmd.size << std::endl;
    std::cout << "data\t: ";
    for (size_t i = 0; i < cmd.size; i++) {
        uint8_t byte = cmd.data[i];
        std::cout << std::hex << (int)(byte >> 4) << (int)(byte & 0xF);
        buffer[count++] = cmd.data[i];
    }
    std::cout << std::endl;
}

struct image_data {
    uint32_t address;
    const uint8_t *data;
    const size_t size;
};

using adapter = zoal::ic::sh1106_adapter_0<128, 64>;
using graphics = zoal::gfx::renderer<uint8_t, adapter>;
uint8_t canvas[1024];

void read_png() {
    FILE *fp = fopen("D:\\aaa\\a.png", "rb");
    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png == nullptr) {
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (info == nullptr) {
        png_destroy_read_struct(&png, nullptr, nullptr);
        return;
    }

    png_init_io(png, fp);
    png_read_info(png, info);
    auto width = png_get_image_width(png, info);
    auto height = png_get_image_height(png, info);
    auto color_type = png_get_color_type(png, info);
    auto bit_depth = png_get_bit_depth(png, info);

    std::cout << "width\t: " << width << std::endl;
    std::cout << "height\t: " << height << std::endl;
    std::cout << "color_type\t: " << (int)color_type << std::endl;
    std::cout << "bit_depth\t: " << (int)bit_depth << std::endl;

    auto row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    auto row_bytes = png_get_rowbytes(png, info);
    for (size_t y = 0; y < height; y++) {
        row_pointers[y] = (png_byte *)malloc(row_bytes);
    }
    png_read_image(png, row_pointers);

    auto gfx = graphics::from_memory(canvas);
    gfx->clear(0);
    for (size_t y = 0; y < height; y++) {
        png_bytep row = row_pointers[y];
        for (size_t x = 0; x < width; x++) {
            png_bytep px = &(row[x * 4]);
            if (px[0] == 0) {
                gfx->pixel(x, y, 1);
            }
        }
    }

    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    png_destroy_read_struct(&png, &info, nullptr);
}

int main(int argc, char *argv[]) {
    read_png();

    fm.callback(fm_callback);

    image_data images[] = {{1 * 4096, canvas, ecafe_logo_size}, {2 * 4096, epd_bitmap_ecafe, epd_bitmap_ecafe_size}};
    std::stringstream ss;
    auto image = images[0];
    auto address = image.address;

    ss << "erase_sector 1" << std::endl;

    for (size_t i = 0; i < image.size; i++) {
        if ((i & 0xFF) == 0) {
            ss << std::dec << std::endl << "prog_mem " << address << " ";
            address += 0x100;
        }
        uint8_t byte = image.data[i];
        ss << std::hex << (int)(byte >> 4) << (int)(byte & 0xF);
    }

    ss << std::endl << "finish" << std::endl;

    std::string str = ss.str();
    std::ofstream file;
    file.open("D:/aaa/flash.txt");
    file << str;
    file.close();

    auto cmds = str.c_str();
    for (int i = 0; cmds[i] != 0; i++) {
        fm.run_machine(cmds + i, cmds + i + 1, nullptr);
    }
    for (size_t i = 0; i < image.size; i++) {
        if (buffer[i] != image.data[i]) {
            std::cout << "Error! " << (int)buffer[i] << " !=" << (int)image.data[i] << std::endl;
        }
    }

    return 0;
}
