#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define CART_ROM0_OFFSET 0x0000
#define CART_ROM1_OFFSET 0x4000
#define CART_ID_OFFSET 0x0147
#define CART_NAME_OFFSET 0x0134

typedef struct {
    uint8_t rom0[16384];
    uint8_t rom1[16384];
    uint8_t id;
    uint16_t name;
} Cartridge;

int cartridgeInit(Cartridge*, char[]);

#endif