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
    cmd_go = 'go' %{ this->command_ = command_type::go; };
    cmd_play = ('play' space+ positive) %{ this->command_ = command_type::play; };
    cmd_valve = ('valve' space+ positive) %{ this->command_ = command_type::valve; };
    cmd_pump = ('pump' space+ positive) %{ this->command_ = command_type::pump; };
    cmd_enc = ('enc' space+ integer) %{ this->command_ = command_type::enc; };
    cmd_press = 'press' %{ this->command_ = command_type::press; };
    cmd_settings = 'settings' %{ this->command_ = command_type::settings; };
    cmd_flash = ('flash') %{ this->command_ = command_type::flash; };

    cmd_motor_enable = ('motor' space+ 'enable') %{ this->command_ = command_type::motor_enable; };
    cmd_motor_disable = ('motor' space+ 'disable') %{ this->command_ = command_type::motor_disable; };
    cmd_motor_dir_cw = ('motor' space+ 'dir' space+ 'cw') %{ this->command_ = command_type::motor_direction_cw; };
    cmd_motor_dir_ccw = ('motor' space+ 'dir' space+ 'ccw') %{ this->command_ = command_type::motor_direction_ccw; };
    cmd_motor_rpm = ('motor' space+ 'rpm' space+ integer) %{ this->command_ = command_type::motor_rpm; };
    cmd_motor_accel = ('motor' space+ 'accel' space+ integer) %{ this->command_ = command_type::motor_accel; };
    cmd_motor_step = ('motor' space+ 'step' space+ integer) %{ this->command_ = command_type::motor_step; };
    cmd_motor_deg = ('motor' space+ 'deg' space+ integer) %{ this->command_ = command_type::motor_deg; };
    cmd_motor_info = ('motor' space+ 'info') %{ this->command_ = command_type::motor_info; };

    cmd_ui_anim = ('ui' space+ 'anim' space+ integer) %{ this->command_ = command_type::ui_anim; };
    cmd_ui_image = ('ui' space+ 'image' space+ integer) %{ this->command_ = command_type::ui_image; };
    cmd_ui_logo = ('ui' space+ 'logo') %{ this->command_ = command_type::ui_logo; };

    cmd_df_volume_read = ('df_volume') %{ this->command_ = command_type::df_volume_read; };
    cmd_df_volume_write = ('df_volume' space+ integer) %{ this->command_ = command_type::df_volume_write; };
    cmd_df_reset = ('df_reset') %{ this->command_ = command_type::df_reset; };
    cmd_df_status = ('df_status') %{ this->command_ = command_type::df_status; };

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
        cmd_go |
        cmd_settings |
        cmd_press |
        cmd_flash |
        cmd_motor_enable |
        cmd_motor_disable |
        cmd_motor_dir_cw |
        cmd_motor_dir_ccw |
        cmd_motor_rpm |
        cmd_motor_accel |
        cmd_motor_step |
        cmd_motor_deg |
        cmd_motor_info |

        cmd_ui_anim |
        cmd_ui_logo |
        cmd_ui_image |

        cmd_df_volume_read |
        cmd_df_volume_write |
        cmd_df_reset |
        cmd_df_status
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
