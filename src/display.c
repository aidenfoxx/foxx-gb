#include "display.h"

static void displayScanline(Display*, MMU*);
static int displayGetColor(uint16_t, uint8_t);

void displaySetRenderCallback(Display *display, RenderCallback callback)
{
	display->render = callback;
}

void displaySetDrawCallback(Display *display, DrawCallback callback)
{
	display->draw = callback;
}

// TODO: Verify this logic.
void displayStep(Display *display, MMU *mmu, uint8_t cycles)
{
	display->cycles += cycles;

	switch (display->mode) {
		case DISPLAY_HBLANK:
			if (display->cycles >= 204) {
				if (display->scanline == 143) {
					/**
					 * Set vblank interrupt flag
					 */
					if (mmuReadByte(mmu, 0xFFFF) & 0x1) {
						mmuWriteByte(mmu, 0xFF0F, mmuReadByte(mmu, 0xFF0F) | 0x1);
					}

					/**
					 * Handle OAM DMA copy request
					 */
					uint8_t dma = mmuReadByte(mmu, 0xff46);

					if (dma) {
						for (int i = 0; i < 160; i++) {
							mmuWriteByte(mmu, 0xFE00 + i, mmuReadByte(mmu, (dma * 256) + i));
						}
						mmuWriteByte(mmu, 0xff46, 0);
					}

					if (display->draw) {
						display->draw();
					}

					display->mode = DISPLAY_VBLANK;
				} else {
					display->mode = DISPLAY_OAM;
				}

				/**
				 *  Write current line to IO
				 */
				mmuWriteByte(mmu, 0xFF44, ++display->scanline);
				display->cycles -= 204;
			}
			break;

		case DISPLAY_VBLANK:
			if (display->cycles >= 456) {
				display->scanline++;

				if (display->scanline == 153) {
					display->mode = DISPLAY_OAM;
					display->scanline = 0;
				}

				/**
				 *  Write current line to IO
				 */
				mmuWriteByte(mmu, 0xFF44, display->scanline);
				display->cycles -= 456;
			}
			break;

		case DISPLAY_OAM:
			if (display->cycles >= 80) {
				display->mode = DISPLAY_VRAM;
				display->cycles -= 80;
			}
			break;

		case DISPLAY_VRAM:
			if (display->cycles >= 172) {
				if (display->render) {
					displayScanline(display, mmu);
				}

				display->mode = DISPLAY_HBLANK;
				display->cycles -= 172;
			}
			break;
	}
}

void displayScanline(Display *display, MMU *mmu)
{
	uint8_t lcdc = mmuReadByte(mmu, 0xFF40);

	/**
	 * BG layer
	 */
	if (lcdc & 0x1) {
		uint16_t mapAddress = lcdc & 0x8 ? 0x9C00 : 0x9800;
		uint16_t dataAddress = lcdc & 0x10 ? 0x8000 : 0x8800;

		uint8_t scrollX = mmuReadByte(mmu, 0xFF43); // 0xFF43 = Scroll position X
		uint8_t scrollY = mmuReadByte(mmu, 0xFF42); // 0xFF42 = Scroll position Y

		uint8_t tileY = scrollY + display->scanline;
		uint8_t tileRow = tileY % 8;

		for (int x = 0; x < 160; x++) {
			uint8_t tileX = scrollX + x;
			uint8_t tileColumn = tileX % 8;
			uint8_t tileIndex = mmuReadByte(mmu, mapAddress + (tileX / 8) + (tileY / 8) * 32);
			tileIndex += dataAddress == 0x8800 ? 128 : 0; // Set signed addressing

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
		uint8_t spriteHeight = lcdc & 0x4 ? 0x10 : 0x8;

		/**
		 * Scan backwards for priority
		 */
		for (int i = 0; i < 40; i++) {
			/**
			 * 4 Bytes per OAM record
			 */
			uint8_t spriteOffset = i * 4;
			uint8_t spriteAddress = mmuReadByte(mmu, oamAddress + spriteOffset + 0x2);
			uint8_t spriteData = mmuReadByte(mmu, oamAddress + spriteOffset +0x3);

			uint8_t posX = mmuReadByte(mmu, oamAddress + spriteOffset + 0x1) - 0x8;
			uint8_t posY = mmuReadByte(mmu, oamAddress + spriteOffset) - 0x10;

			// TODO: Early return.
			if (display->scanline >= posY && display->scanline < (posY + spriteHeight)) {
				uint8_t spriteY = spriteData & 0x40 ? spriteHeight - (display->scanline - posY) : display->scanline - posY; /* Flip Y */

				/**
				 * 16 Bytes in a Sprite
				 * 2 Bytes in a Sprite Line
				 */
				uint16_t spriteLine = mmuReadWord(mmu, 0x8000 + (spriteAddress * 16) + (spriteY * 2));

				for (int x = 0; x < 8; x++) {
					uint8_t pixelIndex = spriteData & 0x20 ? 0x01 << x : 0x01 << (7 - x); /* Flip X */
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
		uint16_t tilemapAddress = lcdc & 0x40 ? 0x9C00 : 0x9800;
		uint16_t tiledataAddress = lcdc & 0x10 ? 0x8000 : 0x8800;

		uint8_t scrollX = mmuReadByte(mmu, 0xFF43);
		uint8_t scrollY = mmuReadByte(mmu, 0xFF42);

		uint8_t mapOffsetX = scrollX / 8;
		uint8_t mapOffsetY = (scrollY + display->scanline) / 8;

		uint8_t tileX = scrollX % 8;
		uint8_t tileY = (display->scanline + scrollY) % 8;

		uint8_t tileAddress = 0x0;
		uint16_t tileLine = 0;

		for (int x = 0; x < 160; x++) {
			if (!x || tileX == 8) {
				if (x > 0) {
					tileX = 0;
					mapOffsetX++;
				}

				/**
				 * 16 Bytes in a Tile
				 * 2 Bytes in a Tile Line
				 */
				tileAddress = mmuReadByte(mmu, tilemapAddress + mapOffsetX + (mapOffsetY * 32));
				tileAddress += tiledataAddress == 0x8800 ? 0x80 : 0x0; /* Signed check */
				tileLine = mmuReadWord(mmu, tiledataAddress + (tileAddress * 16) + (tileY * 2));
			}

			display->render(x, display->scanline, displayGetColor(tileLine, 1 << (7 - tileX)));
			tileX++;
		}
	}
}

int displayGetColor(uint16_t tileLine, uint8_t colorBit)
{
	return ((tileLine & 0xFF) & colorBit ? 1 : 0) + ((tileLine >> 8) & colorBit ? 2 : 0);
}
