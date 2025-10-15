# 水平约束实现说明

## 用户请求

用户要求"水平约束"触发`EaSession::createHorizontalConstraint`函数，请完善`SLVS_C_HORIZONTAL`约束。

## 实现内容

### 1. EaSession::createHorizontalConstraint函数

**文件**: `main/easession.cpp`

```cpp
void EaSession::createHorizontalConstraint()
{
    this->clear();
    
    // 创建一条线段，初始不是水平的
    int pt1 = this->addPoint(50.0, 80.0);    // 线段起点
    int pt2 = this->addPoint(150.0, 120.0);  // 线段终点（不是水平）
    
    // 创建线段
    int line1 = this->addLine(pt1, pt2);
    
    // 固定一个点以稳定约束系统
    this->createFixPointConstraint(pt1);     // 固定线段起点
    
    // 添加水平约束
    this->addHorizontalConstraint(line1);
    
    qDebug() << "EaSession: Created horizontal constraint for line" << line1;
}
```

### 2. EaSession::addHorizontalConstraint函数

**文件**: `main/easession.h` 和 `main/easession.cpp`

```cpp
// 头文件声明
void addHorizontalConstraint(int lineId);

// 实现文件
void EaSession::addHorizontalConstraint(int lineId)
{
    // 验证线段是否存在
    EaLine* line = getLine(lineId);
    
    if (!line) {
        qWarning() << "EaSession: Cannot add horizontal constraint - invalid line ID:" << lineId;
        return;
    }
    
    // 添加水平约束
    Constraint horizontalConstraint(m_nextConstraintId++, "horizontal");
    horizontalConstraint.data["line"] = lineId;
    
    m_constraints.push_back(horizontalConstraint);
    
    qDebug() << "EaSession: Added horizontal constraint" << horizontalConstraint.id 
             << "for line" << lineId;
}
```

### 3. GeometrySolver中的水平约束处理

**文件**: `main/eageosolver.cpp`

```cpp
else if (type == "horizontal") {
    int lineId = std::any_cast<int>(constraint.data.at("line"));
    
    if (lineToEntity.find(lineId) != lineToEntity.end()) {
        
        // 添加水平约束
        int constraintId = m_sys.constraints + 1;
        Slvs_Constraint constraint = Slvs_MakeConstraint(
            constraintId, g,
            SLVS_C_HORIZONTAL,
            200,
            0.0,
            0, 0, lineToEntity[lineId], 0);
        constraint.entityC = 0;
        constraint.entityD = 0;
        m_sys.constraint[m_sys.constraints++] = constraint;
        
        qDebug() << "GeometrySolver: Added horizontal constraint" << constraintId
                 << "for line" << lineId << "entity" << lineToEntity[lineId];
    } else {
        qWarning() << "GeometrySolver: Cannot add horizontal constraint - missing line entity" << lineId;
    }
}
```

### 4. QML界面更新

**文件**: `main.qml`

```qml
Button {
    text: "水平约束"
    Layout.fillWidth: true
    enabled: drawingArea !== null
    onClicked: {
        globalSession.createHorizontalConstraint()
        statusText.text = "水平约束已创建"
        statusText.color = "green"
    }
}
```

## 几何布局

### 点的位置
- **P1**: (50, 80) - 线段起点，**固定**
- **P2**: (150, 120) - 线段终点，**可移动**

### 线段配置
- **线段1**: P1-P2 (初始不是水平，会被约束为水平)

### 约束系统
- **固定约束**: P1被固定
- **水平约束**: 线段1被约束为水平

## 自由度分析

### 总自由度
- 2个点 × 2 = **4个自由度**

### 约束减少的自由度
- **水平约束**: 减少**1个自由度**
- **固定P1**: 减少**2个自由度**

### 剩余自由度
```
剩余自由度 = 4 - 1 - 2 = 1个自由度
```

### 剩余自由度的含义
- **P2可以沿水平方向移动**：调整线段的长度
- **线段保持水平**：无论P2如何移动，线段都保持水平

## 预期行为

### 点击"水平约束"按钮后
1. 清空现有几何元素
2. 创建2个点和1条线段
3. 添加固定约束和水平约束
4. 线段会被约束为水平

### 拖拽行为
- **拖拽P1**: 由于固定约束，不能移动
- **拖拽P2**: 可以沿水平方向移动，调整线段长度
- **线段保持水平**: 无论P2如何移动，线段都保持水平

## 调试信息

系统会输出详细的调试信息：
```
EaSession: Created horizontal constraint for line 1
EaSession: Added horizontal constraint 1 for line 1
GeometrySolver: Added horizontal constraint 1 for line 1 entity 300
```

## 相关文件修改

1. **`main/easession.h`**: 添加`addHorizontalConstraint`函数声明
2. **`main/easession.cpp`**: 
   - 实现`createHorizontalConstraint`函数
   - 实现`addHorizontalConstraint`函数
3. **`main/eageosolver.cpp`**: 添加"horizontal"约束类型处理
4. **`main.qml`**: 更新按钮点击事件

## 技术细节

### 约束类型
- 使用`SLVS_C_HORIZONTAL`约束类型
- 约束线段为水平

### 参数设置
- **constraintId**: 自动生成的约束ID
- **g**: 组ID（Group 2）
- **SLVS_C_HORIZONTAL**: 水平约束类型
- **200**: 工作平面ID
- **0.0**: 约束值（水平约束不需要值）
- **lineToEntity[lineId]**: 线段的实体ID

### 约束效果
- 线段会被强制保持水平
- P2可以沿水平方向移动
- 线段长度可以调整，但方向保持水平

## 与垂直约束的对比

### 垂直约束
- 需要2条线段
- 约束两条线段垂直
- 需要2个固定点

### 水平约束
- 只需要1条线段
- 约束线段水平
- 只需要1个固定点

水平约束比垂直约束更简单，因为它只涉及一条线段的方向约束。

现在水平约束功能已经完全实现，用户可以点击"水平约束"按钮来创建一条水平的线段！
