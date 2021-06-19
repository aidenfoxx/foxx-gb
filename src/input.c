#include "input.h"

void inputInit(Joypad *joypad)
{
	joypad->directions = 0x0F;
	joypad->buttons = 0x0F;
}

void inputStep(Joypad *joypad, MMU *mmu)
{
	uint8_t input = mmuReadByte(mmu, 0xFF00);

	if (!(input & 0x10)) {
		mmuWriteByte(mmu, 0xFF00, (input & 0xF0) + (joypad->directions & 0x0F));
	}

	if (!(input & 0x20)) {
		mmuWriteByte(mmu, 0xFF00, (input & 0xF0) + (joypad->buttons & 0x0F));
	}
}

void inputTrigger(Joypad *joypad, MMU *mmu, int button)
{
	switch (button) {
		case BUTTON_UP:
			joypad->directions &= 0xFB;
			break;

		case BUTTON_DOWN:
			joypad->directions &= 0xF7;
			break;

		case BUTTON_LEFT:
			joypad->directions &= 0xFD;
			break;

		case BUTTON_RIGHT:
			joypad->directions &= 0xFE;
			break;

		case BUTTON_START:
			joypad->buttons &= 0xF7;
			break;

		case BUTTON_SELECT:
			joypad->buttons &= 0xFB;
			break;

		case BUTTON_B:
			joypad->buttons &= 0xFD;
			break;

		case BUTTON_A:
			joypad->buttons &= 0xFE;
			break;
	}

	/**
	 * Set joypad interrupt flag
	 */
	if (mmuReadByte(mmu, 0xFFFF) & 0x10) {
		mmuWriteByte(mmu, 0xFF0F, mmuReadByte(mmu, 0xFF0F) | 0x10);
	}
}

void inputRelease(Joypad *joypad, int button)
{
	switch (button) {
		case BUTTON_UP:
			joypad->directions |= 0x04;
			break;

		case BUTTON_DOWN:
			joypad->directions |= 0x08;
			break;

		case BUTTON_LEFT:
			joypad->directions |= 0x02;
			break;

		case BUTTON_RIGHT:
			joypad->directions |= 0x01;
			break;

		case BUTTON_START:
			joypad->buttons |= 0x08;
			break;

		case BUTTON_SELECT:
			joypad->buttons |= 0x04;
			break;

		case BUTTON_B:
			joypad->buttons |= 0x02;
			break;

		case BUTTON_A:
			joypad->buttons |= 0x01;
			break;
	}
}
