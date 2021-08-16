//
// Created by andrii on 03.07.21.
//

#include "gui.hpp"

#include "./fonts/roboto_regular_12.hpp"
#include "./fonts/roboto_regular_16.hpp"
#include "./fonts/roboto_regular_18.hpp"
#include "./hardware.hpp"
#include "./logo/ecafe_logo.hpp"
#include "./voice.hpp"

#include <avr/pgmspace.h>
#include <math.h>
#include <time.h>
#include <zoal/arch/avr/utils/progmem_reader.hpp>
#include <zoal/gfx/glyph_renderer.hpp>
#include <zoal/gfx/renderer.hpp>
#include <zoal/io/output_stream.hpp>

static inline const zoal::text::font *get_font() {
    return &roboto_regular_16;
}

template<class T>
static inline T ensure_range(T value, T min, T max) {
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

static void create_back_trace(menu_item *item) {
    menu_item *prev = nullptr;
    while (item != nullptr) {
        item->prev = prev;
        prev = item;
        item = item->next;
    }
}

constexpr int menu_item_title_offset = 12;
constexpr int menu_item_value_offset = 85;

static const wchar_t text_segments[] PROGMEM = L"Сегментів";
static const wchar_t text_pump_time[] PROGMEM = L"Час";
static const wchar_t text_max[] PROGMEM = L"Макс.";
static const wchar_t text_min[] PROGMEM = L"Мін.";
static const wchar_t text_sector[] PROGMEM = L"Сектор";
static const wchar_t text_sector_a[] PROGMEM = L"Сектор А";
static const wchar_t text_sector_b[] PROGMEM = L"Сектор Б";
static const wchar_t text_ir_sensor[] PROGMEM = L"ІЧС";
static const wchar_t text_hall[] PROGMEM = L"Холл";
static const wchar_t text_back[] PROGMEM = L"Назад";
static const wchar_t text_calibration[] PROGMEM = L"Калібування...";
static const wchar_t text_error[] PROGMEM = L"Ой шось не так";
static const wchar_t suffix_ms[] PROGMEM = L"ms";

static const wchar_t main_menu0_text[] PROGMEM = L"Стор!";
static const wchar_t main_menu1_text[] PROGMEM = L"Запуск";
static const wchar_t main_menu2_text[] PROGMEM = L"Калібування";
static const wchar_t main_menu3_text[] PROGMEM = L"Налаштув.";
static const wchar_t main_menu4_text[] PROGMEM = L"Сегмент++";
static const wchar_t main_menu5_text[] PROGMEM = L"Лого";
static const wchar_t main_menu6_text[] PROGMEM = L"Прокачка";
static const wchar_t main_menu7_text[] PROGMEM = L"Сенсори";

class glyph_renderer : public zoal::gfx::glyph_renderer<graphics, zoal::utils::progmem_reader> {
public:
    glyph_renderer(graphics *g, const zoal::text::font *font)
        : zoal::gfx::glyph_renderer<graphics, zoal::utils::progmem_reader>(g, font) {}

    void draw_progmem(const wchar_t *ptr) {
        auto v = pgm_read_word(ptr++);
        while (v != 0) {
            draw((wchar_t)v);
            v = pgm_read_word(ptr++);
        }
    }
};

using gr_stream = zoal::io::output_stream<glyph_renderer>;

menu_item::menu_item(const wchar_t *t)
    : text(t) {}

menu_item::menu_item(const wchar_t *t, void (*a)(gui &, abstract_screen &parent))
    : text(t)
    , action(a) {}

static void render_menu_items(menu_item *active) {
    auto font = get_font();
    auto gfx = graphics::from_memory(screen.buffer.canvas);
    glyph_renderer gr(gfx, font);

    gfx->clear(0);

    int origin_y = (64 + font->y_advance) / 2;
    int y = origin_y;
    gr.color(1);
    gr.position(0, y).draw(">");
    gr.position(menu_item_title_offset, y);
    gr.draw_progmem(active->text);

    auto item = active->next;
    y = origin_y + font->y_advance;
    while (y <= 80 && item != nullptr) {
        gr.position(menu_item_title_offset, y);
        gr.draw_progmem(item->text);
        y += font->y_advance;
        item = item->next;
    }

    item = active->prev;
    y = origin_y - font->y_advance;
    while (y > 0 && item != nullptr) {
        gr.position(menu_item_title_offset, y);
        gr.draw_progmem(item->text);
        y -= font->y_advance;
        item = item->prev;
    }
}

void gui::render() {
    current_screen_->render(*this);
}

void gui::process_event(event &e) {
    current_screen_->process_event(e, *this);
}
void gui::current_screen(abstract_screen *scr) {
    current_screen_ = scr;
    send_command(command_type::request_render_screen);
}

main_screen::main_screen()
    : menu_item_stop(main_menu0_text, stop_action)
    , menu_item_go(main_menu1_text, go_action)
    , menu_item_calibrate(main_menu2_text, calibrate_action)
    , menu_item_settings(main_menu3_text, settings_action)
    , menu_item_next(main_menu4_text, next_segment_action)
    , menu_item_logo(main_menu5_text, logo_action)
    , menu_item_pump(main_menu6_text, pump)
    , menu_item_sensors(main_menu7_text, sensors) {
    menu_item_logo.next = &menu_item_stop;
    menu_item_stop.next = &menu_item_go;
    menu_item_go.next = &menu_item_pump;
    menu_item_pump.next = &menu_item_next;
    menu_item_next.next = &menu_item_calibrate;
    menu_item_calibrate.next = &menu_item_sensors;
    menu_item_sensors.next = &menu_item_settings;
    menu_item_settings.next = nullptr;

    create_back_trace(&menu_item_logo);
    current_ = &menu_item_logo;
}

void main_screen::go_action(gui &, abstract_screen &) {
    send_command(command_type::go);
}

void main_screen::calibrate_action(gui &, abstract_screen &) {
    send_command(command_type::calibrate);
}

void main_screen::next_segment_action(gui &, abstract_screen &) {
    send_command(command_type::next_segment);
}

void main_screen::logo_action(gui &gui, abstract_screen &) {
    send_command(command_type::logo);
}

void main_screen::pump(gui &, abstract_screen &) {
    send_command(command_type::pump);
}

void main_screen::sensors(gui &g, abstract_screen &) {
    g.current_screen(&g.sensor_screen_);
}

void main_screen::settings_action(gui &g, abstract_screen &parent) {
    g.current_screen(&g.settings_screen_);
}

void main_screen::stop_action(gui &, abstract_screen &parent) {
    send_command(command_type::stop);
}

void ir_settings_screen::process_event(event &e, gui &gui) {
    switch (e.type) {
    case event_type::encoder_cw:
        menu_item_index++;
        break;
    case event_type::encoder_ccw:
        menu_item_index--;
        break;
    case event_type::encoder_press: {
        auto current = gui.current_screen();
        switch (menu_item_index) {
        case 0:
            gui.current_screen(&gui.settings_screen_);
            break;
        case 1:
            gui.input_int_screen_.value = global_app_state.settings.ir_max_value_;
            gui.input_int_screen_.min = 0;
            gui.input_int_screen_.max = 1023;
            gui.input_int_screen_.title_progmem = text_max;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui, current](int v) {
                global_app_state.settings.ir_max_value_ = v;
                global_app_state.save_settings();

                gui.current_screen(current);
            };
            gui.current_screen(&gui.input_int_screen_);
            break;
        case 2:
            gui.input_int_screen_.value = global_app_state.settings.ir_min_value_;
            gui.input_int_screen_.min = 0;
            gui.input_int_screen_.max = 1023;
            gui.input_int_screen_.title_progmem = text_max;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui, current](int v) {
                global_app_state.settings.ir_min_value_ = v;
                global_app_state.save_settings();

                gui.current_screen(current);
            };
            gui.current_screen(&gui.input_int_screen_);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    menu_item_index = ensure_range(menu_item_index, 0, 2);
    send_command(command_type::request_render_screen);
}

void ir_settings_screen::render(gui &g) {
    auto font = get_font();
    auto gfx = graphics::from_memory(screen.buffer.canvas);
    glyph_renderer gr(gfx, font);
    gr_stream grs(gr);

    gfx->clear(0);
    gr.color(1);
    gr.position(menu_item_title_offset, font->y_advance);
    gr.draw_progmem(text_back);
    if (menu_item_index == 0) {
        gr.position(0, font->y_advance * 1).draw(">");
    }

    gr.position(menu_item_title_offset, font->y_advance * 2);
    gr.draw_progmem(text_max);
    gr.position(menu_item_value_offset, font->y_advance * 2);
    grs << global_app_state.settings.ir_max_value_;
    if (menu_item_index == 1) {
        gr.position(0, font->y_advance * 2).draw(">");
    }

    gr.position(menu_item_title_offset, font->y_advance * 3);
    gr.draw_progmem(text_min);
    gr.position(menu_item_value_offset, font->y_advance * 3);
    grs << global_app_state.settings.ir_min_value_;
    if (menu_item_index == 2) {
        gr.position(0, font->y_advance * 3).draw(">");
    }
}

void input_int_screen::process_event(event &e, gui &g) {
    switch (e.type) {
    case event_type::encoder_cw:
        value++;
        break;
    case event_type::encoder_ccw:
        value--;
        break;
    case event_type::encoder_press:
        if (callback) {
            callback(value);
        }
        break;
    default:
        break;
    }

    value = ensure_range(value, min, max);
    send_command(command_type::request_render_screen);
}

void input_int_screen::render(gui &g) {
    auto font = get_font();
    auto gfx = graphics::from_memory(screen.buffer.canvas);
    glyph_renderer gr(gfx, font);
    gr_stream grs(gr);

    gfx->clear(0);
    gr.color(1);

    if (title_progmem) {
        gr.position(0, font->y_advance * 1);
        gr.draw_progmem(title_progmem);
    }

    gr.position(0, font->y_advance * 3);
    grs << value;
    if (suffix_progmem) {
        gr.draw_progmem(title_progmem);
    }
}

void logo_screen::render(gui &g) {
    memcpy_P(screen.buffer.canvas, ecafe_logo, sizeof(screen.buffer.canvas));
}

void calibration_screen::render(gui &g) {
    constexpr int r = 30;
    constexpr int x = 94;
    constexpr int y = 32;

    auto font = get_font();
    auto gfx = graphics::from_memory(screen.buffer.canvas);
    glyph_renderer gr(gfx, font);
    gr_stream grs(gr);

    gfx->clear(0);
    gfx->draw_circle(x, y, r, 1);

    auto count = global_app_state.settings.total_segments_;
    auto rad = (2 * M_PI) / count;
    for (int i = 0; i < count; i++) {
        auto angle = rad * i;
        auto x1 = static_cast<int>(x + r * cos(angle));
        auto y1 = static_cast<int>(y + r * sin(angle));
        gfx->draw_line(x, y, x1, y1, 1);
    }

    gr.position(0, font->y_advance);
    grs << global_app_state.min_hall_value_;

    gr.position(0, font->y_advance * 2);
    grs << global_app_state.max_hall_value_;
}

void dialog_screen::process_event(event &e, gui &g) {
    switch (e.type) {
    case event_type::encoder_press:
        g.current_screen(&g.menu_screen_);
        break;
    default:
        break;
    }
}

void sensor_screen::render(gui &g) {
    auto font = get_font();
    auto gfx = graphics::from_memory(screen.buffer.canvas);
    glyph_renderer gr(gfx, font);
    gr_stream grs(gr);

    gfx->clear(0);
    gr.color(1);
    gr.position(0, font->y_advance);
    gr.draw_progmem(text_hall);
    gr.position(92, font->y_advance);
    grs << hall_channel::read();

    gr.position(0, font->y_advance * 2);
    gr.draw_progmem(text_ir_sensor);
    gr.position(92, font->y_advance * 2);
    grs << ir_channel::read();

    send_command(command_type::request_render_screen_500ms);
}

void settings_screen::back(gui &g, abstract_screen &) {
    g.current_screen(&g.menu_screen_);
}

void settings_screen::ir_settings(gui &g, abstract_screen &) {
    g.current_screen(&g.ir_settings_screen_);
}

void settings_screen::sector_settings(gui &g, abstract_screen &) {
    g.current_screen(&g.sector_settings_screen_);
}

settings_screen::settings_screen()
    : menu_item_back(text_back, back)
    , menu_item_ir(text_ir_sensor, ir_settings)
    , menu_item_sector(text_sector, sector_settings) {
    menu_item_back.next = &menu_item_ir;
    menu_item_ir.next = &menu_item_sector;
    create_back_trace(&menu_item_back);

    current_ = &menu_item_back;
}

void menu_screen::process_event(event &e, gui &g) {
    switch (e.type) {
    case event_type::encoder_cw:
        next_item(g);
        break;
    case event_type::encoder_ccw:
        prev_item(g);
        break;
    case event_type::encoder_press:
        exec_item(g);
        break;
    default:
        break;
    }
}

void menu_screen::render(gui &g) {
    render_menu_items(current_);
}

void menu_screen::next_item(gui &g) {
    if (current_->next != nullptr) {
        current_ = current_->next;
        send_command(command_type::render_screen);
    }
}

void menu_screen::prev_item(gui &g) {
    if (current_->prev != nullptr) {
        current_ = current_->prev;
        send_command(command_type::render_screen);
    }
}

void menu_screen::exec_item(gui &g) {
    if (current_->action != nullptr) {
        current_->action(g, *this);
    }
}

void sector_settings_screen::process_event(event &e, gui &gui) {
    switch (e.type) {
    case event_type::encoder_cw:
        menu_item_index++;
        break;
    case event_type::encoder_ccw:
        menu_item_index--;
        break;
    case event_type::encoder_press: {
        auto current = gui.current_screen();
        switch (menu_item_index) {
        case 0:
            gui.current_screen(&gui.settings_screen_);
            break;
        case 1:
            gui.input_int_screen_.value = global_app_state.settings.sector_a_hall_value;
            gui.input_int_screen_.min = 0;
            gui.input_int_screen_.max = 1023;
            gui.input_int_screen_.title_progmem = text_sector_a;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui, current](int v) {
                global_app_state.settings.sector_a_hall_value = v;
                global_app_state.save_settings();

                gui.current_screen(current);
            };
            gui.current_screen(&gui.input_int_screen_);
            break;
        case 2:
            gui.input_int_screen_.value = global_app_state.settings.sector_b_hall_value;
            gui.input_int_screen_.min = 0;
            gui.input_int_screen_.max = 1023;
            gui.input_int_screen_.title_progmem = text_sector_b;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui, current](int v) {
                global_app_state.settings.sector_b_hall_value = v;
                global_app_state.save_settings();

                gui.current_screen(current);
            };
            gui.current_screen(&gui.input_int_screen_);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    menu_item_index = ensure_range(menu_item_index, 0, 2);
    send_command(command_type::request_render_screen);
}

void sector_settings_screen::render(gui &g) {
    auto font = get_font();
    auto gfx = graphics::from_memory(screen.buffer.canvas);
    glyph_renderer gr(gfx, font);
    gr_stream grs(gr);

    gfx->clear(0);
    gr.color(1);
    gr.position(menu_item_title_offset, font->y_advance);
    gr.draw_progmem(text_back);
    if (menu_item_index == 0) {
        gr.position(0, font->y_advance * 1).draw(">");
    }

    gr.position(menu_item_title_offset, font->y_advance * 2);
    gr.draw_progmem(text_sector_a);
    gr.position(menu_item_value_offset, font->y_advance * 2);
    grs << global_app_state.settings.sector_a_hall_value;
    if (menu_item_index == 1) {
        gr.position(0, font->y_advance * 2).draw(">");
    }

    gr.position(menu_item_title_offset, font->y_advance * 3);
    gr.draw_progmem(text_sector_b);
    gr.position(menu_item_value_offset, font->y_advance * 3);
    grs << global_app_state.settings.sector_b_hall_value;
    if (menu_item_index == 2) {
        gr.position(0, font->y_advance * 3).draw(">");
    }
}
