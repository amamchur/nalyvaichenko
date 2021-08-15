//
// Created by andrii on 03.07.21.
//

#include "gui.hpp"

#include "./fonts/roboto_regular_18.hpp"
#include "./voice.hpp"

#include <avr/pgmspace.h>
#include <zoal/arch/avr/utils/progmem_reader.hpp>
#include <zoal/gfx/glyph_renderer.hpp>
#include <zoal/gfx/renderer.hpp>

menu_item::menu_item(const wchar_t *t)
    : text(t) {}

menu_item::menu_item(const wchar_t *t, void (*a)(gui &))
    : text(t)
    , action(a) {}

static const wchar_t text_calibration[] PROGMEM = L"Калібування...";
static const wchar_t text_error[] PROGMEM = L"Ой шось не так";

static const wchar_t menu1_text[] PROGMEM = L"Запуск";
static const wchar_t menu2_text[] PROGMEM = L"Калібування";
static const wchar_t menu3_text[] PROGMEM = L"Револьвер";
static const wchar_t menu4_text[] PROGMEM = L"Сегмент++";
static const wchar_t menu5_text[] PROGMEM = L"Лого";
static const wchar_t menu5_1_text[] PROGMEM = L"Привіт";
static const wchar_t menu6_text[] PROGMEM = L"Прокачка";

static void go_action(gui &) {
    send_command(command_type::go);
}

menu_item item1(menu1_text, go_action);

static void calibrate_action(gui &) {
    send_command(command_type::calibrate);
}

menu_item item2(menu2_text, calibrate_action);
menu_item item3(menu3_text);

static void next_segment_action(gui &) {
    send_command(command_type::next_segment);
}

menu_item item4(menu4_text, next_segment_action);

static void logo_action(gui &gui) {
    send_command(command_type::logo);
}

menu_item item5(menu5_text, logo_action);

static void hello(gui &) {
    command cmd(command_type::play);
    cmd.value = voice::hello;
    send_command(cmd);
}

menu_item item5_1(menu5_1_text, hello);

static void pump(gui &) {
    send_command(command_type::pump);
}

menu_item item6(menu6_text, pump);

gui::gui(app_state &app_state)
    : app_state_(app_state) {
    item1.next = &item2;
    item2.prev = &item1;

    item2.next = &item3;
    item3.prev = &item2;

    item3.next = &item4;
    item4.prev = &item3;

    item4.next = &item5;
    item5.prev = &item4;

    item5.next = &item6;
    item6.prev = &item5;

//    item6.next = &item7;
//    item7.prev = &item6;

    current_ = &item1;
}

void gui::render_calibration() {
    auto font = &roboto_regular_18;
    auto g = graphics::from_memory(screen.buffer.canvas);
    zoal::gfx::glyph_renderer<graphics, zoal::utils::progmem_reader> gr(g, font);

    g->clear(0);
    gr.color(1);
    gr.position(0, font->y_advance * 2);
    render_progmem_text(gr, text_calibration);
}

void gui::render_error() {
    auto font = &roboto_regular_18;
    auto g = graphics::from_memory(screen.buffer.canvas);
    zoal::gfx::glyph_renderer<graphics, zoal::utils::progmem_reader> gr(g, font);

    g->clear(0);
    gr.position(0, font->y_advance);
    render_progmem_text(gr, text_error);
}

void gui::render_menu() const {
    auto font = &roboto_regular_18;
    auto g = graphics::from_memory(screen.buffer.canvas);
    zoal::gfx::glyph_renderer<graphics, zoal::utils::progmem_reader> gr(g, font);

    g->clear(0);

    int origin_y = (64 + font->y_advance) / 2;
    int y = origin_y;
    gr.color(1);
    gr.position(0, y).draw(">");
    gr.position(12, y);
    render_progmem_text(gr, current_->text);

    auto item = current_->next;
    y = origin_y + font->y_advance;
    while (y <= 80 && item != nullptr) {
        gr.position(12, y);
        render_progmem_text(gr, item->text);
        y += font->y_advance;
        item = item->next;
    }

    item = current_->prev;
    y = origin_y - font->y_advance;
    while (y > 0 && item != nullptr) {
        gr.position(12, y);
        render_progmem_text(gr, item->text);
        y -= font->y_advance;
        item = item->prev;
    }
}

void gui::render() {
    if ((app_state_.flags & app_state_flags_error) != 0) {
        render_error();
        return;
    }

    if ((app_state_.flags & app_state_flags_calibration) != 0) {
        render_calibration();
        return;
    }

    render_menu();
}

void gui::next_item() {
    if (app_state_.flags != app_state_flags_idle) {
        return;
    }

    if (current_->next != nullptr) {
        current_ = current_->next;
        send_command(command_type::request_render_frame);
    }
}

void gui::prev_item() {
    if (app_state_.flags != app_state_flags_idle) {
        return;
    }

    if (current_->prev != nullptr) {
        current_ = current_->prev;
        send_command(command_type::request_render_frame);
    }
}

void gui::exec_item() {
    if (app_state_.flags == app_state_flags_error) {
        send_command(command_type::clear_error);
        return;
    }

    if (app_state_.flags != app_state_flags_idle) {
        return;
    }

    if (current_->action != nullptr) {
        current_->action(*this);
    }
}
