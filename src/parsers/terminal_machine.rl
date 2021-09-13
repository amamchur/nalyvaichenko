#include "terminal_machine.hpp"

%%{
    machine fsm_name;

    esc = 27;
    main := |*
        esc 79 67 => {
            handler_(this, terminal_machine_event::ctrl_right_key);
        };
        esc 79 68 => {
            handler_(this, terminal_machine_event::ctrl_left_key);
        };
        esc 91 65 => {
            handler_(this, terminal_machine_event::up_key);
        };
        esc 91 66 => {
            handler_(this, terminal_machine_event::down_key);
        };
        esc 91 67 => {
            handler_(this, terminal_machine_event::right_key);
        };
        esc 91 68 => {
            handler_(this, terminal_machine_event::left_key);
        };
        esc 91 49 126 => {
            handler_(this, terminal_machine_event::home_key);
        };
        esc 91 51 126 => {
            handler_(this, terminal_machine_event::delete_key);
        };
        esc 91 52 126 => {
            handler_(this, terminal_machine_event::end_key);
        };
        127 => {
            handler_(this, terminal_machine_event::backspace_key);
        };
        13 => {
            handler_(this, terminal_machine_event::carriage_return);
        };
        (ascii - cntrl)=> {
            handler_(this, terminal_machine_event::ascii);
        };
        esc 91 any;
        esc 79 any;
        esc 91 any;
        any;
    *|;
}%%

namespace {
    %% write data;
}

namespace zoal { namespace misc {
    void terminal_machine::init_machine() {
        %% write init;
    }

    int terminal_machine::start_state() const {
        return fsm_name_start;
    }

    const char *terminal_machine::run_machine(const char *p, const char *pe) {
        %% write exec;
        return p;
    }
}}
