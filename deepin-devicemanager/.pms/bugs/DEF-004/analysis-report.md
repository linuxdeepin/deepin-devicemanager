# BUG-DEF-004 根因分析

- **Bug ID**: DEF-004
- **标题**: 文件级 static firstLoadFlag 跨实例共享，新建实例不再 emit finishedReadFilePool 信号
- **PMS URL**: 无（来自父 Issue V-6208 单元测试缺陷导出）
- **状态**: 待分析
- **Bug 类型**: D1 功能逻辑缺陷
- **分析时间**: 2026-10-23
- **修复提交**: 未提取到
- **涉及项目**: deepin-devicemanager
- **工作模式**: multica 模式（上下文）

## 一、问题概述

`LoadInfoThread` 类在 `slotFinishedReadFilePool()` 槽函数中通过文件级 `static bool firstLoadFlag = true` 控制是否 emit `finishedReadFilePool()` 信号——仅在"首次加载"时发出。由于该变量是文件级静态变量，其生命周期跨越进程内所有 `LoadInfoThread` 实例，一旦第一个实例的 `slotFinishedReadFilePool()` 被调用并将 `firstLoadFlag` 置为 `false`，后续创建的所有新实例在调用同一槽函数时将永远不再 emit `finishedReadFilePool()` 信号。

在当前生产代码中，`MainWindow` 仅创建一个 `LoadInfoThread` 实例（`MainWindow.cpp:82`），因此单实例场景下该缺陷不会显现。但在单元测试场景（`ut_loadinfothread.cpp` SetUp/TearDown 反复 new/delete 实例）以及未来可能的重构中（如 `MainWindow` 被销毁重建），第二个及以后的实例将无法触发 `finishedReadFilePool` 信号，导致 `refreshDataBaseLater()` 不被调用，设备信息数据库不刷新。

**历史记录要点**：

- `firstLoadFlag` 由 commit `38c2a19b`（"fix: 优化启动时间"，2022-11-21）引入，原始用途为首次加载后调用 `DBusInterface::getInstance()->refreshInfo()` + `QTimer::singleShot(500, start())` 二次启动。后续提交将行为改为仅 emit `finishedReadFilePool()` 信号。
- 最近一次涉及该文件的提交 `78d7e059`（"fix: resolve device generation timing disorder after S4 wake"，2026-10-22）未修改 `firstLoadFlag` 相关逻辑。

## 二、日志分析

无日志附件（本缺陷来自单元测试静态分析导出，非运行时日志触发）。

## 二（补充）、附件图片分析

无附件图片。

## 三、源码定位

### 缺陷位置

- **文件**: `src/GenerateDevice/LoadInfoThread.cpp:23`
- **缺陷类型**: 逻辑错误（变量作用域不当——文件级 static 导致跨实例共享）
- **问题代码**:

```cpp
// LoadInfoThread.cpp:23
static bool firstLoadFlag = true;

// LoadInfoThread.cpp:132-142
void LoadInfoThread::slotFinishedReadFilePool(const QString &)
{
    qCDebug(appLog) << "LoadInfoThread::slotFinishedReadFilePool";
    m_FinishedReadFilePool = true;
    // 首次加载刷新
    if (firstLoadFlag) {
        qCDebug(appLog) << "LoadInfoThread::slotFinishedReadFilePool first load, emit finishedReadFilePool";
        firstLoadFlag = false;
        emit finishedReadFilePool();
    }
}
```

### 缺陷说明

`firstLoadFlag` 声明为文件级 `static` 变量（`LoadInfoThread.cpp:23`），其存储期为整个程序运行期，被该编译单元内所有 `LoadInfoThread` 实例共享。`slotFinishedReadFilePool()` 在首次调用时将 `firstLoadFlag` 置为 `false` 并 emit `finishedReadFilePool()` 信号；此后同一进程内创建的任何新 `LoadInfoThread` 实例调用此槽函数时，`firstLoadFlag` 已为 `false`，不再 emit 信号。

对比：同类中 `m_FinishedReadFilePool`（`LoadInfoThread.h:45`）正确声明为实例成员 `bool`，每个实例独立维护。`firstLoadFlag` 应同样为实例成员，但误用为文件级 static。

### 关联项目/上游源码验证

未执行关联项目源码验证（根因在当前项目内）。

## 四、触发机理

### 因果链

1. **输入**：创建 `LoadInfoThread` 实例 A → `GetInfoPool::finishedAll` 信号触发 `slotFinishedReadFilePool()`
2. **路径**：`slotFinishedReadFilePool()` 检查 `firstLoadFlag`（文件级 static，初始 `true`）→ 置为 `false` → emit `finishedReadFilePool()`
3. **时序**：实例 A 销毁后，创建实例 B → `GetInfoPool::finishedAll` 再次触发 `slotFinishedReadFilePool()`
4. **缺陷表现**：实例 B 调用 `slotFinishedReadFilePool()` 时 `firstLoadFlag` 已为 `false`（被实例 A 修改），不 emit `finishedReadFilePool()`
5. **下游影响**：`MainWindow`（`MainWindow.cpp:144`）connect 的 lambda 不被触发 → `refreshDataBaseLater()` 不执行 → `DBusInterface::refreshInfo()` 不调用 → `refreshDataBase()` 不执行 → 设备信息数据库不刷新

### 5-Why 分析

| Level | Question | Answer | Evidence |
|-------|----------|--------|----------|
| Why 1 | 为什么新建 `LoadInfoThread` 实例不再 emit `finishedReadFilePool` 信号？ | 因为 `firstLoadFlag` 已被前一个实例设为 `false`，`if (firstLoadFlag)` 条件不成立 | `LoadInfoThread.cpp:137` `if (firstLoadFlag)` + `LoadInfoThread.cpp:139` `firstLoadFlag = false` |
| Why 2 | 为什么 `firstLoadFlag` 在新实例中已是 `false`？ | 因为 `firstLoadFlag` 是文件级 `static` 变量，存储期为整个程序运行期，所有实例共享同一份 | `LoadInfoThread.cpp:23` `static bool firstLoadFlag = true;` |
| Why 3 | 为什么开发者使用了文件级 static 而非实例成员？ | 原始设计意图是进程级"仅首次加载"刷新，而非实例级"每个实例首次加载"刷新。原始 commit `38c2a19b` 的行为是首次加载后调用 `refreshInfo()` + 延迟 `start()`，本质是一次性初始化逻辑。但随着代码演进（emit 信号替代直接调用），语义已从"进程级一次性"变为"实例级首次"，但变量作用域未同步调整 | git log: commit `38c2a19b` 引入 `firstLoadFlag`，原始逻辑为 `DBusInterface::getInstance()->refreshInfo()` + `QTimer::singleShot(500, start())`；当前代码改为 `emit finishedReadFilePool()` |

**5-Why 收敛于 Why 3（设计层根因）**，根因层修复指向系统性原因（Why 3），非单一 file:line 可修——需将变量从文件级 static 改为实例成员，并确认设计意图是实例级还是进程级。

## 五、版本后续修复检查

**未发现后续修复**。通过 `git log -p --all -S "firstLoadFlag" -- src/GenerateDevice/LoadInfoThread.cpp` 检查，`firstLoadFlag` 自 commit `38c2a19b`（2022-11-21）引入以来，无任何后续提交将其从文件级 static 改为实例成员或移除。最近涉及该文件的提交 `78d7e059`（2026-10-22）仅修改了 `generateDevice()` 相关逻辑，未触及 `firstLoadFlag`。缺陷在当前 master 分支 HEAD 仍然存在。

## 六、根因结论

**强根因**

代码缺陷定位（`LoadInfoThread.cpp:23` 文件级 static 变量）+ 触发机理（跨实例共享导致后续实例不 emit 信号）+ 复现路径（创建实例 A 触发槽函数 → 销毁 A → 创建实例 B 触发槽函数 → B 不 emit 信号）+ 已验证风险预测，四要素齐备。

5-Why 收敛于 Why 3（设计层根因）。

## 七、证据链

- [代码] `LoadInfoThread.cpp:23` — `static bool firstLoadFlag = true;`，文件级 static 变量，进程内所有 `LoadInfoThread` 实例共享
- [代码] `LoadInfoThread.cpp:137-141` — `if (firstLoadFlag) { firstLoadFlag = false; emit finishedReadFilePool(); }`，首次调用后永久关闭信号
- [代码] `LoadInfoThread.h:45` — `m_FinishedReadFilePool` 正确声明为实例成员 `bool`，对比证明 `firstLoadFlag` 应为实例成员
- [代码] `MainWindow.cpp:82` — `mp_WorkingThread(new LoadInfoThread)`，生产代码仅创建一个实例（低风险）
- [代码] `MainWindow.cpp:144-146` — `connect(mp_WorkingThread, &LoadInfoThread::finishedReadFilePool, this, [=]() { refreshDataBaseLater(); })`，信号消费方
- [代码] `ut_loadinfothread.cpp:24` — `m_loadInfoThread = new LoadInfoThread`，单元测试 SetUp 中反复创建实例（缺陷暴露场景）
- [git] commit `38c2a19b` — 引入 `firstLoadFlag`，原始逻辑为进程级一次性初始化
- [git] commit `78d7e059` — 最近修改该文件，未触及 `firstLoadFlag`

### 证伪检验记录

| # | 风险预测 | 核验方式 | 结果 |
|---|---------|---------|------|
| 1 | 若根因成立，`firstLoadFlag` 在整个代码库中仅在 `slotFinishedReadFilePool` 中被读写（无其他路径重置为 true） | `rg -n "firstLoadFlag" .` 搜索全仓库 | 通过——仅 3 处引用：声明（:23）、读取（:137）、写入（:139），无其他重置路径 |
| 2 | 若根因成立，`MainWindow` 在生产代码中仅创建一个 `LoadInfoThread` 实例（故生产环境不显现） | 读 `MainWindow.cpp:82` 及搜索 `new LoadInfoThread` 全仓库 | 通过——生产代码仅 `MainWindow.cpp:82` 一处 `new LoadInfoThread`，测试代码 `ut_loadinfothread.cpp:24` 一处 |
| 3 | 若根因成立，将 `firstLoadFlag` 改为实例成员后，单实例生产场景行为不变（每个实例的首次调用仍 emit 信号） | 代码路径分析：单实例下实例成员与 static 行为等价 | 通过——单实例场景 `firstLoadFlag` 只被读写一次，实例成员与 static 行为完全一致 |

**差分证据**: 无并列存活假设。唯一假设"文件级 static 导致跨实例共享"由代码静态分析直接证实，无需差分证据。

## 八、排除项

- **排除假设A**："`finishedReadFilePool` 信号本就不应对后续实例发出（设计意图为进程级一次性）"——理由：信号被 `MainWindow` connect 用于触发 `refreshDataBaseLater()` 刷新设备数据库，这是每个 `LoadInfoThread` 实例首次完成文件读取后都应执行的操作，而非进程级一次性操作。且原始 commit 的 `refreshInfo() + start()` 逻辑已被改为 emit 信号，语义已转向实例级。但此点需用户确认。
- **排除假设B**："存在其他代码路径手动重置 `firstLoadFlag`"——理由：`rg` 全仓库搜索仅 3 处引用（声明、读取、写入），无其他重置路径。

## 九、修复建议

### 根因层修复方案

将 `firstLoadFlag` 从文件级 static 改为 `LoadInfoThread` 类的实例成员 `m_FirstLoad`：

1. **`LoadInfoThread.h`**：在 private 成员区添加 `bool m_FirstLoad;`
2. **`LoadInfoThread.cpp`**：
   - 删除 `static bool firstLoadFlag = true;`（:23）
   - 构造函数初始化列表添加 `m_FirstLoad(true)`
   - `slotFinishedReadFilePool()` 中将 `firstLoadFlag` 替换为 `m_FirstLoad`

改动范围：2 个文件（`LoadInfoThread.h`、`LoadInfoThread.cpp`），4 处修改。

### 症状层补丁方案

根因层与症状层一致——本缺陷的直接原因（Why 1）和根本原因（Why 3）都指向同一处变量声明，修复方式相同。无独立症状层方案。

### 方案对比（五维特性评估）

| 特性 | 根因层修复 | 症状层补丁 |
|------|-----------|-----------|
| 安全 | 好：仅改变变量作用域，不引入空指针/越界/竞态 | 同根因层（方案一致） |
| 稳定 | 好：单实例场景行为不变，多实例场景修复信号缺失 | 同根因层 |
| 性能 | 好：无额外开销，bool 成员访问与 static 等价 | 同根因层 |
| 兼容 | 好：不改变对外接口/信号/DBus，`finishedReadFilePool` 信号签名不变 | 同根因层 |
| 可维护性 | 好：消除文件级 static，变量作用域与实例生命周期一致，符合 OOP 封装原则 | 同根因层 |

**改动范围**（补充）：根因层涉及 `LoadInfoThread.h` + `LoadInfoThread.cpp`，受影响调用方 1 个（`MainWindow`，行为修复而非接口变更）。

### 推荐方案

**推荐**: 根因层修复

**理由**: 五维评估均无"差"评级。将 `firstLoadFlag` 改为实例成员 `m_FirstLoad` 是最小化改动（2 文件 4 处），消除跨实例共享缺陷，且对单实例生产场景零行为变化。5-Why 收敛于 Why 3（设计层），根因层修复直接指向该系统性原因。

## 十、需要用户确认/补充的信息

- **设计意图确认**：`firstLoadFlag` 的"仅首次加载"语义应为**实例级**（每个 `LoadInfoThread` 实例首次完成文件读取时 emit 信号）还是**进程级**（整个进程生命周期仅 emit 一次）？分析认为应为实例级（信号用于触发数据库刷新，是每个实例首次加载后都应执行的操作），但需用户确认。若确为进程级，则当前代码无缺陷，但建议补充注释说明设计意图。
