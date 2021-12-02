#include "../logo/ecafe_logo.hpp"
#include "../logo/test_logo.hpp"
#include "../parsers/command_machine.hpp"
#include "../parsers/flash_machine.hpp"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

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

int main(int argc, char *argv[]) {
    fm.callback(fm_callback);

    image_data images[] = {{1 * 4096, ecafe_logo, ecafe_logo_size}, {2 * 4096, epd_bitmap_ecafe, epd_bitmap_ecafe_size}};
    std::stringstream ss;
    auto image = images[1];
    auto address = image.address;

    ss << "erase_sector 2" << std::endl;

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
