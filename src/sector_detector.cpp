#include "sector_detector.hpp"

#include <cmath>
#include <cstdlib>

detection_result sector_detector::handle(int value) {
    const int threshold = 100;
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
