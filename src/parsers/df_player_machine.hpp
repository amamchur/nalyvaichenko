#ifndef NALYVAICHENKO_DF_PLAYER_MACHINE_HPP
#define NALYVAICHENKO_DF_PLAYER_MACHINE_HPP

#include <stdint.h>
#include <zoal/parse/ragel_parser.hpp>

namespace zoal { namespace misc {
    class df_player_scanner : public zoal::parse::ragel_scanner<const df_player_scanner&> {
    public:
        void init_machine();
        int start_state() const;
        const char *run_machine(const char *p, const char *pe);

        void *context{nullptr};
    };

    class df_player_parser :  public zoal::parse::ragel_parser<df_player_scanner> {
    public:
        df_player_parser(void *buffer, size_t buffer_size);
    };
}}

#endif
