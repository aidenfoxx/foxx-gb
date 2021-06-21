#include "cartridge.h"

int cartridgeInit(Cartridge *cartridge, const char *path)
{
	FILE *file = fopen(path, "rb");

	if (!file) {
		return -1;
	}

	fseek(file, 0, SEEK_END);
	unsigned length = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint8_t *buffer = malloc(length);

	if (length != fread(buffer, sizeof(uint8_t), length, file)) {
		free(buffer);
		return -1;
	}

	fclose(file);

	if (length != 0x8000) {
		free(buffer);
		printf("WARN: Cartridge unsupported.");
		return -1;
	}

	memcpy(cartridge->rom, buffer, 0x8000);
	memcpy(&cartridge->id, buffer + 0x147, sizeof(uint8_t));
	memcpy(&cartridge->name, buffer + 0x134, sizeof(uint16_t));

	free(buffer);
	return 0;
}
