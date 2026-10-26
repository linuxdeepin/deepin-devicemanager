// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_MOCK_THREAD_H
#define UT_MOCK_THREAD_H

// stub-c 通过取函数地址打桩,而 Itanium ABI 下指向虚成员函数的指针首字节为
// "vtable 偏移|1"(如 0x61),并非代码地址,直接 stub.set 会段错误。
// QDialog::exec 为虚函数,因此对模态弹窗一律用子类重写(MOCK)方式替换,
// 通过 #define private public 访问到的 mp_FailedDialog 成员指针换绑。
// (QThread::run 同为虚函数,但线程启动已改用 QThread::start 计数桩,不再需要 MOCK 线程。)

#include <DDialog>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class MockFailedDialog : public DDialog
{
public:
    explicit MockFailedDialog(QWidget *parent = nullptr)
        : DDialog(parent)
    {
    }
    int m_execCount = 0;
    int m_execReturn = 0;
    int exec() override
    {
        m_execCount++;
        return m_execReturn;
    }
};

#endif // UT_MOCK_THREAD_H
