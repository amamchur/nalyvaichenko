%%{
	machine fsm_test;

	action finished {
	    this->handler_(this, this->command_, this->argc, this->argv);
	}

	action arg_start {
        argv[argc].start = p;
	}

    action arg_finished {
        argv[argc++].end = p;
    }

    hours =  ([0-1] . digit) | ('2' . [0-3]);
    min_sec = ([0-5] . digit);
    time = hours . ':' . min_sec . ':' min_sec;

    year = ('0') | ('1'..'9' . digit*);
    month = ('0' . digit) | ('1' . '0..2');
    day = ('3' . '0'..'1') | ('0'..'2' . digit);
    date = year . '.' . month . '.' day;

    date_time = (date . space . time) >arg_start %arg_finished;
    quoted_string = ('"' ((ascii - cntrl - space - '\\') | [ \t] | '\\'["tvfnr])+ '"') >arg_start %arg_finished;
	positive = ([1-9] digit*) >arg_start %arg_finished;
	integer = ('-'? digit+) >arg_start %arg_finished;

    cmd_adc = 'adc' %{ this->command_ = command_type::show_adc; };
    cmd_help = 'help' %{ this->command_ = command_type::show_help; };
    cmd_stop = 'stop' %{ this->command_ = command_type::stop; };
    cmd_i2c = 'i2c' %{ this->command_ = command_type::scan_i2c; };
    cmd_next = 'next' %{ this->command_ = command_type::next_segment; };
    cmd_logo = 'logo' %{ this->command_ = command_type::logo; };
    cmd_go = 'go' %{ this->command_ = command_type::go; };
    cmd_play = ('play' space+ positive) %{ this->command_ = command_type::play; };
    cmd_valve = ('valve' space+ positive) %{ this->command_ = command_type::valve; };
    cmd_pump = ('pump' space+ positive) %{ this->command_ = command_type::pump; };
    cmd_enc = ('enc' space+ integer) %{ this->command_ = command_type::enc; };
    cmd_rotate = ('rotate' space+ integer) %{ this->command_ = command_type::rotate; };
    cmd_calibrate = 'calibrate' %{ this->command_ = command_type::calibrate; };
    cmd_settings = 'settings' %{ this->command_ = command_type::settings; };

    commands = (
        cmd_adc |
        cmd_help |
        cmd_stop |
        cmd_i2c |
        cmd_play |
        cmd_valve |
        cmd_pump |
        cmd_enc |
        cmd_next |
        cmd_logo |
        cmd_go |
        cmd_calibrate |
        cmd_rotate |
        cmd_settings
    );

	main := (space* commands space*) %finished;
}%%

#include "command_machine.hpp"

namespace zoal { namespace misc {
    %% write data noerror nofinal noentry;

    command_machine::command_machine() noexcept {
        reset();
    }

    void command_machine::reset() {
        this->command_ = command_type::none;
        this->argc = 0;

        %% write init;
    }

    int command_machine::start_state() {
        return %%{ write start; }%%;
    }

    const char *command_machine::run_machine(const char *p, const char *pe, const char *eof) {
        %% write exec;
        return p;
    }
}}
