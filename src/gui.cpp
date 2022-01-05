#include "gui.hpp"

#include "./flash_manager.hpp"
#include "./fonts/roboto_regular_16.hpp"
#include "./hardware.hpp"

#include <cmath>
#include <cstdio>
#include <ctime>
#include <zoal/gfx/glyph_renderer.hpp>
#include <zoal/gfx/renderer.hpp>
#include <zoal/io/output_stream.hpp>

#if defined(STM32F401xC)
#include "adc.h"
#else

#endif

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

constexpr int menu_item_title_offset = 10;
constexpr int menu_item_details_offset = 80;
constexpr int menu_item_value_offset = 80;

static const wchar_t text_time[] = L"Час";
static const wchar_t text_test[] = L"Тест";
static const wchar_t text_steps[] = L"Кроків";
static const wchar_t text_max[] = L"Макс.";
static const wchar_t text_min[] = L"Мін.";
static const wchar_t text_pause[] = L"Пауза";
static const wchar_t text_sector[] = L"Сектор";
static const wchar_t text_sector_a[] = L"Поріг R";
static const wchar_t text_sector_b[] = L"Поріг F";
static const wchar_t text_ir_sensor[] = L"ІЧС";
static const wchar_t text_adjustment[] = L"Корекція";
static const wchar_t text_hall[] = L"Холл";
static const wchar_t text_back[] = L"Назад";
static const wchar_t text_portion[] = L"Порція";
static const wchar_t text_portions[] = L"Порції";
static const wchar_t text_power[] = L"Потужність";
static const wchar_t suffix_ms[] = L"ms";
static const wchar_t suffix_mg[] = L"mg";
static const wchar_t text_sensors[] = L"Сенсори";
static const wchar_t text_next_segment[] = L"Сегмент++";
static const wchar_t text_weight[] = L"Вага";
static const wchar_t text_stop[] = L"Стоп!";
static const wchar_t text_start[] = L"Запуск";
static const wchar_t text_calibrate[] = L"Калібрування";
static const wchar_t text_config[] = L"Налаштув.";
static const wchar_t text_logo[] = L"Лого";
static const wchar_t text_pump[] = L"Прокачка";

gui user_interface;

namespace zoal { namespace utils {
    class mem_reader {
    public:
        template<class T>
        static T read_mem(const void *ptr) {
            return *reinterpret_cast<const T *>(ptr);
        }
    };
}}

class glyph_renderer : public zoal::gfx::glyph_renderer<graphics, zoal::utils::mem_reader> {
public:
    glyph_renderer(graphics *g, const zoal::text::font *font)
        : zoal::gfx::glyph_renderer<graphics, zoal::utils::mem_reader>(g, font) {}

    void draw_progmem(const wchar_t *ptr) {
        auto v = *ptr++;
        while (v != 0) {
            draw((wchar_t)v);
            v = *ptr++;
        }
    }
};

using gr_stream = zoal::io::output_stream<glyph_renderer>;

menu_item::menu_item(const wchar_t *t)
    : text(t) {
    details[0] = '\0';
}

menu_item::menu_item(const wchar_t *t, void (*a)(gui &, menu_item &parent))
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

    if (*active->details) {
        gr.position(menu_item_details_offset, y);
        gr.draw(active->details);
    }

    auto item = active->next;
    y = origin_y + font->y_advance;
    while (y <= 80 && item != nullptr) {
        gr.position(menu_item_title_offset, y);
        gr.draw_progmem(item->text);
        if (*item->details) {
            gr.position(menu_item_details_offset, y);
            gr.draw(item->details);
        }

        y += font->y_advance;
        item = item->next;
    }

    item = active->prev;
    y = origin_y - font->y_advance;
    while (y > 0 && item != nullptr) {
        gr.position(menu_item_title_offset, y);
        gr.draw_progmem(item->text);
        if (*item->details) {
            gr.position(menu_item_details_offset, y);
            gr.draw(item->details);
        }
        y -= font->y_advance;
        item = item->prev;
    }
}

void gui::render() {
    if (current_screen_ != nullptr) {
        current_screen_->render(*this);
    }
}

void gui::process_event(event &e) {
    if (current_screen_ != nullptr) {
        current_screen_->process_event(e, *this);
    }
}

gui::gui() noexcept = default;

void gui::push_screen(abstract_screen *scr) {
    screen_stack[stack_size++] = scr;
    current_screen_ = scr;
    current_screen_->activate(*this);
    send_command(command_type::request_render_screen);
}

abstract_screen *gui::pop_screen() {
    abstract_screen *scr = screen_stack[stack_size - 1];
    if (stack_size > 0) {
        stack_size--;
        current_screen_ = screen_stack[stack_size - 1];
        current_screen_->activate(*this);
        send_command(command_type::request_render_screen);
    }

    return scr;
}

main_screen::main_screen()
    : menu_item_stop(text_stop, stop_action)
    , menu_item_go(text_start, go_action)
    , menu_item_settings(text_config, settings_action)
    , menu_item_logo(text_logo, logo_action)
    , menu_item_pump(text_pump, pump_liquid)
    , menu_item_portions(text_portions, portions) {
    menu_item_logo.next = &menu_item_go;
    menu_item_go.next = &menu_item_stop;
    menu_item_stop.next = &menu_item_portions;
    menu_item_portions.next = &menu_item_pump;
    menu_item_pump.next = &menu_item_settings;

    create_back_trace(&menu_item_logo);
    current_ = &menu_item_logo;
}

void main_screen::go_action(gui &, menu_item &) {
    send_command(command_type::go);
}

void main_screen::logo_action(gui &gui, menu_item &) {
    send_command(command_type::logo);
}

void main_screen::pump_liquid(gui &, menu_item &) {
    send_command(command_type::pump, 500);
}

void main_screen::settings_action(gui &g, menu_item &parent) {
    g.push_screen(&g.settings_screen_);
}

void main_screen::stop_action(gui &, menu_item &parent) {
    send_command(command_type::stop);
}

void main_screen::portions(gui &g, menu_item &parent) {
    g.push_screen(&g.portions_screen_);
}

void main_screen::activate(gui &g) {
    const auto &s = global_app_state.settings;
    const auto &p = s.portion_settings_[s.current_portion_];
    sprintf(menu_item_go.details, "%dmg", p.mg_);
}

void ir_settings_screen::process_event(event &e, gui &gui) {
    auto segments = global_app_state.settings.segments_;
    auto &rs = global_app_state.settings.revolver_settings_[segments];
    switch (e.type) {
    case event_type::encoder_cw:
        menu_item_index++;
        break;
    case event_type::encoder_ccw:
        menu_item_index--;
        break;
    case event_type::encoder_press: {
        switch (menu_item_index) {
        case 0:
            gui.push_screen(&gui.settings_screen_);
            break;
        case 1:
            gui.input_int_screen_.value = rs.ir_max_value_;
            gui.input_int_screen_.min = 0;
            gui.input_int_screen_.max = 1023;
            gui.input_int_screen_.title_progmem = text_max;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui, &rs](int v) {
                rs.ir_max_value_ = v;
                global_app_state.save_settings();

                gui.pop_screen();
            };
            gui.push_screen(&gui.input_int_screen_);
            break;
        case 2:
            gui.input_int_screen_.value = rs.ir_min_value_;
            gui.input_int_screen_.min = 0;
            gui.input_int_screen_.max = 1023;
            gui.input_int_screen_.title_progmem = text_max;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui, &rs](int v) {
                rs.ir_min_value_ = v;
                global_app_state.save_settings();

                gui.pop_screen();
            };
            gui.push_screen(&gui.input_int_screen_);
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
    auto segments = global_app_state.settings.segments_;
    auto &rs = global_app_state.settings.revolver_settings_[segments];
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
    grs << rs.ir_max_value_;
    if (menu_item_index == 1) {
        gr.position(0, font->y_advance * 2).draw(">");
    }

    gr.position(menu_item_title_offset, font->y_advance * 3);
    gr.draw_progmem(text_min);
    gr.position(menu_item_value_offset, font->y_advance * 3);
    grs << rs.ir_min_value_;
    if (menu_item_index == 2) {
        gr.position(0, font->y_advance * 3).draw(">");
    }
}

void ir_settings_screen::activate(gui &g) {}

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
        gr.draw_progmem(suffix_progmem);
    }
}

void input_int_screen::activate(gui &g) {}

void logo_screen::render(gui &g) {
    fm.read_frame(flash_resource::logo, 0, screen.buffer.canvas, sizeof(screen.buffer.canvas));
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

    auto count = global_app_state.settings.segments_;
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
        g.pop_screen();
        break;
    default:
        break;
    }
}

void dialog_screen::activate(gui &g) {}

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
    grs << sensors_values[0];

    gr.position(0, font->y_advance * 2);
    gr.draw_progmem(text_ir_sensor);
    gr.position(92, font->y_advance * 2);
    grs << sensors_values[1];

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sensors_values, 2);
    send_command(command_type::request_render_screen_500ms);
}

void settings_screen::back(gui &g, menu_item &) {
    g.push_screen(&g.menu_screen_);
}

void settings_screen::ir_settings(gui &g, menu_item &) {
    g.push_screen(&g.ir_settings_screen_);
}

void settings_screen::adjustment_settings(gui &g, menu_item &) {
    g.push_screen(&g.adjustment_settings_screen_);
}

void settings_screen::sector_settings(gui &g, menu_item &) {
    g.push_screen(&g.sector_settings_screen_);
}

void settings_screen::power_settings(gui &g, menu_item &) {
    g.push_screen(&g.power_screen_);
}

void settings_screen::portions_settings(gui &g, menu_item &) {
    g.push_screen(&g.portions_settings_screen_);
}

void settings_screen::sensors_setting(gui &g, menu_item &) {
    g.push_screen(&g.sensor_screen_);
}

settings_screen::settings_screen()
    : menu_item_back(text_back, back)
    , menu_item_portions(text_portions, portions_settings)
    , menu_item_power(text_power, power_settings)
    , menu_item_ir(text_ir_sensor, ir_settings)
    , menu_item_adjust(text_adjustment, adjustment_settings)
    , menu_item_sector(text_sector, sector_settings)
    , menu_item_sensors(text_sensors, sensors_setting)
    , menu_item_next(text_next_segment, next_segment_action)
    , menu_item_calibrate(text_calibrate, calibrate_action) {
    menu_item_back.next = &menu_item_portions;
    menu_item_portions.next = &menu_item_power;
    menu_item_power.next = &menu_item_ir;
    menu_item_ir.next = &menu_item_adjust;
    menu_item_adjust.next = &menu_item_sector;
    menu_item_sector.next = &menu_item_sensors;
    menu_item_sensors.next = &menu_item_next;
    menu_item_next.next = &menu_item_calibrate;
    create_back_trace(&menu_item_back);

    current_ = &menu_item_back;
}

void settings_screen::next_segment_action(gui &, menu_item &) {
    send_command(command_type::next_segment);
}

void settings_screen::calibrate_action(gui &, menu_item &) {
    send_command(command_type::calibrate);
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
        current_->action(g, *current_);
    }
}

void menu_screen::activate(gui &g) {}

void sector_settings_screen::process_event(event &e, gui &gui) {
    switch (e.type) {
    case event_type::encoder_cw:
        menu_item_index++;
        break;
    case event_type::encoder_ccw:
        menu_item_index--;
        break;
    case event_type::encoder_press: {
        switch (menu_item_index) {
        case 0:
            gui.push_screen(&gui.settings_screen_);
            break;
        case 1:
            gui.input_int_screen_.value = global_app_state.settings.hall_rising_threshold_;
            gui.input_int_screen_.min = 0;
            gui.input_int_screen_.max = 4095;
            gui.input_int_screen_.title_progmem = text_sector_a;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui](int v) {
                global_app_state.settings.hall_rising_threshold_ = v;
                global_app_state.save_settings();

                gui.pop_screen();
            };
            gui.push_screen(&gui.input_int_screen_);
            break;
        case 2:
            gui.input_int_screen_.value = global_app_state.settings.hall_falling_threshold_;
            gui.input_int_screen_.min = 0;
            gui.input_int_screen_.max = 4095;
            gui.input_int_screen_.title_progmem = text_sector_b;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui](int v) {
                global_app_state.settings.hall_falling_threshold_ = v;
                global_app_state.save_settings();

                gui.pop_screen();
            };
            gui.push_screen(&gui.input_int_screen_);
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
    grs << global_app_state.settings.hall_rising_threshold_;
    if (menu_item_index == 1) {
        gr.position(0, font->y_advance * 2).draw(">");
    }

    gr.position(menu_item_title_offset, font->y_advance * 3);
    gr.draw_progmem(text_sector_b);
    gr.position(menu_item_value_offset, font->y_advance * 3);
    grs << global_app_state.settings.hall_falling_threshold_;
    if (menu_item_index == 2) {
        gr.position(0, font->y_advance * 3).draw(">");
    }
}

void sector_settings_screen::activate(gui &g) {}

void edit_portion_screen::activate(gui &g) {
    auto s = global_app_state.settings;
    auto &ps = s.portion_settings_[portion];
    sprintf(weight.details, "%dmg", ps.mg_);
    sprintf(time.details, "%dms", ps.time_);
}

edit_portion_screen::edit_portion_screen()
    : back(text_back, back_action)
    , weight(text_weight, edit_weight)
    , time(text_time, edit_time)
    , test(text_test, test_portion) {
    back.next = &weight;
    weight.next = &time;
    time.next = &test;

    create_back_trace(&back);
    current_ = &back;
}

void edit_portion_screen::back_action(gui &g, menu_item &) {
    g.push_screen(&g.settings_screen_);
}

void edit_portion_screen::edit_weight(gui &g, menu_item &) {
    auto me = g.edit_portion_screen_;
    auto &s = global_app_state.settings;
    auto &ps = s.portion_settings_[me.portion];

    g.input_int_screen_.value = ps.mg_;
    g.input_int_screen_.min = 1;
    g.input_int_screen_.max = 150;
    g.input_int_screen_.title_progmem = text_weight;
    g.input_int_screen_.suffix_progmem = suffix_mg;
    g.input_int_screen_.callback = [&g, &ps](int v) {
        ps.mg_ = v;
        global_app_state.save_settings();

        g.pop_screen();
    };
    g.push_screen(&g.input_int_screen_);
}

void edit_portion_screen::edit_time(gui &g, menu_item &) {
    auto me = g.edit_portion_screen_;
    auto &s = global_app_state.settings;
    auto &ps = s.portion_settings_[me.portion];

    g.input_int_screen_.value = ps.time_;
    g.input_int_screen_.min = 1;
    g.input_int_screen_.max = 3000;
    g.input_int_screen_.title_progmem = text_time;
    g.input_int_screen_.suffix_progmem = suffix_ms;
    g.input_int_screen_.callback = [&g, &ps](int v) {
        ps.time_ = v;
        global_app_state.save_settings();
        g.pop_screen();
    };
    g.push_screen(&g.input_int_screen_);
}

void edit_portion_screen::test_portion(gui &g, menu_item &) {
    auto me = g.edit_portion_screen_;
    auto &s = global_app_state.settings;
    auto &ps = s.portion_settings_[me.portion];

    command cmd{};
    cmd.type = command_type::pump;
    cmd.value = ps.time_;
    send_command(cmd);
}

void adjustment_settings_screen::process_event(event &e, gui &gui) {
    auto segments = global_app_state.settings.segments_;
    auto &rs = global_app_state.settings.revolver_settings_[segments];
    switch (e.type) {
    case event_type::encoder_cw:
        menu_item_index++;
        break;
    case event_type::encoder_ccw:
        menu_item_index--;
        break;
    case event_type::encoder_press: {
        switch (menu_item_index) {
        case 0:
            gui.pop_screen();
            break;
        case 1:
            gui.input_int_screen_.value = rs.sector_adjustment_;
            gui.input_int_screen_.min = -50;
            gui.input_int_screen_.max = 50;
            gui.input_int_screen_.title_progmem = text_adjustment;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui, &rs](int v) {
                rs.sector_adjustment_ = v;
                global_app_state.save_settings();

                gui.pop_screen();
            };
            gui.push_screen(&gui.input_int_screen_);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    menu_item_index = ensure_range(menu_item_index, 0, 1);
    send_command(command_type::request_render_screen);
}

void adjustment_settings_screen::render(gui &g) {
    auto segments = global_app_state.settings.segments_;
    auto &rs = global_app_state.settings.revolver_settings_[segments];
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
    gr.draw_progmem(text_steps);
    gr.position(menu_item_value_offset, font->y_advance * 2);
    grs << rs.sector_adjustment_;
    if (menu_item_index == 1) {
        gr.position(0, font->y_advance * 2).draw(">");
    }
}

void adjustment_settings_screen::activate(gui &g) {}

void power_screen::process_event(event &e, gui &gui) {
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
            gui.pop_screen();
            break;
        case 1:
            gui.input_int_screen_.value = global_app_state.settings.pump_power_;
            gui.input_int_screen_.min = 40;
            gui.input_int_screen_.max = 100;
            gui.input_int_screen_.title_progmem = text_power;
            gui.input_int_screen_.suffix_progmem = nullptr;
            gui.input_int_screen_.callback = [&gui](int v) {
                global_app_state.settings.pump_power_ = v;
                global_app_state.save_settings();

                gui.pop_screen();
            };
            gui.push_screen(&gui.input_int_screen_);
            break;
        default:
            break;
        }
        break;
    }
    default:
        break;
    }

    menu_item_index = ensure_range(menu_item_index, 0, 1);
    send_command(command_type::request_render_screen);
}

void power_screen::render(gui &g) {
    auto &power = global_app_state.settings.pump_power_;
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
    grs << "P";
    gr.position(menu_item_value_offset, font->y_advance * 2);
    grs << power << "%";
    if (menu_item_index == 1) {
        gr.position(0, font->y_advance * 2).draw(">");
    }
}

void power_screen::activate(gui &g) {}

portions_screen::portions_screen()
    : back_item(text_back, back)
    , portions{//
               {text_portion, item_action},
               {text_portion, item_action},
               {text_portion, item_action}} {
    back_item.next = &portions[0];

    for (size_t i = 0; i < total_portions - 1; ++i) {
        portions[i].next = portions + i + 1;
    }

    create_back_trace(&back_item);
    current_ = &back_item;
}

void portions_screen::back(gui &g, menu_item &) {
    g.pop_screen();
}

void portions_screen::item_action(gui &g, menu_item &item) {
    auto &me = g.portions_screen_;
    auto &s = global_app_state.settings;
    int index = -1;
    for (size_t i = 0; i < total_portions; i++) {
        if (me.portions + i == &item) {
            index = static_cast<int>(i);
            break;
        }
    }

    if (index >= 0) {
        s.current_portion_ = index;
        global_app_state.save_settings();
        g.pop_screen();
    }
}

void portions_screen::activate(gui &g) {
    auto &ps = global_app_state.settings.portion_settings_;
    for (size_t i = 0; i < total_portions; i++) {
        sprintf(portions[i].details, "%dmg", ps[i].mg_);
    }
}

portions_settings_screen::portions_settings_screen()
    : back_item(text_back, back)
    , portions{//
               {text_portion, edit_portions},
               {text_portion, edit_portions},
               {text_portion, edit_portions}} {
    back_item.next = &portions[0];

    for (size_t i = 0; i < total_portions - 1; ++i) {
        portions[i].next = portions + i + 1;
    }

    create_back_trace(&back_item);
    current_ = &back_item;
}

void portions_settings_screen::back(gui &g, menu_item &) {
    g.pop_screen();
}

void portions_settings_screen::activate(gui &g) {
    auto &ps = global_app_state.settings.portion_settings_;
    for (size_t i = 0; i < total_portions; i++) {
        sprintf(portions[i].details, "%dmg", ps[i].mg_);
    }
}

void portions_settings_screen::edit_portions(gui &g, menu_item &item) {
    auto &me = g.portions_settings_screen_;
    int index = -1;
    for (size_t i = 0; i < total_portions; i++) {
        if (me.portions + i == &item) {
            index = static_cast<int>(i);
            break;
        }
    }

    if (index >= 0) {
        g.edit_portion_screen_.portion = index;
        g.push_screen(&g.edit_portion_screen_);
    }
}

void animation_screen::render(gui &g) {
    fm.read_frame(tag_, frame_, screen.buffer.canvas, sizeof(screen.buffer.canvas));
    screen.display();

    frame_++;
    if (frame_ >= record_.animation.frames) {
        frame_ = 0;
        if (repeats_ > 0) {
            repeats_--;
        }
    }

    if (repeats_ == 0) {
        if (callback) {
            callback(*this);
        } else {
            g.pop_screen();
        }
    } else {
        send_command(command_type::request_render_screen_ms, 30);
    }
}

void animation_screen::animation(uint32_t tag, int repeats) {
    this->tag_ = tag;
    this->repeats_ = repeats;
    this->frame_ = 0;
    fm.get_record(tag, record_);
}
