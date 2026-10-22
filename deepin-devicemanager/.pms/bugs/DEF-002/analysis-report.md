# BUG-DEF-002 根因分析

- **Bug ID**: DEF-002
- **标题**: clear() 未复位 m_BusIdList
- **PMS URL**: 无（multica issue V-6368）
- **状态**: 问题分析中
- **Bug 类型**: D1 功能逻辑缺陷
- **分析时间**: 2026-10-23
- **修复提交**: 未提取到
- **涉及项目**: deepin-devicemanager
- **工作模式**: multica 模式（上下文）

## 一、问题概述

`DeviceManager::clear()` 方法负责在每次设备信息刷新周期开始时清空所有设备数据结构。该方法清空了 `m_cmdInfo`、所有 `m_ListDevice*` 列表以及 `m_DeviceClassMap`，但遗漏了 `m_BusIdList`。由于 `addBusId()` 仅执行 `append` 操作，从不先清空，导致每次刷新周期后旧的总线 ID 在 `m_BusIdList` 中持续累积，不会被清除。

`m_BusIdList` 被 `DeviceGenerator` 用于判断设备是否已在其他类别中显示过（通过 `getBusId()` 获取列表后做 `indexOf` 匹配）。旧总线 ID 残留会导致刷新后设备被误判为"已显示过"，从而影响设备列表的完整性和一致性。

**历史记录要点**：

- 无开发人员历史备注（缺陷由父 Issue V-6208 单元测试 Mode 5 导出）

## 二、日志分析（可选）

无日志附件。

## 二（补充）、附件图片分析

无附件图片。

## 三、源码定位

### 缺陷位置

- **文件**: `src/DeviceManager/DeviceManager.cpp:57-140`
- **缺陷类型**: 逻辑错误（遗漏复位）
- **问题代码**:

```cpp
// DeviceManager.cpp:57-140
void DeviceManager::clear()
{
    qCDebug(appLog) << "Starting to clear all device resources";
    
    // 清除所有命令
    m_cmdInfo.clear();

    // 清除内存中的所有设备指针（省略 foreach delete ...）
    // ...

    // 清空存储设备指针的列表
    m_ListDeviceMouse.clear();
    m_ListDeviceStorage.clear();
    // ... 所有 m_ListDevice*.clear() ...
    m_ListDeviceCPU.clear();
    m_DeviceClassMap.clear();
    qCDebug(appLog) << "All device resources cleared successfully";
    // ↑ 缺少 m_BusIdList.clear();
}
```

```cpp
// DeviceManager.cpp:1738-1743
void DeviceManager::addBusId(const QStringList &busId)
{
    m_BusIdList.append(busId);  // 仅 append，从不先清空
}
```

### 缺陷说明

`clear()` 方法（第 57-140 行）系统性地清空了所有设备列表和映射，但唯独遗漏了 `m_BusIdList`。`m_BusIdList` 是 `QStringList` 类型成员变量（`DeviceManager.h:674`），用于存储所有设备总线 ID。

`addBusId()`（第 1738 行）仅执行 `m_BusIdList.append(busId)`，没有任何清空逻辑。`clear()` 是唯一的复位入口（由 `GetInfoPool::getAllInfo()` 在每次刷新周期开始时调用，`GetInfoPool.cpp:54`），因此 `m_BusIdList` 在多次刷新后不断增长，旧总线 ID 永远不会被清除。

### 关联项目/上游源码验证

未执行关联项目源码验证（根因在当前项目内）。

## 四、触发机理

1. 用户触发设备信息刷新（`GetInfoPool::getAllInfo()` 被调用）
2. `getAllInfo()` 第一行调用 `DeviceManager::instance()->clear()` 清空所有设备数据
3. `clear()` 清空了 `m_cmdInfo`、所有 `m_ListDevice*`、`m_DeviceClassMap`，但**未清空 `m_BusIdList`**
4. 各设备生成器开始工作，通过 `GenerateDevicePool::slotFinished()` 调用 `DeviceManager::instance()->addBusId(lst)` 将新的总线 ID **追加**到 `m_BusIdList`
5. `DeviceGenerator` 在处理 hwinfo 数据时，通过 `DeviceManager::instance()->getBusId()` 获取 `m_BusIdList`，用 `lstBusId.indexOf(curBus)` 判断设备是否已在其他类别中显示（`DeviceGenerator.cpp:1394-1403`）
6. 由于旧总线 ID 残留，`indexOf` 可能命中上一轮的过期总线 ID，导致当前轮次中本应被归为"其他设备"的设备被误判为"已在其他类别显示"，从而被跳过或错误分类

### 5-Why 分析

- **Why 1**: 为什么设备列表可能出现重复或状态不一致？
  → `m_BusIdList` 中残留了上一刷新周期的旧总线 ID，干扰了 `DeviceGenerator` 的设备分类判断。
  - 证据: `DeviceGenerator.cpp:1394-1403` 使用 `getBusId()` 返回值做 `indexOf` 匹配

- **Why 2**: 为什么 `m_BusIdList` 中有旧总线 ID？
  → `addBusId()` 仅执行 `append`，`clear()` 未复位 `m_BusIdList`。
  - 证据: `DeviceManager.cpp:1742` `m_BusIdList.append(busId)`；`clear()` 第 57-140 行无 `m_BusIdList.clear()`

- **Why 3**: 为什么 `clear()` 遗漏了 `m_BusIdList`？
  → `m_BusIdList` 是在设备生成阶段动态填充的成员，与设备列表类成员同级声明在 `DeviceManager.h:674`，但 `clear()` 方法在实现时未将其纳入清理范围——这是一个编码遗漏，`clear()` 的设计意图是清空所有状态以准备下一轮刷新，但 `m_BusIdList` 被漏掉了。
  - 证据: `clear()` 清空了同级的 `m_cmdInfo`（`DeviceManager.h:675`）、`m_DeviceClassMap`（`DeviceManager.h:677`），唯独跳过了 `m_BusIdList`（`DeviceManager.h:674`）

5-Why 在 Why 3 收敛到编码遗漏（系统性根因）。

## 五、版本后续修复检查

**结论**: 未发现后续修复。

检查依据：通过 `git log --all --oneline -- src/DeviceManager/DeviceManager.cpp` 查看该文件全部提交历史，未发现任何提交在 `clear()` 方法中添加了 `m_BusIdList.clear()`。当前 HEAD（`f2d46ee9`）的 `clear()` 方法（第 57-140 行）仍不包含 `m_BusIdList.clear()`，缺陷仍然存在。

## 六、根因结论

**结论级别**: 强根因

`clear()` 方法遗漏了 `m_BusIdList.clear()`，导致 `m_BusIdList` 在多次刷新周期中持续累积旧总线 ID。`DeviceGenerator` 依赖 `getBusId()` 判断设备是否已在其他类别显示，旧总线 ID 残留会干扰此判断，导致设备被误分类或遗漏。

### 证据链

- [源码] `DeviceManager.cpp:57-140` — `clear()` 方法清空了 `m_cmdInfo`、所有 `m_ListDevice*`、`m_DeviceClassMap`，但无 `m_BusIdList.clear()`
- [源码] `DeviceManager.cpp:1738-1742` — `addBusId()` 仅 `append`，无清空逻辑
- [源码] `GetInfoPool.cpp:54` — `getAllInfo()` 调用 `clear()` 作为刷新起点，证明 `clear()` 是复位入口
- [源码] `GenerateDevicePool.cpp:204` — `slotFinished()` 调用 `addBusId()`，新总线 ID 追加到未清空的 `m_BusIdList`
- [源码] `DeviceGenerator.cpp:1394-1403` — `getBusId()` 返回值用于 `indexOf` 判断设备是否已显示
- [源码] `DeviceManager.h:674` — `m_BusIdList` 声明，`QStringList` 类型
- [日志] 无
- [历史记录] 无
- [fix_commit] 无

### 证伪检验记录

| # | 风险预测 | 核验方式 | 结果 |
|---|---------|---------|------|
| 1 | 若根因成立，`clear()` 方法中不应存在 `m_BusIdList.clear()` 调用 | 读 `DeviceManager.cpp:57-140` | 通过（确认不存在） |
| 2 | 若根因成立，`addBusId()` 应仅执行 append 而无 clear 逻辑 | 读 `DeviceManager.cpp:1738-1743` | 通过（仅 append） |
| 3 | 若根因成立，`m_BusIdList` 应被 `DeviceGenerator` 使用且依赖其准确性 | 读 `DeviceGenerator.cpp:1394-1403` | 通过（用于 indexOf 判断设备分类） |

**差分证据**: 仅有一个存活假设（clear() 遗漏 m_BusIdList.clear()），无并列假设需要区分。

## 七、排除项

- **排除假设A（m_BusIdList 在别处被清空）**: 全局搜索 `m_BusIdList` 的所有引用（`DeviceManager.cpp:1742` append、`DeviceManager.cpp:1748` return、`DeviceManager.h:674` 声明），以及测试文件中的手动 `m_BusIdList.clear()`（仅存在于 `ut_devicemanager.cpp:658,669` 的测试代码中），确认生产代码中无任何地方清空 `m_BusIdList`。

## 八、修复建议

### 根因层修复方案

根因层与症状层一致，无独立根因层方案。5-Why 在 Why 3 收敛，但根因（编码遗漏）与直接原因（clear() 未清空 m_BusIdList）指向同一处修改，两层重合。

### 症状层补丁方案

在 `DeviceManager::clear()` 方法中，紧随 `m_DeviceClassMap.clear()` 之后（第 138 行后），添加 `m_BusIdList.clear();`。

改动范围：仅 `src/DeviceManager/DeviceManager.cpp` 一行新增，无接口变更，无其他文件修改。

```cpp
// 在 clear() 方法末尾，m_DeviceClassMap.clear(); 之后添加：
m_BusIdList.clear();
```

### 方案对比（五维特性评估）

| 特性 | 根因层修复 | 症状层补丁 |
|------|-----------|-----------|
| 安全 | 好：不引入新风险，仅清空一个 QStringList | 好：同左 |
| 稳定 | 好：修复后每次刷新 m_BusIdList 正确复位，消除累积 | 好：同左 |
| 性能 | 好：QStringList::clear() 是 O(n) 操作，开销极小 | 好：同左 |
| 兼容 | 好：不改变任何对外接口、信号、DBus 接口 | 好：同左 |
| 可维护性 | 好：与 clear() 中其他 .clear() 调用风格一致，一行修复 | 好：同左 |

**改动范围**（补充）：根因层与症状层重合，仅涉及 `src/DeviceManager/DeviceManager.cpp` 一处一行新增。

### 推荐方案

**推荐**: 症状层补丁（与根因层重合，仅一处一行修改）

**理由**: 两层方案完全重合，在 `clear()` 方法中添加 `m_BusIdList.clear();` 即可。五维评估均为"好"，改动极小且与现有代码风格一致，无回归风险。

## 十、需要用户确认/补充的信息（若结论非强根因）

不适用（结论为强根因）。

---

<!-- 报告写完后，向用户展示摘要：根因结论 + 关键证据（最多3条）+ 修复建议。询问用户是否认可。 -->
