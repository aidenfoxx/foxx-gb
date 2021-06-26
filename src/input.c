#include "input.h"

void inputInit(Input *input, MMU *mmu)
{
	input->mmu = mmu;
	input->directions = 0xF;
	input->buttons = 0x0F;
}

void inputStep(Input *input)
{
	uint8_t status = mmuReadByte(input->mmu, 0xFF00);

	if (!(status & 0x10)) {
		mmuWriteByte(input->mmu, 0xFF00, (status & 0xF0) + (input->directions & 0x0F));
	}

	if (!(status & 0x20)) {
		mmuWriteByte(input->mmu, 0xFF00, (status & 0xF0) + (input->buttons & 0x0F));
	}
}

void inputTrigger(Input *input, unsigned button)
{
	switch (button) {
		case BUTTON_UP:
			input->directions &= 0xFB;
			break;

		case BUTTON_DOWN:
			input->directions &= 0xF7;
			break;

		case BUTTON_LEFT:
			input->directions &= 0xFD;
			break;

		case BUTTON_RIGHT:
			input->directions &= 0xFE;
			break;

		case BUTTON_START:
			input->buttons &= 0xF7;
			break;

		case BUTTON_SELECT:
			input->buttons &= 0xFB;
			break;

		case BUTTON_B:
			input->buttons &= 0xFD;
			break;

		case BUTTON_A:
			input->buttons &= 0xFE;
			break;
	}

	/**
	 * Set input interrupt flag
	 */
	mmuWriteByte(input->mmu, 0xFF0F, mmuReadByte(input->mmu, 0xFF0F) | 0x10);
}

void inputRelease(Input *input, unsigned button)
{
	switch (button) {
		case BUTTON_UP:
			input->directions |= 0x04;
			break;

		case BUTTON_DOWN:
			input->directions |= 0x08;
			break;

		case BUTTON_LEFT:
			input->directions |= 0x02;
			break;

		case BUTTON_RIGHT:
			input->directions |= 0x01;
			break;

		case BUTTON_START:
			input->buttons |= 0x08;
			break;

		case BUTTON_SELECT:
			input->buttons |= 0x04;
			break;

		case BUTTON_B:
			input->buttons |= 0x02;
			break;

		case BUTTON_A:
			input->buttons |= 0x01;
			break;
	}
}
