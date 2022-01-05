#include "../parsers/df_player_machine.hpp"

#include <iomanip>
#include <iostream>

uint8_t buffer[1000];

void handler(const zoal::misc::df_player_scanner &m) {
    auto s = reinterpret_cast<const uint8_t *>(m.token_start());
    auto e = reinterpret_cast<const uint8_t *>(m.token_end());
    while (s < e) {
        std::cout << " 0x" << std::hex << static_cast<int>(*s);
        s++;
    }
    std::cout << std::endl;
}

uint8_t bytes[] = {0x7E, 0xFF, 0x06, 0x40, 0x00, 0x00, 0x03, 0xFE, 0xB8, 0xEF};

int main(int argc, char *argv[]) {
    zoal::misc::df_player_parser pp(buffer, sizeof(buffer));
    pp.callback(&handler);
    for (size_t i = 0; i < sizeof(bytes); i++) {
        pp.push_and_scan(bytes + i, 1);
    }
    return 0;
}
