#include "cartridge.h"

int cartridgeInit(Cartridge *cartridge, char path[])
{
	FILE *file = fopen(path, "rb");

	if (!file) {
		return -1;
	}

	fseek(file, 0, SEEK_END);
	unsigned length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buffer = calloc(1, length + 1);

	if (length != fread(buffer, sizeof(char), length, file)) {
		free(buffer);
		return -2;
	}

	fclose(file);

	memcpy(cartridge->rom0, buffer + CART_ROM0_OFFSET, sizeof(cartridge->rom0));
	memcpy(&cartridge->id, buffer + CART_ID_OFFSET, sizeof(cartridge->id));
	memcpy(&cartridge->name, buffer + CART_NAME_OFFSET, sizeof(cartridge->name));

	if (length > 32767) {
		memcpy(cartridge->rom1, buffer + CART_ROM1_OFFSET, sizeof(cartridge->rom1));
	}

	free(buffer);
	return 1;
}
