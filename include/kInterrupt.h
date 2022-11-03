#ifndef __KINTERRUPT_H
#define __KINTERRUPT_H

#include "k_Init.h"
#include "kTypes.h"
#include "k_DllExport.h"
#include "exl_EnumFlagOperators.h"
#include "k_Regs.h"

K_REG_DEFINE(IME, int, 0x04000208);
K_REG_DEFINE(IE, int, 0x04000210);
K_REG_DEFINE(IF, int, 0x04000214);

namespace k {
    enum InterruptID
    {
        VBLANK = 0x0,
        HBLANK = 0x1,
        VCOUNT_MATCH = 0x2,
        TIMER_0 = 0x3,
        TIMER_1 = 0x4,
        TIMER_2 = 0x5,
        TIMER_3 = 0x6,
        DMA_0 = 0x8,
        DMA_1 = 0x9,
        DMA_2 = 0xA,
        DMA_3 = 0xB,
        KEYPAD = 0xC,
        GBA_SLOT = 0xD,
        IPC_SYNC = 0xE,
        IPC_SEND_FIFO_EMPTY = 0xF,
        IPC_RECV_FIFO_NOT_EMPTY = 0x10,
        CART_TRANSFER_DONE = 0x11,
        CART_IREQ_MC = 0x12,
        GE_FIFO = 0x13,
    };

    enum InterruptBit
    {
        BIT_VBLANK = 0x1,
        BIT_HBLANK = 0x2,
        BIT_VCOUNT_MATCH = 0x4,
        BIT_TIMER_0 = 0x8,
        BIT_TIMER_1 = 0x10,
        BIT_TIMER_2 = 0x20,
        BIT_TIMER_3 = 0x40,
        BIT_DMA_0 = 0x100,
        BIT_DMA_1 = 0x200,
        BIT_DMA_2 = 0x400,
        BIT_DMA_3 = 0x800,
        BIT_KEYPAD = 0x1000,
        BIT_GBA_SLOT = 0x2000,
        BIT_IPC_SYNC = 0x10000,
        BIT_IPC_SEND_FIFO_EMPTY = 0x20000,
        BIT_IPC_RECV_FIFO_NOT_EMPTY = 0x40000,
        BIT_CART_TRANSFER_DONE = 0x80000,
        BIT_CART_IREQ_MC = 0x100000,
        BIT_GE_FIFO = 0x200000
    };
    DEFINE_ENUM_FLAG_OPERATORS(InterruptBit)

    typedef void(*InterruptFunc)();

    class Interrupt {
    public:
        class _InitClass {
        private:
            static void _Init();
            static void _Terminate();
        
        friend class KInitializer;
        };

    private:
        static void HandleInterrupt();

    public:
        K_PUBLIC static void Set(InterruptBit intrBits, InterruptFunc func);
        K_PUBLIC static void Set(InterruptID intrId, InterruptFunc func);

        K_PUBLIC static void Enable(InterruptBit mask);
        K_PUBLIC static void Disable(InterruptBit mask);
        K_PUBLIC static bool IsEnabled(InterruptBit mask);
    };
};

#endif