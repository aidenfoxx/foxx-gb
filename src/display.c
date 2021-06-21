#include "display.h"

static void displayWriteLy(Display*, MMU*);
static void displayWriteMode(Display*, MMU*);
static void displayRender(Display*, MMU*);
static int displayGetColor(uint16_t, uint8_t);

void displaySetRenderCallback(Display *display, RenderCallback callback)
{
	display->render = callback;
}

void displaySetDrawCallback(Display *display, DrawCallback callback)
{
	display->draw = callback;
}

void displayStep(Display *display, MMU *mmu, uint8_t cycles)
{
	display->cycles += cycles;

	switch (display->mode) {
		case DISPLAY_HBLANK:
			if (display->cycles >= 204) {
				display->cycles -= 204;
				display->mode = DISPLAY_OAM;
				display->scanline++;

				/**
				 * Handle DMA request
				 */
				uint8_t dma = mmuReadByte(mmu, 0xff46);

				if (dma) {
					for (int i = 0; i < 160; i++) {
						mmuWriteByte(mmu, 0xFE00 + i, mmuReadByte(mmu, (dma * 256) + i));
					}
					mmuWriteByte(mmu, 0xff46, 0);
				}

				if (display->scanline == 144) {
					display->mode = DISPLAY_VBLANK;

					/**
					 * Set vblank interrupt flag
					 */
					if (mmuReadByte(mmu, 0xFFFF) & 0x1) {
						mmuWriteByte(mmu, 0xFF0F, mmuReadByte(mmu, 0xFF0F) | 0x1);
					}

					if (display->draw) {
						display->draw();
					}
				}

				displayWriteLy(display, mmu);
				displayWriteMode(display, mmu);
			}
			break;

		case DISPLAY_VBLANK:
			if (display->cycles >= 456) {
				display->cycles -= 456;
				display->scanline++;

				if (display->scanline == 154) {
					display->mode = DISPLAY_OAM;
					display->scanline = 0;

					displayWriteMode(display, mmu);
				}

				displayWriteLy(display, mmu);
			}
			break;

		case DISPLAY_OAM:
			if (display->cycles >= 80) {
				display->cycles -= 80;
				display->mode = DISPLAY_VRAM;
			}
			break;

		case DISPLAY_VRAM:
			if (display->cycles >= 172) {
				display->cycles -= 172;
				display->mode = DISPLAY_HBLANK;

				if (display->render) {
					displayRender(display, mmu);
				}

				displayWriteMode(display, mmu);
			}
			break;
	}
}

/* https://github.com/drhelius/Gearboy/blob/master/src/Video.cpp https://gbdev.io/pandocs/STAT.html */
void displayWriteLy(Display *display, MMU *mmu)
{
	uint8_t lycFlag = display->scanline == mmuReadByte(mmu, 0xFF45);

	/**
	* Set stat interrupt flag
	*/
	if (lycFlag && mmuReadByte(mmu, 0xFF41) & 0x40 && mmuReadByte(mmu, 0xFFFF) & 0x2) {
		mmuWriteByte(mmu, 0xFF0F, mmuReadByte(mmu, 0xFF0F) | 0x2);
	}

	mmuWriteByte(mmu, 0xFF44, display->scanline); /* LY */
	mmuWriteByte(mmu, 0xFF41, (mmuReadByte(mmu, 0xFF41) & 0xFB) | (lycFlag << 2)); /* STAT */
}

void displayWriteMode(Display *display, MMU *mmu)
{
	/**
	* Set stat interrupt flag
	*/
	if (mmuReadByte(mmu, 0xFF41) & (0x8 << display->mode) && mmuReadByte(mmu, 0xFFFF) & 0x2) {
		mmuWriteByte(mmu, 0xFF0F, mmuReadByte(mmu, 0xFF0F) | 0x2);
	}

	mmuWriteByte(mmu, 0xFF41, (mmuReadByte(mmu, 0xFF41) & 0xFC) | display->mode); /* STAT */
}

void displayRender(Display *display, MMU *mmu)
{
	uint8_t lcdc = mmuReadByte(mmu, 0xFF40);

	/**
	 * Display disbled.
	 */
	if (!(lcdc & 0x80)) {
		return;
	}

	/**
	 * BG layer
	 */
	if (lcdc & 0x1) {
		uint16_t mapAddress = lcdc & 0x8 ? 0x9C00 : 0x9800;
		uint16_t dataAddress = lcdc & 0x10 ? 0x8000 : 0x8800;

		uint8_t scrollX = mmuReadByte(mmu, 0xFF43); /* 0xFF43 = Scroll position X */
		uint8_t scrollY = mmuReadByte(mmu, 0xFF42); /* 0xFF42 = Scroll position Y */

		uint8_t tileY = scrollY + display->scanline;
		uint8_t tileRow = tileY % 8;

		for (int x = 0; x < 160; x++) {
			uint8_t tileX = scrollX + x;
			uint8_t tileColumn = tileX % 8;
			uint8_t tileIndex = mmuReadByte(mmu, mapAddress + (tileX / 8) + (tileY / 8) * 32);
			tileIndex += dataAddress == 0x8800 ? 128 : 0; /* Set signed addressing */

			/**
			 * 16 Bytes in a tile
			 * 2 Bytes in a line
			 */
			uint16_t tileLine = mmuReadWord(mmu, dataAddress + (tileIndex * 16) + (tileRow * 2));

			display->render(x, display->scanline, displayGetColor(tileLine, 1 << (7 - tileColumn)));
		}
	}

	/**
	 * Sprite layer
	 */
	if (lcdc & 0x2) {
		uint16_t oamAddress = 0xFE00;
		uint8_t spriteHeight = lcdc & 0x4 ? 16 : 8;

		for (int i = 0; i < 40; i++) {
			/**
			 * 4 Bytes per OAM record
			 */
			uint8_t spriteOffset = i * 4;
			uint8_t spriteAddress = mmuReadByte(mmu, oamAddress + spriteOffset + 2);
			uint8_t spriteData = mmuReadByte(mmu, oamAddress + spriteOffset + 3);

			uint8_t posX = mmuReadByte(mmu, oamAddress + spriteOffset + 1) - 8;
			uint8_t posY = mmuReadByte(mmu, oamAddress + spriteOffset) - 16;

			// TODO: Early return.
			if (display->scanline >= posY && display->scanline < (posY + spriteHeight)) {
				uint8_t spriteY = spriteData & 0x40 ? spriteHeight - (display->scanline - posY) : display->scanline - posY; /* Flip Y */

				/**
				 * 16 Bytes in a sprite
				 * 2 Bytes in a line
				 */
				uint16_t spriteLine = mmuReadWord(mmu, 0x8000 + (spriteAddress * 16) + (spriteY * 2));

				for (int x = 0; x < 8; x++) {
					uint8_t pixelIndex = spriteData & 0x20 ? 1 << x : 1 << (7 - x); /* Flip X */
					int pixelColor = displayGetColor(spriteLine, pixelIndex);

					if (posX + x > 0 && posX + x < 160 && pixelColor > 0) {
						display->render(posX + x, display->scanline, pixelColor);
					}
				}
			}
		}
	}

	/**
	 * Window layer
	 * TODO: This uses the same logic as BG.
	 */
	if (lcdc & 0x1 && lcdc & 0x20) {
		uint16_t mapAddress = lcdc & 0x40 ? 0x9C00 : 0x9800;
		uint16_t dataAddress = lcdc & 0x10 ? 0x8000 : 0x8800;

		uint8_t windowX = mmuReadByte(mmu, 0xFF4B); /* 0xFF4B = Window position X */
		uint8_t windowY = mmuReadByte(mmu, 0xFF4A); /* 0xFF4A = Window position Y */

		uint8_t tileY = display->scanline - windowY;
		uint8_t tileRow = tileY % 8;

		for (int x = 0; x < 160; x++) {
			uint8_t tileX = x - windowX;
			uint8_t tileColumn = tileX % 8;
			uint8_t tileIndex = mmuReadByte(mmu, mapAddress + (tileX / 8) + (tileY / 8) * 32);
			tileIndex += dataAddress == 0x8800 ? 128 : 0; /* Set signed addressing */

			/**
			 * 16 Bytes in a tile
			 * 2 Bytes in a line
			 */
			uint16_t tileLine = mmuReadWord(mmu, dataAddress + (tileIndex * 16) + (tileRow * 2));

			display->render(x, display->scanline, displayGetColor(tileLine, 1 << (7 - tileColumn)));
		}
	}
}

int displayGetColor(uint16_t tileLine, uint8_t colorBit)
{
	return ((tileLine & 0xFF) & colorBit ? 1 : 0) + ((tileLine >> 8) & colorBit ? 2 : 0);
}
