#include "power.h"
#include "reg_power.h"

#define POWER_PASSWORD (0x5A << 24)

// reboot after watchdog timer expire
void power_reset(int tick) {
    *POWER_RSTC = POWER_PASSWORD | 0x20;    // full reset
    *POWER_WDOG = POWER_PASSWORD | tick;    // number of watchdog tick
}

void power_cancel_reset() {
    *POWER_RSTC = POWER_PASSWORD | 0x0;    // full reset
    *POWER_WDOG = POWER_PASSWORD | 0x0;    // number of watchdog tick
}