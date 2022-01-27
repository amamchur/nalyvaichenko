#ifndef NALYVAICHENKO_GUI_HPP
#define NALYVAICHENKO_GUI_HPP

#include "./app_state.hpp"
#include "./flash_struct.hpp"
#include "./message.hpp"

#include <zoal/gfx/glyph_renderer.hpp>

class gui;
class abstract_screen;

class menu_item {
public:
    explicit menu_item(const wchar_t *t);
    menu_item(const wchar_t *t, void (*a)(gui &g, menu_item &item));

    const wchar_t *text{nullptr};
    char details[9]{0};
    void (*action)(gui &gui, menu_item &parent){nullptr};
    menu_item *prev{nullptr};
    menu_item *next{nullptr};
};

class abstract_screen {
public:
    virtual void process_event(event &e, gui &g) = 0;
    virtual void render(gui &g) = 0;
    virtual void activate(gui &g) = 0;
};

class menu_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    void activate(gui &g) override;

protected:
    menu_item *current_{nullptr};

    void next_item(gui &g);
    void prev_item(gui &g);
    void exec_item(gui &g);
};

class main_screen : public menu_screen {
public:
    main_screen();

private:
    static void stop_action(gui &, menu_item &item);
    static void go_action(gui &, menu_item &item);
    static void settings_action(gui &g, menu_item &item);
    static void logo_action(gui &gui, menu_item &item);
    static void pump_liquid(gui &, menu_item &item);
    static void portions(gui &, menu_item &item);

    void activate(gui &g) override;

    menu_item menu_item_stop;
    menu_item menu_item_go;
    menu_item menu_item_settings;
    menu_item menu_item_logo;
    menu_item menu_item_pump;
    menu_item menu_item_portions;
};

class input_int_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    void activate(gui &g) override;

    const wchar_t *title_progmem{nullptr};
    const wchar_t *suffix_progmem{nullptr};
    int value{0};
    int min{0};
    int max{0};
    zoal::func::function<sizeof(void *) * 8, void, int> callback;
};

class portions_screen : public menu_screen {
public:
    portions_screen();

private:
    static void back(gui &g, menu_item &);
    static void item_action(gui &g, menu_item &);
    void activate(gui &g) override;

    menu_item back_item;
    menu_item portions[total_portions];
};

class portions_settings_screen : public menu_screen {
public:
    portions_settings_screen();

private:
    static void back(gui &g, menu_item &);
    static void edit_portions(gui &g, menu_item &);
    void activate(gui &g) override;

    menu_item back_item;
    menu_item portions[total_portions];
};

class settings_screen : public menu_screen {
public:
    settings_screen();

private:
    static void back(gui &g, menu_item &);
    static void portions_settings(gui &g, menu_item &);
    static void power_settings(gui &g, menu_item &);
    static void ir_settings(gui &g, menu_item &);
    static void adjustment_settings(gui &g, menu_item &);
    static void sector_settings(gui &g, menu_item &);
    static void sensors_setting(gui &g, menu_item &);
    static void next_segment_action(gui &, menu_item &);

    menu_item menu_item_back;
    menu_item menu_item_portions;
    menu_item menu_item_power;
    menu_item menu_item_ir;
    menu_item menu_item_adjust;
    menu_item menu_item_sector;
    menu_item menu_item_sensors;
    menu_item menu_item_next;
};

class edit_portion_screen : public menu_screen {
public:
    edit_portion_screen();

    static void back_action(gui &g, menu_item &);
    static void edit_weight(gui &g, menu_item &);
    static void edit_time(gui &g, menu_item &);
    static void test_portion(gui &g, menu_item &);

    void activate(gui &g) override;
    int portion{0};

    menu_item back;
    menu_item weight;
    menu_item time;
    menu_item test;
};

class power_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    void activate(gui &g) override;
    int menu_item_index{0};
};

class ir_settings_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    void activate(gui &g) override;
    int menu_item_index{0};
};

class sector_settings_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    void activate(gui &g) override;
    int menu_item_index{0};
};

class adjustment_settings_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    void activate(gui &g) override;
    int menu_item_index{0};
};

class dialog_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void activate(gui &g) override;
};

class logo_screen : public dialog_screen {
public:
    void render(gui &g) override;
};

class calibration_screen : public dialog_screen {
public:
    void render(gui &g) override;
};

class sensor_screen : public dialog_screen {
public:
    void render(gui &g) override;
};

class animation_screen : public dialog_screen {
public:
    void animation(uint32_t tag, int repeats = 1);
    void render(gui &g) override;

    zoal::func::function<sizeof(void *) * 8, void, animation_screen&> callback;
private:
    uint32_t tag_{0};
    uint32_t frame_{0};
    int repeats_{1};
    flash_record record_{};
};

class gui : public event_handler {
public:
    gui() noexcept;

    void render();
    void process_event(event &e) override;

    input_int_screen input_int_screen_;

    main_screen menu_screen_;
    logo_screen logo_screen_;
    calibration_screen calibration_screen_;
    sensor_screen sensor_screen_;
    power_screen power_screen_;
    portions_screen portions_screen_;

    settings_screen settings_screen_;
    ir_settings_screen ir_settings_screen_;
    sector_settings_screen sector_settings_screen_;
    adjustment_settings_screen adjustment_settings_screen_;
    edit_portion_screen edit_portion_screen_;
    portions_settings_screen portions_settings_screen_;
    animation_screen animation_screen_;

    inline abstract_screen *current_screen() const {
        return current_screen_;
    }

    void push_screen(abstract_screen *scr);
    abstract_screen *pop_screen();
private:
    int stack_size{0};
    abstract_screen *screen_stack[5]{nullptr};
    abstract_screen *current_screen_{&menu_screen_};
};

extern gui user_interface;

#endif
