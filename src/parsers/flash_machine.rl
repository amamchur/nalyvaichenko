%%{
	machine fsm_flash;

	action finished {
	    this->handler_(this, this->command_);
        this->command_.type = flash_cmd_type::none;
        this->command_.address = 0;
        this->command_.size = 0;
	}

    action address_all {
        this->command_.address = this->command_.address * 10 + (*p - '0');
    }

	address = ('-'? digit+) $address_all;
	byte = (xdigit xdigit) ${ take_octet(*p); } %{ this->command_.data[this->command_.size++] = byte; };
	bytes = byte+;

    cmd_erase_chip = 'erase_chip' %{ this->command_.type = flash_cmd_type::erase_chip; };
    cmd_erase_sector = ('erase_sector' space+ address) %{ this->command_.type = flash_cmd_type::erase_sector; };
    cmd_prog_mem = ('prog_mem' space+ address space+ bytes) %{ this->command_.type = flash_cmd_type::prog_mem; };
    cmd_finish = 'finish' %{ this->command_.type = flash_cmd_type::finish; };
    commands = (cmd_erase_sector | cmd_finish | cmd_prog_mem) %finished;
	main := (space* commands space* '\r'? '\n')*;
}%%

#include "flash_machine.hpp"

namespace zoal { namespace misc {
    %% write data noerror nofinal noentry;

    void flash_machine::take_octet(char ch) {
        byte = (byte << 4) & 0xFF;

        if (ch >= '0' && ch <= '9') {
            byte += ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            byte += ch - 'a' + 10;
        }
        if (ch >= 'A' && ch <= 'F') {
            byte += ch - 'A' + 10;
        }
    }

    flash_machine::flash_machine() noexcept {
        reset();
    }

    void flash_machine::reset() {
        this->command_.type = flash_cmd_type::none;
        this->command_.address = 0;
        this->command_.size = 0;
        byte = 0;
        %% write init;
    }

    int flash_machine::start_state() {
        return %%{ write start; }%%;
    }

    const char *flash_machine::run_machine(const char *p, const char *pe, const char *eof) {
        %% write exec;
        return p;
    }
}}
