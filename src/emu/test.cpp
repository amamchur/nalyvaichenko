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

char text1[] = "prog_mem 4096 0000c0\n";
char text[] = "prog_mem 4096 "
              "000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
              "00000000000000000000000000000000000000000000000000000000c0e0c08000000000000000000000000000000000000000000000000000000000000000000000000000000000"
              "000000000000000000000000000000808080c0c0c0c08080800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
              "00000000000000000080f0fcffffff3f0f0600000000000000000000000000000000000000000000\n"
              "prog_mem 4351 "
              "00000000000000000000000000183870e0e0c0c0808080c0f0f8fcfcfe7f3f1f0f0f07070303038383c3e77e3c000000000000000000000000000000000000000000000000000000"
              "00000000000000000000000000000000000000000000c0f8feffff7f1f03000000000000000000000000000000000000000000000000000000000000000000000000000000000080"
              "808181c3c3c7c7c7dfffffffffeecece8e8e8e0f0707070303010000000000000000000080c0e0f0f0f8f87c3c1c1c18b8f000000080c0e0f0f0f8f8787060e0f0f8f8f0e0000010"
              "181898f8fcffffff1f1f191818080080c0e0f0f8fc7c3c38f0e00000000000000000000000000000\n"
              "prog_mem 4606 "
              "0000000000000080e0f8fcfefe7f1f0f070303010101000000000000000101010101010100000000000000000000000000e0fcfeff7f1f0f0301000c0c0c060703e1f0fcfeff3f0f"
              "0783c17038fcffffff7f1f07010080e0f8feff3f0fffff3f3060e0f0fcfeff7f6f67633130180c0703000000000000000000000000000000000000000000001f7fffffffffe08000"
              "000000000000000000000000000000000080c0c06030180c00003078fc78300000071f3f3f7c706060607030180c0c060f1f3f3f3f3c1c0e070100001f3f7f7f233018ccf6ffffff"
              "3f0f01e0fe7f030000000f1f3f7f70706060603030180c070000c0fcf8e000000000000000000000\n"
              "prog_mem 4861 "
              "000000000000000000010303070f0f0f1e1e1c1c18181818181c1c0c0e0e060703018180c0c0e0e0f0f0f8f8783c3c3c1c1e1e0e0e0e0e0e07070707070707070303030303030303"
              "030303030706060606feffffdf9f1b98f8f87e3f33707070707070f0f0f0f0f0f0f0f0f878787c3c3e1f1f0f03000000000000000000000000000000000000000000000000000000"
              "000000000000000000000000000000000003070f0f070703010100000000000000000000000000000000000000000000000000000000000000000000000000000000010103030301"
              "01000000000000000000000000000000000000000000000000000000000000000000000000000000\n"
              "finish\n";

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

int main(int argc, char *argv[]) {
    fm.callback(fm_callback);

//    for (int i = 0; text1[i] != 0; i++) {
//        fm.run_machine(text1 + i, text1 + i + 1, nullptr);
//    }

    std::stringstream ss;
    int address = 4096 * 2;

    ss << "erase_sector 2" << std::endl;

    for (size_t i = 0; i < epd_bitmap_ecafe_size; i++) {
        if ((i & 0xFF) == 0) {
            ss << std::dec << std::endl << "prog_mem " << address << " ";
            address += 0xFF;
        }
        uint8_t byte = epd_bitmap_ecafe[i];
        ss << std::hex << (int)(byte >> 4) << (int)(byte & 0xF);
    }

    ss << std::endl;

    std::string str = ss.str();
    std::ofstream file;
    file.open("D:/flash.txt");
    file << str;
    file.close();

    auto cmds = str.c_str();
    for (int i = 0; cmds[i] != 0; i++) {
        fm.run_machine(cmds + i, cmds + i + 1, nullptr);
    }
    for (size_t i = 0; i < epd_bitmap_ecafe_size; i++) {
        if (buffer[i] != epd_bitmap_ecafe[i]) {
            std::cout << "Error! " << (int)buffer[i] << " !=" << (int)epd_bitmap_ecafe[i] << std::endl;
        }
    }

    return 0;
}
