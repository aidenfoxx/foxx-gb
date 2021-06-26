#include "timer.h"

const static uint16_t timaPeriods[4] = {
	0x400, 0x10, 0x40, 0x100
};

void timerInit(Timer *timer, MMU *mmu)
{
	timer->mmu = mmu;
}

void timerStep(Timer *timer, unsigned cycles)
{
	timer->divcount += cycles;

	if (timer->divcount >= 0x40)	{
		timer->divcount -= 0x40;
		mmuWriteByte(timer->mmu, 0xFF04, mmuReadByte(timer->mmu, 0xFF04) + 1); // 0xFF04 = DIV
	}

	uint8_t tac = mmuReadByte(timer->mmu, 0xFF07); // 0xFF07 = TAC

	if (!(tac & 0x4)) {
		return;
	}

	/**
	 * Increment TIMA
	 */
	timer->timacount += cycles;

	if (timer->timacount >= timaPeriods[tac & 0x3]) {
		timer->timacount -= timaPeriods[tac & 0x3];
		uint8_t tima = mmuReadByte(timer->mmu, 0xFF05) + 1;

		if (tima) {
			mmuWriteByte(timer->mmu, 0xFF05, tima);
			return;
		}

		mmuWriteByte(timer->mmu, 0xFF05, mmuReadByte(timer->mmu, 0xFF06));

		/**
		 * Set timer interrupt flag
		 */
		mmuWriteByte(timer->mmu, 0xFF0F, mmuReadByte(timer->mmu, 0xFF0F) | 0x4);
	}
}
