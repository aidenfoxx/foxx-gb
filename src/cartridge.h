#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint8_t rom[0x8000];
    uint8_t id;
    uint16_t name;
} Cartridge;

int cartridgeInit(Cartridge*, const char*);

#endif
