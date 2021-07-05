#ifndef CMD_LINE_PARSER_HPP
#define CMD_LINE_PARSER_HPP

#include <stddef.h>
#include <stdint.h>
#include <zoal/parse/ragel_scanner.hpp>

namespace zoal { namespace misc {
    enum class command_line_event { token, line_end };

    class command_line_machine : public zoal::parse::scanner_machine<command_line_machine *, command_line_event> {
    public:
        void init_machine();
        int start_state() const;
        void quoted_param_found_action();

        const char *run_machine(const char *p, const char *pe);
    };

    class command_line_parser : public zoal::parse::ragel_scanner<command_line_machine> {
    public:
        command_line_parser(void *buffer, size_t buffer_size);
    };
}}

#endif
