#ifndef __ARMCPU_H
#define __ARMCPU_H

namespace arm {
    enum CPUMode {
        MODE_USER = 0x10,
        MODE_FIQ = 0x11,
        MODE_IRQ = 0x12,
        MODE_SVC = 0x13,
        MODE_MON = 0x16,
        MODE_ABT = 0x17,
        MODE_HYP = 0x1A,
        MODE_UND = 0x1B,
        MODE_SYS = 0x1F
    };

    enum CPSRMask {
        MASK_MODE = 0x1F,
        MASK_THUMB = 0x20,
        MASK_FIQ = 0x40,
        MASK_IRQ = 0x80
    };
}

#define ARMFUNC __attribute__((target("arm"))) \
    __attribute__ ((noinline))

#endif