#pragma once
#include <stdint.h>

// ── Types ────────────────────────────────────────────────────────────────────
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int16_t  s16;
typedef int32_t  s32;

#define INLINE static inline __attribute__((always_inline))

// ── Display registers ────────────────────────────────────────────────────────
#define REG_DISPCNT  (*(volatile u16*)0x04000000)
#define REG_DISPSTAT (*(volatile u16*)0x04000004)
#define REG_VCOUNT   (*(volatile u16*)0x04000006)

#define DISPCNT_MODE0    0x0000
#define DISPCNT_BG0_ON   (1 << 8)
#define DISPCNT_BG1_ON   (1 << 9)
#define DISPCNT_BG2_ON   (1 << 10)
#define DISPCNT_OBJ_ON   (1 << 12)
#define DISPCNT_OBJ_1D  (1 << 6)

// ── BG control registers ─────────────────────────────────────────────────────
#define REG_BG0CNT (*(volatile u16*)0x04000008)
#define REG_BG1CNT (*(volatile u16*)0x0400000A)
#define REG_BG2CNT (*(volatile u16*)0x0400000C)

// BG scroll registers
#define REG_BG0HOFS (*(volatile u16*)0x04000010)
#define REG_BG0VOFS (*(volatile u16*)0x04000012)
#define REG_BG1HOFS (*(volatile u16*)0x04000014)
#define REG_BG1VOFS (*(volatile u16*)0x04000016)
#define REG_BG2HOFS (*(volatile u16*)0x04000018)
#define REG_BG2VOFS (*(volatile u16*)0x0400001A)

// BGxCNT fields
#define BG_PRIO(n)       ((n) & 3)
#define BG_CBB(n)        (((n) & 3) << 2)   // character base block (tile data)
#define BG_SBB(n)        (((n) & 31) << 8)  // screen base block (tile map)
#define BG_4BPP          0
#define BG_8BPP          (1 << 7)
#define BG_SIZE_32x32    0
#define BG_SIZE_64x32    (1 << 14)

// ── Memory ───────────────────────────────────────────────────────────────────
// VRAM: 96KB at 0x06000000
// BG tile data (char blocks): each block = 16KB
// BG tile maps (screen blocks): each block = 2KB (32x32 tiles of 2 bytes each)
// OBJ tile data starts at 0x06010000 in Mode 0
// ALL VRAM/PAL/OAM pointers must be volatile — otherwise the compiler may
// reorder or eliminate writes to memory-mapped hardware.

#define VRAM_BASE    ((volatile u16*)0x06000000)
#define CBB(n)       ((volatile u16*)(0x06000000 + (n)*16*1024))
#define SBB(n)       ((volatile u16*)(0x06000000 + (n)*2*1024))

// Palettes
#define BG_PAL       ((volatile u16*)0x05000000)
#define OBJ_PAL      ((volatile u16*)0x05000200)

// OAM
#define OAM          ((volatile u16*)0x07000000)

// ── Palette helpers ──────────────────────────────────────────────────────────
#define RGB15(r,g,b) ((u16)((r) | ((g)<<5) | ((b)<<10)))

// ── VSync ────────────────────────────────────────────────────────────────────
// Wait for VBlank using VCOUNT. GBA VCOUNT cycles 0-227 @ ~60Hz.
// Lines 0-159 = active. Lines 160-227 = vblank.
INLINE void vsync(void) {
    while (REG_VCOUNT != 160) {}
}

// ── Fixed-point (16.16) ──────────────────────────────────────────────────────
typedef s32 fixed;
#define INT_TO_FX(n)   ((n) << 16)
#define FX_TO_INT(f)   ((f) >> 16)
#define FX_MUL(a,b)    (((a) >> 8) * ((b) >> 8))
