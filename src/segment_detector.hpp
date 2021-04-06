//
// Created by andrii on 06.04.21.
//

#ifndef NALYVAICHENKO_SEGMENT_DETECTOR_HPP
#define NALYVAICHENKO_SEGMENT_DETECTOR_HPP

enum class function_state {
    none,
    sector_a,
    sector_b
};

class segment_detector {
public:
    bool handle(int value);

    function_state state{function_state::none};
    int prev_value{0};
    int min_value{170};
    int max_value{550};
    int threshold{30};
};


#endif //NALYVAICHENKO_SEGMENT_DETECTOR_HPP
