#ifndef NALYVAICHENKO_FLASH_MACHINE_HPP
#define NALYVAICHENKO_FLASH_MACHINE_HPP

#include <zoal/parse/ragel_parser.hpp>
#include <stdint.h>

namespace zoal { namespace misc {
    enum class flash_cmd_type {
        none,
        erase_chip,
        erase_sector,
        prog_mem,
        finish
    };

    class flash_cmd {
    public:
        flash_cmd_type type;
        uint32_t address;
        size_t size;
        uint8_t data[512];
    };

    class flash_machine : public zoal::parse::ragel_machine<flash_machine *, const flash_cmd&> {
    public:
        flash_machine() noexcept;

        void reset();
        static int start_state();
        const char *run_machine(const char *p, const char *pe, const char *eof);

        void *context;
    private:
        void take_octet(char ch);
        uint8_t byte;
        flash_cmd command_;
    };
}}

#endif
