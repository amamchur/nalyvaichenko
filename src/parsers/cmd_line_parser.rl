#include "cmd_line_parser.hpp"

%%{
	machine fsm_name;

	action quoted_param_found {
		quoted_param_found_action();
	}

	quoted_param = '"' ((ascii - cntrl - space - '\\') | [ \t] | '\\'["tvfnr])+ '"';
    simple_param = (ascii - space - ['"\\])+;
	main := |*
        quoted_param => quoted_param_found;
        simple_param => {
            handler_(this, command_line_event::token);
        };
        32;
        '\n' => {
            handler_(this, command_line_event::line_end);
        };

        any;
	*|;
}%%

namespace {
%% write data;
}

namespace zoal { namespace misc {
    command_line_parser::command_line_parser(void *buffer, size_t buffer_size)
        : ragel_parser(buffer, buffer_size) {}

    void command_line_machine::quoted_param_found_action() {
        ts++;
        te--;

        auto dst = const_cast<char *>(ts);
        for (auto ptr = ts; ptr != te; ptr++) {
            if (*ptr != '\\') {
                *dst++ = *ptr;
                continue;
            }

            switch (*++ptr) {
            case 'n':
                *dst = '\n';
                break;
            case 'r':
                *dst = '\r';
                break;
            case 't':
                *dst = '\t';
                break;
            case 'v':
                *dst = '\v';
                break;
            case 'f':
                *dst = '\f';
                break;
            default:
                *dst = *ptr;
                break;
            }

            dst++;
        }

        te = dst;

        if (handler_) {
            handler_(this, command_line_event::token);
        }
    }

    void command_line_machine::init_machine() {
        %% write init;
    }

    const char *command_line_machine::run_machine(const char *p, const char *pe) {
        %% write exec;
        return p;
    }

    int command_line_machine::start_state() const {
        return fsm_name_start;
    }
}}
