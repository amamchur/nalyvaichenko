#ifndef NALYVAICHENKO_FLASH_MANAGER_HPP
#define NALYVAICHENKO_FLASH_MANAGER_HPP

#include "./flash_struct.hpp"
#include "./parsers/flash_machine.hpp"

#include <cstddef>
#include <cstdint>

class flash_resource {
public:
    static constexpr uint32_t logo = 0;
};

enum class flash_command_result { chip_erased, sector_erased, page_programed, finished };

class flash_manager {
public:
    zoal::misc::flash_machine machine;
    flash_record records[256]{0};
    size_t records_count{0};

    flash_manager() noexcept;
    void read_records();
    size_t read_frame(uint32_t tag, uint32_t index, void *buffer, size_t size);
    void process_command(const void *buffer, size_t size);
    bool get_record(uint32_t tag, flash_record &r);

    void (*status_callback)(flash_command_result r, uint32_t address, uint32_t size){null_callback};

private:
    static void null_callback(flash_command_result r, uint32_t address, uint32_t size);
    static void parse_callback(zoal::misc::flash_machine *m, const zoal::misc::flash_cmd &cmd);
};

extern flash_manager fm;

#endif
