#include "timer.h"

const static uint16_t timaPeriods[4] = {
	0x0400, 0x10, 0x40, 0x0100
};

void timerInit(Timer *timer, MMU *mmu)
{
	timer->mmu = mmu;
}

void timerStep(Timer *timer, unsigned cycles)
{
	timer->divcount += cycles;

	if (timer->divcount >= 0x40)	{
		mmuWriteByte(timer->mmu, 0xFF04, mmuReadByte(timer->mmu, 0xFF04) + 0x01);
		timer->divcount -= 0x40;
	}

	uint8_t tac = mmuReadByte(timer->mmu, 0xFF07);

	if (!(tac & 0x04)) {
		timer->timacount = 0x00;
		return;
	}

	/**
	 * Increment TIMA
	 */
	timer->timacount += cycles;

	if (timer->timacount >= timaPeriods[tac & 0x03]) {
		mmuWriteByte(timer->mmu, 0xFF05, mmuReadByte(timer->mmu, 0xFF05) + 0x01);
		timer->timacount -= timaPeriods[tac & 0x03];

		if (mmuReadByte(timer->mmu, 0xFF05) == 0) {
			mmuWriteByte(timer->mmu, 0xFF05, mmuReadByte(timer->mmu, 0xFF06));

			/**
			 * Set timer interrupt flag
			 */
			if (mmuReadByte(timer->mmu, 0xFFFF) & 0x04) {
				mmuWriteByte(timer->mmu, 0xFF0F, mmuReadByte(timer->mmu, 0xFF0F) | 0x04);
			}
		}
	}
}
