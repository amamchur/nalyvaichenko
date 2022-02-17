#ifndef NALYVAICHENKO_SECTOR_DETECTOR_HPP
#define NALYVAICHENKO_SECTOR_DETECTOR_HPP

enum class sector_state { unknown, entering_sector, sector, leaving_sector };
enum class edge_state { idle, rising, falling };
enum class detection_result { skipped, handled, changed };

class sector_detector {
public:
    detection_result handle(int value);

    sector_state sector_state_{sector_state::unknown};
    edge_state edge_state_{edge_state::idle};

    int prev_value{0};
    int rising_threshold{2800};
    int falling_threshold{2400};
};

#endif
