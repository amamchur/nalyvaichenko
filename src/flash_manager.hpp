#ifndef NALYVAICHENKO_FLASH_MANAGER_HPP
#define NALYVAICHENKO_FLASH_MANAGER_HPP

#include "./parsers/flash_machine.hpp"

#include <cstddef>
#include <cstdint>

struct flash_record {
    uint32_t tag;
    uint32_t offset;
    uint32_t size;
};

class flash_resource {
public:
    static constexpr uint32_t logo = 0;
};

class flash_manager {
public:
    zoal::misc::flash_machine machine;
    flash_record records[256]{0};
    size_t records_count{0};

    flash_manager() noexcept;
    void read_records();
    size_t read_by_tag(uint32_t tag, void *buffer, size_t size);
    void process_command(const void *buffer, size_t size);

private:
    static void callback(zoal::misc::flash_machine *m, const zoal::misc::flash_cmd &cmd);
};

extern flash_manager fm;

#endif
