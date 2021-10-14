#ifndef NALYVAICHENKO_SECTOR_DETECTOR_HPP
#define NALYVAICHENKO_SECTOR_DETECTOR_HPP

enum class sector_state { unknown, sector, out_off_sector };
enum class edge_state { idle, rising, falling };
enum class detection_result { skipped, handled, changed };

class sector_detector {
public:
    detection_result handle(int value);

    sector_state sector_state_{sector_state::unknown};
    edge_state edge_state_{edge_state::idle};

    int prev_value{0};
    int noise_filter{1};
    int rising_threshold{700};
    int falling_threshold{600};
};

#endif