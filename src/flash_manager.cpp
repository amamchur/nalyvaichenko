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

size_t flash_manager::read_frame(uint32_t tag, uint32_t index, void *buffer, size_t size) {
    for (size_t i = 0; i < records_count; i++) {
        if (records[i].tag == tag) {
            auto address = records[i].address + 1024 * index;
            w25q32::fast_read(address, buffer, size);
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
    machine.callback(parse_callback);
    machine.context = this;
}

void flash_manager::parse_callback(zoal::misc::flash_machine *m, const zoal::misc::flash_cmd &cmd) {
    auto me = reinterpret_cast<flash_manager *>(m->context);
    switch (cmd.type) {
    case zoal::misc::flash_cmd_type::erase_chip:
        w25q32::chip_erase();
        me->status_callback(flash_command_result::chip_erased, 0, 0);
        break;
    case zoal::misc::flash_cmd_type::erase_sector:
        w25q32::sector_erase(cmd.address);
        me->status_callback(flash_command_result::sector_erased, cmd.address, 0);
        break;
    case zoal::misc::flash_cmd_type::prog_mem:
        w25q32::page_program(cmd.address, cmd.data, cmd.size);
        me->status_callback(flash_command_result::page_programed, cmd.address, cmd.size);
        break;
    case zoal::misc::flash_cmd_type::finish:
        reinterpret_cast<flash_manager *>(m->context)->read_records();
        global_app_state.flash_editor = false;
        me->status_callback(flash_command_result::finished, 0, 0);
        break;
    default:
        break;
    }
}

bool flash_manager::get_record(uint32_t tag, flash_record &r) {
    for (size_t i = 0; i < records_count; i++) {
        if (records[i].tag == tag) {
            r = records[i];
            return true;
        }
    }
    return false;
}

void flash_manager::null_callback(flash_command_result r, uint32_t address, uint32_t size) {}
