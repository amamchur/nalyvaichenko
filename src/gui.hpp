//
// Created by andrii on 03.07.21.
//

#ifndef NALYVAICHENKO_GUI_HPP
#define NALYVAICHENKO_GUI_HPP

#include "app_state.hpp"
#include "command.hpp"
#include "hardware.hpp"

#include <zoal/gfx/glyph_render.hpp>

class menu_item {
public:
    explicit menu_item(const wchar_t *t);
    menu_item(const wchar_t *t, void (*a)());

    const wchar_t *text{nullptr};
    void (*action)(){nullptr};
    menu_item *prev{nullptr};
    menu_item *next{nullptr};
};

class gui {
public:
    explicit gui(app_state &app_state);

    void next_item();
    void prev_item();
    void exec_item();

    void render();

private:
    static void render_progmem_text(zoal::gfx::glyph_render<graphics> &gr, const wchar_t *ptr) ;

    menu_item *current_{nullptr};
    app_state &app_state_;
    void render_calibration();
    void render_menu() const;
    void render_error();
};

#endif //NALYVAICHENKO_GUI_HPP
