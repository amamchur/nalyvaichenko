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
    int sector_a_value{750};
    int sector_b_value{500};
    int threshold{50};
};


#endif //NALYVAICHENKO_SEGMENT_DETECTOR_HPP
