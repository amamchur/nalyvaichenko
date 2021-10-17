#ifndef NALYVAICHENKO_PROGMEM_HPP
#define NALYVAICHENKO_PROGMEM_HPP

#ifdef __AVR_ARCH__
#include <avr/pgmspace.h>
#else
#define PROGMEM
#endif

#endif
