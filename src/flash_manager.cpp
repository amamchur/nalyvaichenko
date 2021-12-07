#include "flash_manager.hpp"

#include "./app_state.hpp"
#include "./config.hpp"

flash_manager fm;

void flash_manager::read_records() {
    records_count = 0;
    w25q32::fast_read(4096, &records, sizeof(records));
    for (auto &record : records) {
        if (record.tag == 0xFFFFFFFF) {
            break;
        }
        records_count++;
    }
}

size_t flash_manager::read_by_tag(uint32_t tag, void *buffer, size_t size) {
    for (size_t i = 0; i < records_count; i++) {
        if (records[i].tag == tag) {
            w25q32::fast_read(records[i].offset, buffer, size);
            return size;
        }
    }

    return 0;
}

void flash_manager::process_command(const void *buffer, size_t size) {
    auto start = reinterpret_cast<const char *>(buffer);
    machine.run_machine(start, start + size, nullptr);
}

flash_manager::flash_manager() noexcept {
    machine.callback(callback);
    machine.context = this;
}

void flash_manager::callback(zoal::misc::flash_machine *m, const zoal::misc::flash_cmd &cmd) {
    switch (cmd.type) {
    case zoal::misc::flash_cmd_type::erase_chip:
        w25q32::chip_erase();
        break;
    case zoal::misc::flash_cmd_type::erase_sector:
        w25q32::sector_erase(cmd.address);
        break;
    case zoal::misc::flash_cmd_type::prog_mem:
        w25q32::page_program(cmd.address, cmd.data, cmd.size);
        break;
    case zoal::misc::flash_cmd_type::finish:
        reinterpret_cast<flash_manager *>(m->context)->read_records();
        global_app_state.flash_editor = false;
        break;
    default:
        break;
    }
}
