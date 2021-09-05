#ifndef NALYVAICHENKO_GUI_HPP
#define NALYVAICHENKO_GUI_HPP

#include "app_state.hpp"
#include "hardware.hpp"
#include "message.hpp"

#include <avr/pgmspace.h>
#include <zoal/gfx/glyph_renderer.hpp>

class gui;
class abstract_screen;

class menu_item {
public:
    explicit menu_item(const wchar_t *t);
    menu_item(const wchar_t *t, void (*a)(gui &g, abstract_screen &parent));

    const wchar_t *text{nullptr};
    void (*action)(gui &gui, abstract_screen &parent){nullptr};
    menu_item *prev{nullptr};
    menu_item *next{nullptr};
};

class abstract_screen {
public:
    virtual void process_event(event &e, gui &g) = 0;
    virtual void render(gui &g) = 0;
};

class menu_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;

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
    static void stop_action(gui &, abstract_screen &parent);

    static void go_action(gui &, abstract_screen &parent);

    static void calibrate_action(gui &, abstract_screen &parent);

    static void settings_action(gui &g, abstract_screen &parent);

    static void next_segment_action(gui &, abstract_screen &parent);

    static void logo_action(gui &gui, abstract_screen &parent);

    static void pump(gui &, abstract_screen &parent);

    static void sensors(gui &g, abstract_screen &);

    menu_item menu_item_stop;
    menu_item menu_item_go;
    menu_item menu_item_calibrate;
    menu_item menu_item_settings;
    menu_item menu_item_next;
    menu_item menu_item_logo;
    menu_item menu_item_pump;
    menu_item menu_item_sensors;
};

class input_int_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;

    const wchar_t *title_progmem{nullptr};
    const wchar_t *suffix_progmem{nullptr};
    int value{0};
    int min{0};
    int max{0};
    zoal::func::function<16, void, int> callback;
};

class settings_screen : public menu_screen {
public:
    settings_screen();
private:
    static void back(gui &g, abstract_screen &);
    static void portion_settings(gui &g, abstract_screen &);
    static void ir_settings(gui &g, abstract_screen &);
    static void adjustment_settings(gui &g, abstract_screen &);
    static void sector_settings(gui &g, abstract_screen &);

    menu_item menu_item_back;
    menu_item menu_item_portion;
    menu_item menu_item_ir;
    menu_item menu_item_adjust;
    menu_item menu_item_sector;
};

class portion_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    int menu_item_index{0};
};

class ir_settings_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    int menu_item_index{0};
};

class sector_settings_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    int menu_item_index{0};
};

class adjustment_settings_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
    void render(gui &g) override;
    int menu_item_index{0};
};

class dialog_screen : public abstract_screen {
public:
    void process_event(event &e, gui &g) override;
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

class gui : public event_handler {
public:
    void render();
    void process_event(event &e) override;

    input_int_screen input_int_screen_;
    main_screen menu_screen_;

    settings_screen settings_screen_;
    ir_settings_screen ir_settings_screen_;
    sector_settings_screen sector_settings_screen_;
    adjustment_settings_screen adjustment_settings_screen_;
    portion_screen portion_screen_;

    logo_screen logo_screen_;
    calibration_screen calibration_screen_;
    sensor_screen sensor_screen_;

    void current_screen(abstract_screen *scr);
    inline abstract_screen *current_screen() const {
        return current_screen_;
    }

private:
    abstract_screen *current_screen_{&menu_screen_};
};

#endif //NALYVAICHENKO_GUI_HPP
