#include "timer.h"

const static uint16_t timaPeriods[4] = {
	0x0400, 0x10, 0x40, 0x0100
};

void timerInit(Timer *timer)
{
	timer->divcount = 0x00;
	timer->timacount = 0x00;
}

void timerStep(Timer *timer, MMU *mmu, uint8_t cycles)
{
	timer->divcount += cycles;

	if (timer->divcount >= 0x40)	{
		mmuWriteByte(mmu, 0xFF04, mmuReadByte(mmu, 0xFF04) + 0x01);
		timer->divcount -= 0x40;
	}

	uint8_t tac = mmuReadByte(mmu, 0xFF07);

	/**
	 * Increment TIMA if enabled
	 */
	if (tac & 0x04) {
		timer->timacount += cycles;

		if (timer->timacount >= timaPeriods[tac & 0x03]) {
			mmuWriteByte(mmu, 0xFF05, mmuReadByte(mmu, 0xFF05) + 0x01);
			timer->timacount -= timaPeriods[tac & 0x03];

			if (mmuReadByte(mmu, 0xFF05) == 0) {
				mmuWriteByte(mmu, 0xFF05, mmuReadByte(mmu, 0xFF06));

				/**
				 * Set timer interrupt flag
				 */
				if (mmuReadByte(mmu, 0xFFFF) & 0x04) {
					mmuWriteByte(mmu, 0xFF0F, mmuReadByte(mmu, 0xFF0F) | 0x04);
				}
			}
		}
	} else {
		timer->timacount = 0x00;
	}
}
