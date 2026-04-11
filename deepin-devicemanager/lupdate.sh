#!/bin/bash

# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

/usr/lib/qt6/bin/lupdate ./src -ts -no-obsolete ./translations/deepin-devicemanager.ts
/usr/lib/qt6/bin/lupdate ./src -ts -no-obsolete ./translations/deepin-devicemanager_zh_CN.ts
/usr/lib/qt6/bin/lupdate ./src -ts -no-obsolete ./translations/deepin-devicemanager_zh_TW.ts
/usr/lib/qt6/bin/lupdate ./src -ts -no-obsolete ./translations/deepin-devicemanager_zh_HK.ts
/usr/lib/qt6/bin/lupdate ./src -ts -no-obsolete ./translations/deepin-devicemanager_ug.ts
/usr/lib/qt6/bin/lupdate ./src -ts -no-obsolete ./translations/deepin-devicemanager_bo.ts