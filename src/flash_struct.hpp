#ifndef NALYVAICHENKO_FLASH_STRUCT_HPP
#define NALYVAICHENKO_FLASH_STRUCT_HPP

#include <cstddef>
#include <cstdint>

constexpr uint8_t record_type_image = 0;
constexpr uint8_t record_type_animation = 1;

struct __attribute__((__packed__)) flash_header {
    uint32_t total_records;
};

struct __attribute__((__packed__)) flash_animation {
    uint32_t frames;
};

struct __attribute__((__packed__)) flash_image {
};

struct __attribute__((__packed__)) flash_record {
    uint32_t tag;
    uint32_t address;
    uint32_t size;
    uint8_t type;
    union {
        flash_animation animation;
        flash_image image;
    };
};

#endif
