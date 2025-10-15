# 平行约束崩溃问题修复 V2

## 问题描述

程序在拖拽任意点时仍然崩溃，错误信息：
```
File D:\qt\SolveSpaceLib\libslvs\dsc.h, line 334, function FindById:
Assertion 'Cannot find handle' failed: ((t != NULL) == false).
```

## 深入分析

### 1. 约束结构体字段问题
通过分析`Slvs_Constraint`结构体定义，发现它包含以下字段：
```cpp
typedef struct {
    Slvs_hConstraint    h;
    Slvs_hGroup         group;
    int                 type;
    Slvs_hEntity        wrkpl;
    double              valA;
    Slvs_hEntity        ptA;
    Slvs_hEntity        ptB;
    Slvs_hEntity        entityA;
    Slvs_hEntity        entityB;
    Slvs_hEntity        entityC;  // 这些字段没有被Slvs_MakeConstraint设置
    Slvs_hEntity        entityD;  // 这些字段没有被Slvs_MakeConstraint设置
    int                 other;
    int                 other2;
} Slvs_Constraint;
```

但是`Slvs_MakeConstraint`函数只设置了前8个字段，没有设置`entityC`和`entityD`字段。

### 2. 参数ID冲突问题
工作平面使用了参数ID 1-7，而我们的点参数也从1开始，导致ID冲突。

### 3. 约束ID生成问题
约束ID的生成逻辑可能导致ID冲突。

## 修复方案

### 1. 修复约束结构体字段设置
```cpp
// 修复前：直接使用Slvs_MakeConstraint
m_sys.constraint[m_sys.constraints++] = Slvs_MakeConstraint(...);

// 修复后：手动设置所有字段
Slvs_Constraint constraint = Slvs_MakeConstraint(...);
constraint.entityC = 0;
constraint.entityD = 0;
m_sys.constraint[m_sys.constraints++] = constraint;
```

### 2. 修复参数ID冲突
```cpp
// 修复前：参数ID从1开始，与工作平面冲突
int paramIndex = 1;

// 修复后：参数ID从10开始，避免冲突
int paramIndex = 10;  // 从10开始，避免与工作平面参数ID冲突
```

### 3. 保持约束ID生成逻辑
```cpp
// 使用m_sys.constraints + 1生成约束ID
int constraintId = m_sys.constraints + 1;
```

## 修复后的代码结构

### 约束创建流程
1. 创建工作平面（参数ID 1-7，实体ID 101-102, 200）
2. 创建点参数（参数ID 10+，实体ID 300+）
3. 创建线段实体（实体ID 300+）
4. 创建约束（约束ID 1+）

### 参数ID分配
- 工作平面：1-7
- 点参数：10-17（4个点，每个点2个参数）
- 约束：1+

### 实体ID分配
- 工作平面：101-102, 200
- 点实体：300-303
- 线段实体：304-305

## 预期效果

修复后应该能够：
1. 正常创建平行约束
2. 拖拽任意点而不崩溃
3. 保持两条线段的平行关系
4. 正确求解约束系统

## 测试验证

### 测试步骤
1. 点击"平行约束"按钮创建场景
2. 点击"测试拖拽约束"按钮测试功能
3. 观察控制台输出，确认没有崩溃

### 预期调试输出
```
EaSession: Created parallel constraint with points 1 2 3 4
EaSession: Created lines 1 and 2 with parallel constraint
GeometrySolver: Created point 1 at 50 50 with params 10 11
GeometrySolver: Created point 2 at 150 100 with params 12 13
GeometrySolver: Created point 3 at 50 150 with params 14 15
GeometrySolver: Created point 4 at 150 200 with params 16 17
GeometrySolver: Created line 1 from point 1 to point 2 with entity 304
GeometrySolver: Created line 2 from point 3 to point 4 with entity 305
GeometrySolver: Added parallel constraint 1 between lines 1 and 2 entities 304 305
GeometrySolver: Total constraints added: 1
GeometrySolver: Dragged point 1 parameters: 10 11
```

## 关键改进

1. **完整的约束结构体设置**: 确保所有字段都被正确初始化
2. **避免参数ID冲突**: 使用不同的参数ID范围
3. **正确的实体ID管理**: 确保实体ID不冲突
4. **清晰的调试输出**: 便于问题诊断

这个修复应该彻底解决崩溃问题，让平行约束功能能够正常工作。
