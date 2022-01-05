%%{
	machine fsm_df_player;
	response = 0x7E . extend{8} . 0xEF;
	main := |*
		response => { handler_(*this); };
		extend => { /* trash */};
	*|;
}%%

#include "./df_player_machine.hpp"

namespace zoal { namespace misc {
    %% write data noerror nofinal noentry;

    void df_player_scanner::init_machine() {
        %% write init;
    }

    const char *df_player_scanner::run_machine(const char *p, const char *pe) {
        %% write exec;
        return p;
    }

    int df_player_scanner::start_state() const {
        return fsm_df_player_start;
    }

    df_player_parser::df_player_parser(void *buffer, size_t buffer_size)
        : ragel_parser(buffer, buffer_size) {}
}}
