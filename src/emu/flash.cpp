#include "../parsers/flash_machine.hpp"

#include <boost/endian.hpp>
#include <boost/filesystem.hpp>
#include <boost/json.hpp>
#include <fstream>
#include <gif_lib.h>
#include <iostream>
#include <png.h>
#include <zoal/gfx/renderer.hpp>
#include <zoal/ic/sh1106.hpp>

zoal::misc::flash_machine fm;

uint8_t buffer[4096];
size_t count = 0;

using adapter = zoal::ic::sh1106_adapter_0<128, 64>;
using graphics = zoal::gfx::renderer<uint8_t, adapter>;
uint8_t canvas[1024];

void read_png(const char *file, void *b) {
    FILE *fp = fopen(file, "rb");
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

    auto gfx = graphics::from_memory(b);
    gfx->clear(0);
    for (size_t y = 0; y < height; y++) {
        png_bytep row = row_pointers[y];
        for (size_t x = 0; x < width; x++) {
            png_bytep px = &(row[x * 4]);
            if (px[0] == 0) {
                gfx->pixel((int)x, (int)y, 1);
            }
        }
    }

    for (size_t y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);
    png_destroy_read_struct(&png, &info, nullptr);
}

static std::string file_content(const boost::filesystem::path &path) {
    if (!boost::filesystem::is_regular_file(path)) {
        return {};
    }

    std::ifstream file(path.string(), std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    auto content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return content;
}

struct flash_animation {

    uint32_t width;
    uint32_t height;
};

struct flash_image {
    uint32_t width;
    uint32_t height;
};

struct flash_record {
    uint32_t tag;
    uint32_t offset;
    uint32_t size;
};

std::vector<flash_record> records;
std::vector<uint8_t> data;

void write_flash(std::ostream &ss, uint32_t data_address, const uint8_t *bytes, size_t data_size) {
    auto address = data_address;
    for (size_t i = 0; i < data_size; i++) {
        if ((i & 0xFF) == 0) {
            ss << std::dec << std::endl << "prog_mem " << address << " ";
            address += 0x100;
        }
        uint8_t byte = bytes[i];
        ss << std::hex << (int)(byte >> 4) << (int)(byte & 0xF);
    }
}

bool gif_read(const char *fileName) {
    int error;
    GifFileType *gifFile = DGifOpenFileName(fileName, &error);
    if (!gifFile) {
        std::cout << "DGifOpenFileName() failed - " << error << std::endl;
        return false;
    }
    if (DGifSlurp(gifFile) == GIF_ERROR) {
        std::cout << "DGifSlurp() failed - " << gifFile->Error << std::endl;
        DGifCloseFile(gifFile, &error);
        return false;
    }

    ColorMapObject *commonMap = gifFile->SColorMap;
    std::cout << fileName << ": " << gifFile->SWidth << "x" << gifFile->SHeight << std::endl;

    for (int i = 0; i < gifFile->ImageCount; ++i) {
        const SavedImage &saved = gifFile->SavedImages[i];
        const GifImageDesc &desc = saved.ImageDesc;
        const ColorMapObject *colorMap = desc.ColorMap ? desc.ColorMap : commonMap;
        std::cout << "[" << i << "] " << desc.Width << "x" << desc.Height << "+" << desc.Left << "," << desc.Top
                  << ", has local colorMap: " << (desc.ColorMap ? "Yes" : "No") << std::endl;

//        std::stringstream ss;
//        for (int v = 0; v < desc.Height; ++v) {
//            for (int u = 0; u < desc.Width; ++u) {
//                int c = saved.RasterBits[v * desc.Width + u];
//                printf(" %02X", c);
//                if (colorMap) {
//                    GifColorType rgb = colorMap->Colors[c];
//                    ss << " [" << (int)rgb.Red << "," << (int)rgb.Green << "," << (int)rgb.Blue << "]";
//                }
//            }
//            std::cout << ":" << ss.str() << std::endl;
//        }
    }

    DGifCloseFile(gifFile, &error);
    return true;
}

int main__(int argc, char *argv[]) {
    boost::filesystem::path path(boost::filesystem::current_path());
    path /= "flash";
    boost::filesystem::path json_file = path / "feed.gif";
    gif_read(json_file.string().c_str());
    return 0;
}

int main(int argc, char *argv[]) {
    boost::filesystem::path path(boost::filesystem::current_path());
    path /= "flash";
    boost::filesystem::path json_file = path / "memory.json";

    try {
        auto json = file_content(json_file);
        auto value = boost::json::parse(json);
        auto images = value.as_object()["images"].as_array();
        auto records_sector = value.as_object()["records_sector"].as_int64();
        auto data_sector = value.as_object()["data_sector"].as_int64();
        auto records_address = records_sector << 12;
        auto data_address = data_sector << 12;

        for (auto &img : images) {
            auto tag = img.as_object()["tag"].as_int64();
            auto file = img.as_object()["file"].as_string();
            boost::filesystem::path img_file = path / file.c_str();
            read_png(img_file.string().c_str(), canvas);

            flash_record r{};
            r.tag = tag;
            r.offset = data.size();
            r.size = sizeof(canvas);
            data.insert(data.end(), canvas, canvas + sizeof(canvas));

            std::cout << "v: " << img_file << std::endl;

            records.emplace_back(r);
        }

        for (auto &r : records) {
            r.offset += data_address;
            r.tag = boost::endian::native_to_little(r.tag);
            r.offset = boost::endian::native_to_little(r.offset);
            r.size = boost::endian::native_to_little(r.size);
        }
        {
            flash_record r{};
            r.tag = 0xFFFFFFFF;
            r.offset = 0xFFFFFFFF;
            r.size = 0xFFFFFFFF;
            records.emplace_back(r);
        }

        std::stringstream ss;
        ss << "flash" << std::endl;
        ss << "erase_sector " << records_sector << std::endl;

        auto data_size = data.size();
        auto data_end = data_address + data_size;
        auto address = (size_t)data_address;
        while (address < data_end) {
            ss << "erase_sector " << (address >> 12) << std::endl;
            address += 0x1000;
        }

        write_flash(ss, records_address, (const uint8_t *)records.data(), records.size() * sizeof(flash_record));
        write_flash(ss, data_address, data.data(), data_size);

        ss << std::endl << "finish" << std::endl;

        boost::filesystem::path flash_file = path / "flash.txt";
        std::string str = ss.str();
        std::fstream file;
        file.open(flash_file.c_str(), std::fstream::out | std::fstream::trunc);
        file << str;
        file.close();

        std::cout << str;

    } catch (std::exception &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
