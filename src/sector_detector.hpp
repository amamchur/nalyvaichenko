#ifndef NALYVAICHENKO_SECTOR_DETECTOR_HPP
#define NALYVAICHENKO_SECTOR_DETECTOR_HPP

enum class sector_state { unknown, entering_sector, sector, leaving_sector };
enum class edge_state { idle, rising, falling };
enum class detection_result { skipped, handled, changed };

class sector_detector {
public:
    detection_result handle_v2(int value);
    detection_result handle(int value);

    sector_state sector_state_{sector_state::unknown};
    edge_state edge_state_{edge_state::idle};

    int prev_value{0};
    int noise_filter{30};
    int rising_threshold{2300};
    int falling_threshold{2100};

private:
    float _err_estimate{0.8};
    float _last_estimate{0};
    float simple_kalman(float newVal);
};

#endif
