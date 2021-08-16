#include "app_state.hpp"
#include "message.hpp"

app_state global_app_state;

void app_state::load_settings() {
    ::load_settings(settings);
    send_event(event_type::settings_changed);
}

void app_state::save_settings() {
    ::save_settings(settings);
    send_event(event_type::settings_changed);
}
