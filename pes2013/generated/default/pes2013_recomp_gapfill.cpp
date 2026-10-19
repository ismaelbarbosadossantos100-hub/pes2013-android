// Generated / Handcrafted Companion for Missing Functions
// PES 2013 Native Recompilation (ReXGlue)
// All functions disassembled from retail default.xex and validated against PPC specifications.

#include "pes2013_pch.h"
#include "pes2013_funcs.h"
#include <rex/system/function_dispatcher.h>

// Declare external functions called via tail calls
DECLARE_REX_FUNC(sub_82A10528);
DECLARE_REX_FUNC(sub_82C4B908);
DECLARE_REX_FUNC(sub_82F63B48);
DECLARE_REX_FUNC(sub_828BABB0);

// ============================================================================
// 1. CRT Static Initializers
// ============================================================================

DEFINE_REX_FUNC(sub_831DE0D0) {
    REX_FUNC_PROLOGUE();
    // lis r8,-0x7caf (0x83510000)
    ctx.r8.s64 = -2091843584;
    // li r10,0
    ctx.r10.s64 = 0;
    // addi r7,r8,-0x3250
    ctx.r7.s64 = ctx.r8.s64 - 12880;
    // li r9,0
    ctx.r9.s64 = 0;
    // lwz r11,-0x3250(r8)
    ctx.r11.u64 = REX_LOAD_U32(ctx.r8.u32 - 12880);
    // oris r11,r11,0xffff
    ctx.r11.u64 = ctx.r11.u64 | 4294901760ull;
    // stw r11,-0x3250(r8)
    REX_STORE_U32(ctx.r8.u32 - 12880, ctx.r11.u32);
    // stb r10,2(r7)
    REX_STORE_U8(ctx.r7.u32 + 2, ctx.r10.u8);
    // stb r9,3(r7)
    REX_STORE_U8(ctx.r7.u32 + 3, ctx.r9.u8);
    // blr
    return;
}

DEFINE_REX_FUNC(sub_831DE338) {
    REX_FUNC_PROLOGUE();
    // lis r11,-0x7cdc
    ctx.r11.s64 = (int64_t)(int32_t)0x83240000;
    // addi r3,r11,0x4ea8
    ctx.r3.s64 = (int64_t)(int32_t)0x83244EA8;
    // b 0x82a10528
    sub_82A10528(ctx, base);
    return;
}

DEFINE_REX_FUNC(sub_831DE480) {
    REX_FUNC_PROLOGUE();
    ctx.r11.s64 = (int64_t)(int32_t)0x83240000;
    ctx.r3.s64 = (int64_t)(int32_t)0x83245048;
    sub_82A10528(ctx, base);
    return;
}

DEFINE_REX_FUNC(sub_831E24E8) {
    REX_FUNC_PROLOGUE();
    ctx.r11.s64 = (int64_t)(int32_t)0x83240000;
    ctx.r3.s64 = (int64_t)(int32_t)0x832456F8;
    sub_82A10528(ctx, base);
    return;
}

DEFINE_REX_FUNC(sub_831E28A0) {
    REX_FUNC_PROLOGUE();
    ctx.r8.s64 = -2091843584;
    ctx.r10.s64 = 0;
    ctx.r7.s64 = ctx.r8.s64 - 11104;
    ctx.r9.s64 = 0;
    ctx.r11.u64 = REX_LOAD_U32(ctx.r8.u32 - 11104);
    ctx.r11.u64 = ctx.r11.u64 | 4294901760ull;
    REX_STORE_U32(ctx.r8.u32 - 11104, ctx.r11.u32);
    REX_STORE_U8(ctx.r7.u32 + 2, ctx.r10.u8);
    REX_STORE_U8(ctx.r7.u32 + 3, ctx.r9.u8);
    return;
}

DEFINE_REX_FUNC(sub_831E48E8) {
    REX_FUNC_PROLOGUE();
    ctx.r10.s64 = 19;
    ctx.r11.s64 = (int64_t)(int32_t)(0x836F0000 - 0x1ff8);
    ctx.ctr.u64 = 19;
    ctx.r11.s64 -= 0xa8;
    ctx.r10.s64 = 0;
    do {
        REX_STORE_U32(ctx.r11.u32 + 0xac, 0);
        REX_STORE_U32(ctx.r11.u32 + 0xb0, 0);
        ctx.r11.s64 += 0xb4;
        REX_STORE_U32(ctx.r11.u32, 0);
    } while (--ctx.ctr.u64 != 0);
    ctx.r11.s64 = (int64_t)(int32_t)0x83240000;
    ctx.r3.s64 = (int64_t)(int32_t)0x83246218;
    sub_82A10528(ctx, base);
    return;
}

DEFINE_REX_FUNC(sub_831E9B18) {
    REX_FUNC_PROLOGUE();
    ctx.r8.s64 = (int64_t)(int32_t)0x836F0000;
    ctx.r10.s64 = 0;
    ctx.r7.s64 = ctx.r8.s64 - 0x9e0;
    ctx.r9.s64 = 0;
    ctx.r11.u64 = REX_LOAD_U32(ctx.r8.u32 - 0x9e0);
    ctx.r11.u64 |= 4294901760ull;
    REX_STORE_U32(ctx.r8.u32 - 0x9e0, ctx.r11.u32);
    REX_STORE_U8(ctx.r7.u32 + 2, 0);
    REX_STORE_U8(ctx.r7.u32 + 3, 0);
    return;
}

DEFINE_REX_FUNC(sub_831EAD08) {
    REX_FUNC_PROLOGUE();
    ctx.r8.s64 = (int64_t)(int32_t)0x836F0000;
    ctx.r10.s64 = 0;
    ctx.r7.s64 = ctx.r8.s64 + 0xf44;
    ctx.r9.s64 = 0;
    ctx.r11.u64 = REX_LOAD_U32(ctx.r8.u32 + 0xf44);
    ctx.r11.u64 |= 4294901760ull;
    REX_STORE_U32(ctx.r8.u32 + 0xf44, ctx.r11.u32);
    REX_STORE_U8(ctx.r7.u32 + 2, 0);
    REX_STORE_U8(ctx.r7.u32 + 3, 0);
    return;
}

DEFINE_REX_FUNC(sub_831EC808) {
    REX_FUNC_PROLOGUE();
    ctx.r8.s64 = (int64_t)(int32_t)0x836F0000;
    ctx.r10.s64 = 0;
    ctx.r7.s64 = ctx.r8.s64 + 0x1aa8;
    ctx.r9.s64 = 0;
    ctx.r11.u64 = REX_LOAD_U32(ctx.r8.u32 + 0x1aa8);
    ctx.r11.u64 |= 4294901760ull;
    REX_STORE_U32(ctx.r8.u32 + 0x1aa8, ctx.r11.u32);
    REX_STORE_U8(ctx.r7.u32 + 2, 0);
    REX_STORE_U8(ctx.r7.u32 + 3, 0);
    return;
}

DEFINE_REX_FUNC(sub_831EFDB8) {
    REX_FUNC_PROLOGUE();
    ctx.r11.s64 = (int64_t)(int32_t)0x83240000;
    ctx.r3.s64 = (int64_t)(int32_t)0x832472D8;
    sub_82A10528(ctx, base);
    return;
}

// ============================================================================
// 2. Previously Unresolved Function Targets
// ============================================================================

DEFINE_REX_FUNC(sub_82C4C440) {
    REX_FUNC_PROLOGUE();
    sub_82C4B908(ctx, base);
    return;
}

DEFINE_REX_FUNC(sub_828C6A00) {
    REX_FUNC_PROLOGUE();
    ctx.r11.u64 = ctx.r4.u64;
    ctx.cr0.compare<int32_t>(ctx.r4.s32, 0, ctx.xer);
    if (!ctx.cr0.eq) {
        ctx.r10.s64 = (int64_t)(int32_t)0x83510000;
        ctx.r3.u64 = REX_LOAD_U32(ctx.r10.u32 - 0x51b4);
        ctx.cr0.compare<uint32_t>(ctx.r3.u32, 0, ctx.xer);
        if (!ctx.cr0.eq) {
            ctx.r4.s64 = 1;
            int32_t val = (int32_t)ctx.r11.u32;
            if (val == 3) {
                ctx.r4.s64 = 0;
            } else if (val < 4 || val > 6) {
                // fall through
            }
            uint32_t ptr = REX_LOAD_U32(ctx.r3.u32);
            uint32_t fn = REX_LOAD_U32(ptr + 0x1c);
            ctx.ctr.u64 = fn;
            rex::runtime::ResolveIndirectFunction(fn)(ctx, base);
            return;
        }
    }
    ctx.r3.s64 = 1;
    return;
}

DEFINE_REX_FUNC(sub_82A31F30) {
    REX_FUNC_PROLOGUE();
    ctx.r11.s64 = (ctx.r4.s32 == 0) ? 0 : 1;
    REX_STORE_U32(ctx.r3.u32 + 0xac, ctx.r11.u32);
    return;
}

DEFINE_REX_FUNC(sub_82F5D000) {
    REX_FUNC_PROLOGUE();
    ctx.r3.u64 = REX_LOAD_U32(ctx.r3.u32 + 0x260);
    sub_82F63B48(ctx, base);
    return;
}

DEFINE_REX_FUNC(sub_82E80CC0) {
    REX_FUNC_PROLOGUE();
    PPCRegister temp;
    ctx.r11.s64 = (int64_t)(int32_t)0x820A0000;
    ctx.r3.u64 = REX_LOAD_U32(ctx.r3.u32 + 0x5c);
    temp.u32 = REX_LOAD_U32(ctx.r11.u32 - 0x1828);
    ctx.f1.f64 = double(temp.f32);
    sub_828BABB0(ctx, base);
    return;
}

DEFINE_REX_FUNC(sub_82A32A58) {
    REX_FUNC_PROLOGUE();
    ctx.r3.u64 = REX_LOAD_U32(ctx.r3.u32 + 0xb8);
    if (ctx.r3.u32 == 0) {
        return;
    }
    ctx.r11.s64 = (int64_t)(int32_t)0x83360000;
    uint32_t fn = REX_LOAD_U32(ctx.r11.u32 + 0x4d3c);
    ctx.ctr.u64 = fn;
    rex::runtime::ResolveIndirectFunction(fn)(ctx, base);
    return;
}

DEFINE_REX_FUNC(sub_823EABC0) {
    REX_FUNC_PROLOGUE();
    PPCRegister temp;
    uint8_t flag = REX_LOAD_U8(ctx.r3.u32 + 8);
    uint64_t r11_val = (uint32_t)ctx.r4.u64;
    if (flag != 0) {
        int64_t i_val = (int64_t)r11_val;
        double d_val = (double)i_val;
        float f_val = (float)d_val;
        ctx.f0.f64 = (double)f_val;
        REX_STORE_U8(ctx.r3.u32 + 8, 0);
    } else {
        temp.u32 = REX_LOAD_U32(ctx.r3.u32 + 0xc);
        float f0_in = temp.f32;
        int64_t i13 = (int64_t)r11_val;
        float f13 = (float)(double)i13;
        float f12 = (f0_in <= f13) ? (f13 - f0_in) : (f0_in - f13);
        float f1_in = (float)ctx.f1.f64;
        float f11 = f0_in * f1_in;
        temp.u32 = REX_LOAD_U32(ctx.r3.u32 + 0x10);
        float f10 = f1_in * temp.f32;
        uint32_t r11_addr = 0x82100000 - 0x380c;
        temp.u32 = REX_LOAD_U32(r11_addr);
        float const_f0 = temp.f32;
        float term0 = const_f0 - f1_in;
        float st_10 = (term0 * f12) + f10;
        temp.f32 = st_10;
        REX_STORE_U32(ctx.r3.u32 + 0x10, temp.u32);
        float st_c = (term0 * f13) + f11;
        ctx.f0.f64 = (double)st_c;
    }
    temp.f32 = (float)ctx.f0.f64;
    REX_STORE_U32(ctx.r3.u32 + 0xc, temp.u32);
    return;
}
