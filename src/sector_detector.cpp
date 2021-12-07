#include "sector_detector.hpp"

#include <cstdlib>

detection_result sector_detector::handle(int value) {
    int d = value - prev_value;
    if (abs(d) < noise_filter) {
        return detection_result::skipped;
    }

    prev_value = value;

    int dr = value - rising_threshold;
    int df = value - falling_threshold;
    sector_state st = sector_state::unknown;
    switch (edge_state_) {
    case edge_state::idle:
        if (d > 0 && dr > 0) {
            edge_state_ = edge_state::rising;
        }
        if (d < 0 && df < 0) {
            edge_state_ = edge_state::falling;
        }
        break;
    case edge_state::rising:
        if (d < 0) {
            st = sector_state::sector;
            edge_state_ = edge_state::idle;
        }
        break;
    case edge_state::falling:
        if (d > 0) {
            st = sector_state::out_off_sector;
            edge_state_ = edge_state::idle;
        }
        break;
    }

    bool changed = st != sector_state_ && st != sector_state::unknown;
    if (changed) {
        sector_state_ = st;
        edge_state_ = edge_state::idle;
        return detection_result::changed;
    }
    return detection_result::handled;
}
