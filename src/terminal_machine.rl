#include "terminal_machine.hpp"

%%{
    machine fsm_name;

    main := |*
        27 79 65 => {
            handler_(this, terminal_machine_event::ctrl_up_key);
        };
        27 79 66 => {
            handler_(this, terminal_machine_event::ctrl_down_key);
        };
        27 79 67 => {
            handler_(this, terminal_machine_event::ctrl_right_key);
        };
        27 79 68 => {
            handler_(this, terminal_machine_event::ctrl_left_key);
        };
        27 91 65 => {
            handler_(this, terminal_machine_event::up_key);
        };
        27 91 66 => {
            handler_(this, terminal_machine_event::down_key);
        };
        27 91 67 => {
            handler_(this, terminal_machine_event::right_key);
        };
        27 91 68 => {
            handler_(this, terminal_machine_event::left_key);
        };
        27 91 49 126 => {
            handler_(this, terminal_machine_event::home_key);
        };
        27 91 51 126 => {
            handler_(this, terminal_machine_event::delete_key);
        };
        27 91 52 126 => {
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
        27 91 any;
        27 79 any;
        27 91 any;
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
