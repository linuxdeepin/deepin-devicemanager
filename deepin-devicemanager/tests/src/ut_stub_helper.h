// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_STUB_HELPER_H
#define UT_STUB_HELPER_H

/*
 * @file ut_stub_helper.h
 * @brief stub-ext 辅助：虚函数成员指针的真实地址还原
 *
 * 背景：tests/src/stub.h 的 Stub::set 通过"取函数地址后改写函数头几条指令"实现打桩，
 * 但对**虚成员函数**取地址时，GCC 按 Itanium ABI 编码为 vtable 槽位（PFN = slot*8+1），
 * 直接传给 Stub::set 会把槽位号当成代码地址 mprotect/patch，导致未定义行为/崩溃。
 * （例：DDialog::exec 在本机 libdtk6widget vtable 槽位 53，PMF 位模式 0x1a9。）
 *
 * 用法（先构造一个目标对象，再还原真实地址后打桩）：
 *   DDialog probe;
 *   Stub stub;
 *   stub.set(ut_real_fn_addr(&probe, static_cast<int (DDialog::*)()>(&DDialog::exec)),
 *            ut_ddialog_exec);
 *
 * 打桩的是"该类的虚函数实现代码"本身（替换函数头指令），恢复由 Stub 析构自动完成。
 */

#include <cstring>

template <typename MemFn>
inline MemFn ut_real_fn_addr(void *obj, MemFn pmf)
{
    unsigned long bits = 0;
    std::memcpy(&bits, &pmf, sizeof(bits));
    MemFn out;
    if (bits & 1UL) {
        // GCC 虚函数 PMF：PFN = slot * sizeof(void*) + 1
        void **vptr = *reinterpret_cast<void ***>(obj);
        const unsigned long slot = bits >> 3;
        const unsigned long real = reinterpret_cast<unsigned long>(vptr[slot]);
        std::memcpy(&out, &real, sizeof(real));
    } else {
        out = pmf;   // 非虚成员/静态成员：PFN 即真实代码地址
    }
    return out;
}

#endif  // UT_STUB_HELPER_H
