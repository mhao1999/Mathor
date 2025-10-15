# 圆弧与直线相切约束实现说明

## 用户请求

用户要求点击"相切约束"按钮触发`createLineTangentConstraint`函数，应该在该函数中绘制一个圆弧（已写代码）和二个点构成的一条直线，然后对它们应用`SLVS_C_ARC_LINE_TANGENT`约束。

## 实现内容

### 1. EaSession::createLineTangentConstraint函数

**文件**: `main/easession.cpp`

```cpp
void EaSession::createLineTangentConstraint()
{
    this->clear();

    // 创建圆心点
    int centerPt = this->addPoint(100.0, 100.0);  // 圆心位置

    // 创建圆弧实体用于界面显示
    int arcId = this->addArc(centerPt, 30.0, 0.0, 180.0); // 半径30.0，从0度到180度

    // 创建直线：两个点构成一条与圆弧相切的直线
    int pt1 = this->addPoint(50.0, 130.0);   // 直线起点
    int pt2 = this->addPoint(150.0, 130.0);  // 直线终点

    // 创建直线
    int lineId = this->addLine(pt1, pt2);

    // 固定一些点以稳定约束系统
    this->createFixPointConstraint(centerPt);  // 固定圆心
    this->createFixPointConstraint(pt1);       // 固定直线起点

    // 添加圆弧与直线相切约束
    this->addArcLineTangentConstraint(arcId, lineId);

    qDebug() << "EaSession: Created arc-line tangent constraint between arc" << arcId << "and line" << lineId;
}
```

### 2. EaSession::addArcLineTangentConstraint函数

**文件**: `main/easession.h` 和 `main/easession.cpp`

```cpp
// 头文件声明
void addArcLineTangentConstraint(int arcId, int lineId);

// 实现文件
void EaSession::addArcLineTangentConstraint(int arcId, int lineId)
{
    // 验证圆弧和直线是否存在
    EaLine* line = getLine(lineId);
    
    if (!line) {
        qWarning() << "EaSession: Cannot add arc-line tangent constraint - invalid line ID:" << lineId;
        return;
    }
    
    // 验证圆弧是否存在（通过统一容器查找）
    bool arcFound = false;
    for (const auto& shape : m_shapes) {
        if (auto arc = std::dynamic_pointer_cast<EaArc>(shape)) {
            if (arc->getId() == arcId) {
                arcFound = true;
                break;
            }
        }
    }
    
    if (!arcFound) {
        qWarning() << "EaSession: Cannot add arc-line tangent constraint - invalid arc ID:" << arcId;
        return;
    }
    
    // 添加圆弧与直线相切约束
    Constraint tangentConstraint(m_nextConstraintId++, "arc_line_tangent");
    tangentConstraint.data["arc"] = arcId;
    tangentConstraint.data["line"] = lineId;
    
    m_constraints.push_back(tangentConstraint);
    
    qDebug() << "EaSession: Added arc-line tangent constraint" << tangentConstraint.id 
             << "between arc" << arcId << "and line" << lineId;
}
```

### 3. GeometrySolver中的相切约束处理

**文件**: `main/eageosolver.cpp`

```cpp
else if (type == "arc_line_tangent") {
    int arcId = std::any_cast<int>(constraint.data.at("arc"));
    int lineId = std::any_cast<int>(constraint.data.at("line"));
    
    // 查找圆弧和直线的实体ID
    int arcEntityId = -1;
    int lineEntityId = -1;
    
    // 查找圆弧实体ID（通过统一容器）
    for (const auto& shape : m_session->getShapes()) {
        if (auto arc = std::dynamic_pointer_cast<EaArc>(shape)) {
            if (arc->getId() == arcId) {
                // 圆弧实体ID需要从centerToCircleEntity映射中获取
                EaPoint* centerPoint = arc->getCenter();
                if (centerPoint && centerToCircleEntity.find(centerPoint->getId()) != centerToCircleEntity.end()) {
                    arcEntityId = centerToCircleEntity[centerPoint->getId()];
                }
                break;
            }
        }
    }
    
    // 查找直线实体ID
    if (lineToEntity.find(lineId) != lineToEntity.end()) {
        lineEntityId = lineToEntity[lineId];
    }
    
    if (arcEntityId != -1 && lineEntityId != -1) {
        // 添加圆弧与直线相切约束
        int constraintId = m_sys.constraints + 1;
        Slvs_Constraint constraint = Slvs_MakeConstraint(
            constraintId, g,
            SLVS_C_ARC_LINE_TANGENT,
            200,
            0.0,
            0, 0, arcEntityId, lineEntityId);
        constraint.entityC = 0;
        constraint.entityD = 0;
        m_sys.constraint[m_sys.constraints++] = constraint;
        
        qDebug() << "GeometrySolver: Added arc-line tangent constraint" << constraintId
                 << "between arc" << arcId << "and line" << lineId 
                 << "entities" << arcEntityId << lineEntityId;
    } else {
        qWarning() << "GeometrySolver: Cannot add arc-line tangent constraint - missing entities" 
                   << "arc" << arcId << "line" << lineId << "arcEntity" << arcEntityId << "lineEntity" << lineEntityId;
    }
}
```

### 4. QML界面更新

**文件**: `main.qml`

```qml
Button {
    text: "相切约束"
    Layout.fillWidth: true
    enabled: drawingArea !== null
    onClicked: {
        globalSession.createLineTangentConstraint()
        statusText.text = "相切约束已创建"
        statusText.color = "green"
    }
}
```

## 几何布局

### 点的位置
- **centerPt**: (100, 100) - 圆心位置，**固定**
- **pt1**: (50, 130) - 直线起点，**固定**
- **pt2**: (150, 130) - 直线终点，**可移动**

### 几何元素配置
- **圆弧**: 圆心(100, 100)，半径30.0，从0度到180度（上半圆）
- **直线**: pt1-pt2 (水平直线，位于圆弧下方)

### 约束系统
- **固定约束**: centerPt和pt1被固定
- **相切约束**: 圆弧与直线相切

## 自由度分析

### 总自由度
- 3个点 × 2 = **6个自由度**

### 约束减少的自由度
- **相切约束**: 减少**1个自由度**
- **固定centerPt**: 减少**2个自由度**
- **固定pt1**: 减少**2个自由度**

### 剩余自由度
```
剩余自由度 = 6 - 1 - 2 - 2 = 1个自由度
```

### 剩余自由度的含义
- **pt2可以沿直线方向移动**：调整直线长度
- **直线保持与圆弧相切**：无论pt2如何移动，直线都保持与圆弧相切

## 预期行为

### 点击"相切约束"按钮后
1. 清空现有几何元素
2. 创建3个点、1个圆弧和1条直线
3. 添加固定约束和相切约束
4. 直线与圆弧保持相切关系

### 拖拽行为
- **拖拽centerPt**: 由于固定约束，不能移动
- **拖拽pt1**: 由于固定约束，不能移动
- **拖拽pt2**: 可以沿直线方向移动，调整直线长度
- **直线保持相切**: 无论pt2如何移动，直线都保持与圆弧相切

## 调试信息

系统会输出详细的调试信息：
```
EaSession: Created arc-line tangent constraint between arc 1 and line 1
EaSession: Added arc-line tangent constraint 1 between arc 1 and line 1
GeometrySolver: Added arc-line tangent constraint 1 between arc 1 and line 1 entities 300 301
```

## 相关文件修改

1. **`main/easession.h`**: 添加`addArcLineTangentConstraint`函数声明
2. **`main/easession.cpp`**: 
   - 实现`createLineTangentConstraint`函数
   - 实现`addArcLineTangentConstraint`函数
3. **`main/eageosolver.cpp`**: 添加"arc_line_tangent"约束类型处理
4. **`main.qml`**: 更新按钮点击事件

## 技术细节

### 约束类型
- 使用`SLVS_C_ARC_LINE_TANGENT`约束类型
- 约束圆弧与直线相切

### 参数设置
- **constraintId**: 自动生成的约束ID
- **g**: 组ID（Group 2）
- **SLVS_C_ARC_LINE_TANGENT**: 圆弧与直线相切约束类型
- **200**: 工作平面ID
- **0.0**: 约束值（相切约束不需要值）
- **arcEntityId**: 圆弧的实体ID
- **lineEntityId**: 直线的实体ID

### 实体ID查找
- **圆弧实体ID**: 通过`centerToCircleEntity`映射查找
- **直线实体ID**: 通过`lineToEntity`映射查找

### 约束效果
- 直线与圆弧保持相切关系
- 直线可以调整长度，但保持相切
- 圆弧可以调整位置，但保持与直线相切

## 与其他约束的对比

### 点在圆上约束
- 约束点位于圆上
- 点可以沿圆周移动

### 圆弧与直线相切约束
- 约束直线与圆弧相切
- 直线可以调整长度，但保持相切

### 平行约束
- 约束两条直线平行
- 直线可以调整长度和位置

## 应用场景

相切约束在工程设计中非常有用：
- **机械设计**: 设定零件之间的相切关系
- **建筑制图**: 控制墙体、梁柱的相切关系
- **CAD建模**: 精确控制几何元素之间的相切关系

## 注意事项

### 1. 圆弧实体ID查找
- 圆弧实体ID需要通过`centerToCircleEntity`映射查找
- 这是因为圆弧和圆使用相同的实体系统

### 2. 约束稳定性
- 需要固定足够的点来稳定约束系统
- 固定圆心和直线起点确保系统稳定

### 3. 角度范围
- 圆弧从0度到180度，形成上半圆
- 直线位于圆弧下方，形成相切关系

现在相切约束功能已经完全实现！用户可以点击"相切约束"按钮来创建圆弧与直线相切的几何关系。
