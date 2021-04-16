#ifndef ZOAL_TERMINAL_INPUT_HPP
#define ZOAL_TERMINAL_INPUT_HPP

#include "terminal_machine.hpp"

namespace zoal { namespace misc {
    class terminal_input : public zoal::parse::ragel_scanner<terminal_machine> {
    public:
        using self_type = terminal_input;

        typedef void (*callback_fn)(const self_type *t, const char *s, const char *e);
        typedef void (*handle_v100_fn)(const self_type *t, terminal_machine_event e);

        terminal_input(void *buffer, size_t size) noexcept;
        void init_buffer(char *buffer, size_t size);

        const char *str_start() const;
        const char *str_end() const;
        void clear();
        void sync() const;
        void value(const char *str);

        inline void greeting(const char *g) {
            greeting_ = g;
        }

        inline void vt100_feedback(callback_fn fn) {
            vt100_callback_ = fn;
        }

        inline void input_callback(callback_fn fn) {
            input_callback_ = fn;
        }

        inline void handle_v100(handle_v100_fn fn) {
            handle_v100_fn_ = fn;
        }

        inline void context(void *ctx) {
            context_ = ctx;
        };
        inline void *context() const {
            return context_;
        }

    public:
        char scanner_buffer_[8]{0};

        const char *greeting_{nullptr};
        char *buffer_{nullptr};
        char *cursor_{nullptr};
        char *end_{nullptr};
        size_t size_{0};

        void *context_{nullptr};
        callback_fn vt100_callback_{&empty_callback};
        callback_fn input_callback_{&empty_callback};
        handle_v100_fn handle_v100_fn_{nullptr};

        void cursor(char *c);
        char *cursor() const;
        void insert_char(char ch);
        void do_backspace();
        void do_delete();

        static void scanner_callback(void *parser, terminal_machine_event event);
        static void empty_callback(const self_type *t, const char *s, const char *e);
        void process_event(terminal_machine_event e);
        void move_to_word_end();
        void move_to_word_start();
        char *normalize_cursor(char *c) const;
        void send_cstr(const char *str) const;
    };
}}

#endif
