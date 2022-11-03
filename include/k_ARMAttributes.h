#ifndef __K_ARMATTRIBUTES_H
#define __K_ARMATTRIBUTES_H

#define ARMFUNC __attribute__((target("arm"))) \
    __attribute__ ((noinline))

#define THMFUNC __attribute__((target("thumb"))) \
    __attribute__ ((noinline))

#define IRQFUNC __attribute__((interrupt ("IRQ")))

#define LOWLEVEL __attribute__((naked))

#endif