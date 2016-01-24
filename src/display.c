#include "display.h"

void displayInit(Display *display)
{
	display->cycles = 0x0000;
	display->mode = DISPLAY_MODE_HBLANK;
	display->scanline = 0;
}

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

	switch (display->mode)
	{
		case DISPLAY_MODE_HBLANK:
			if (display->cycles > 203)
			{
				if (display->scanline == 143)
				{
					/**
					 * Set vblank interrupt flag
					 */
					if (mmuReadByte(mmu, 0xFFFF) & 0x01)
					{
						mmuWriteByte(mmu, 0xFF0F, mmuReadByte(mmu, 0xFF0F) | 0x01);
					}
					
					/**
					 * Handle OAM DMA copy request
					 */
					uint8_t dma = mmuReadByte(mmu, 0xff46);

					if (dma)
					{
						for (int i = 0; i < 160; i++)
						{
							mmuWriteByte(mmu, 0xFE00 + i, mmuReadByte(mmu, (dma * 256) + i));
						}
						mmuWriteByte(mmu, 0xff46, 0x00);
					}
					
					/**
					 * Don't draw if not required
					 */
					if (display->draw)
					{
						display->draw();
					}

					display->mode = DISPLAY_MODE_VBLANK;
				}
				else
				{
					display->mode = DISPLAY_MODE_OAM; 
				}

				display->scanline++;

				/**
				 *  Write current line to IO
				 */
				mmuWriteByte(mmu, 0xFF44, display->scanline);
				display->cycles -= 204;
			}
			break;

		case DISPLAY_MODE_VBLANK:
			if (display->cycles > 455)
			{
				display->scanline++;

				if (display->scanline == 153)
				{ 
					display->scanline = 0;
					display->mode = DISPLAY_MODE_OAM;
				}

				/**
				 *  Write current line to IO
				 */
				mmuWriteByte(mmu, 0xFF44, display->scanline);
				display->cycles -= 456;
			}
			break;

		case DISPLAY_MODE_OAM:
			if (display->cycles > 79)
			{
				display->mode = DISPLAY_MODE_VRAM;
				display->cycles -= 80;
			}
			break;

		case DISPLAY_MODE_VRAM:
			if (display->cycles > 171)
			{
				/**
				 * Don't render if not required
				 */
				if (display->render)
				{
					displayScanline(display, mmu);
				}
				
				display->mode = DISPLAY_MODE_HBLANK;
				display->cycles -= 172;
			}
			break;	
	}
}

void displayScanline(Display *display, MMU *mmu)
{
	uint8_t lcdc = mmuReadByte(mmu, 0xFF40);

	/**
	 * If BG display
	 */
	if (lcdc & 0x01)
	{
		uint16_t tilemapAddress = lcdc & 0x08 ? 0x9C00 : 0x9800;
		uint16_t tiledataAddress = lcdc & 0x10 ? 0x8000 : 0x8800;

		uint8_t scrollX = mmuReadByte(mmu, 0xFF43);
		uint8_t scrollY = mmuReadByte(mmu, 0xFF42);

		uint8_t mapOffsetX = scrollX / 8;
		uint8_t mapOffsetY = (scrollY + display->scanline) / 8;

		uint8_t tileX = scrollX % 8;
		uint8_t tileY = (display->scanline + scrollY) % 8;

		uint8_t tileAddress = 0x00;
		uint16_t tileLine = 0x0000;

		for (int x = 0; x < 160; x++)
		{
			if (!x || tileX == 0x08)
			{
				if (x > 0)
				{
					tileX = 0x00;
					mapOffsetX++;
				}

				/**
				 * 16 Bytes in a Tile
				 * 2 Bytes in a Tile Line
				 */
				tileAddress = mmuReadByte(mmu, tilemapAddress + mapOffsetX + (mapOffsetY * 32));
				tileAddress += tiledataAddress == 0x8800 ? 0x80 : 0; /* Signed check */
				tileLine = mmuReadWord(mmu, tiledataAddress + (tileAddress * 16) + (tileY * 2)); 
			}

			display->render(x, display->scanline, displayGetColor(tileLine, 0x01 << (7 - tileX)));
			tileX++;
		}
	}

	/**
	 * If Sprite display
	 */
	if (lcdc & 0x02)
	{
		uint16_t oamAddress = 0xFE00;
		uint8_t spriteHeight = lcdc & 0x04 ? 0x10 : 0x08;

		/**
		 * Scan backwards for priority
		 */
		for (int i = 39; i >= 0; i--)
		{
			/**
			 * 4 Bytes in per OAM record
			 */
			uint8_t posX = mmuReadByte(mmu, oamAddress + (i * 4) + 0x01) - 0x08;
			uint8_t posY = mmuReadByte(mmu, oamAddress + (i * 4)) - 0x10;
			uint8_t spriteAddress = mmuReadByte(mmu, oamAddress + (i * 4) + 0x02);
			uint8_t spriteData = mmuReadByte(mmu, oamAddress + (i * 4) + 0x03);

			if (display->scanline >= posY && display->scanline < (posY + spriteHeight))
			{
				uint8_t spriteY = spriteData & 0x40 ? spriteHeight - (display->scanline - posY) : display->scanline - posY; /* Flip Y */

				/**
				 * 16 Bytes in a Sprite
				 * 2 Bytes in a Sprite Line
				 */
				uint16_t spriteLine = mmuReadWord(mmu, 0x8000 + (spriteAddress * 16) + (spriteY * 2));

				for (int x = 0; x < 8; x++)
				{
					uint8_t pixelIndex = spriteData & 0x20 ? 0x01 << x : 0x01 << (7 - x); /* Flip X */
					int pixelColor = displayGetColor(spriteLine, pixelIndex);

					if (posX + x > 0 && posX + x < 160 && pixelColor > 0)
					{
						display->render(posX + x, display->scanline, pixelColor);
					}
				}
			}
		}
	}
}

int displayGetColor(uint16_t tileLine, uint8_t colorBit)
{
	return (((tileLine & 0xFF) & colorBit) ? 1 : 0) + (((tileLine >> 8) & colorBit) ? 2 : 0);
}