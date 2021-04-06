//
// Created by andrii on 06.04.21.
//

#include "segment_detector.hpp"

#include <stdlib.h>

bool segment_detector::handle(int value) {
    int d = abs(value - prev_value);
    if (d < 10) {
        return false;
    }

    prev_value = value;

    function_state st = function_state::none;
    d = abs(value - min_value);
    if (d < threshold) {
        st = function_state::sector_a;
    }

    d = abs(value - max_value);
    if (d < threshold) {
        st = function_state::sector_b;
    }

    bool changed = st != state && st != function_state::none;
    if (changed) {
        state = st;
    }
    return changed;
}
