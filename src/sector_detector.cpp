#include "sector_detector.hpp"

#include <cmath>
#include <cstdlib>

float sector_detector::simple_kalman(float newVal) {
    const float _err_measure = 0.8;
    const float _q = 0.1;
    float _kalman_gain, _current_estimate;
    _kalman_gain = (float)_err_estimate / (_err_estimate + _err_measure);
    _current_estimate = _last_estimate + (float)_kalman_gain * (newVal - _last_estimate);
    _err_estimate = (1.0f - _kalman_gain) * _err_estimate + fabs(_last_estimate - _current_estimate) * _q;
    _last_estimate = _current_estimate;
    return _current_estimate;
}

detection_result sector_detector::handle_v2(int value) {
    const int threshold = 50;
    value = (int)simple_kalman((float)value);
    if (edge_state_ == edge_state::idle) {
        if (value >= rising_threshold) {
            edge_state_ = edge_state::rising;
            prev_value = value;
        } else {
            prev_value = value;
        }
    }

    int d = value - prev_value;
    switch (edge_state_) {
    case edge_state::idle:
        prev_value = value;
        break;
    case edge_state::rising:
        if (d < -threshold) {
            sector_state_ = sector_state::sector;
            edge_state_ = edge_state::falling;
            prev_value = value;
            return detection_result::changed;
        }

        if (value <= falling_threshold) {
            edge_state_ = edge_state::falling;
            prev_value = value;
        }

        if (prev_value < value) {
            prev_value = value;
        }
        break;
    case edge_state::falling:
        if (value <= falling_threshold) {
            sector_state_ = sector_state::unknown;
            edge_state_ = edge_state::idle;
            return detection_result::changed;
        }

        if (prev_value > value) {
            prev_value = value;
        }
        break;
    }

    return detection_result::handled;
}
