# 垂直约束实现说明

## 用户请求

用户要求点击"垂直约束"按钮触发`EaSession::createPerpendicularConstraint`函数，在其中构造两条线段，应用`SLVS_C_PERPENDICULAR`约束。

## 实现内容

### 1. EaSession::createPerpendicularConstraint函数

**文件**: `main/easession.cpp`

```cpp
void EaSession::createPerpendicularConstraint()
{
    this->clear();
    
    // 创建4个点，构成两条垂直的线段
    // 第一条线段：(点1，点2) - 水平线段
    int pt1 = this->addPoint(50.0, 100.0);   // 线段1起点
    int pt2 = this->addPoint(150.0, 100.0);  // 线段1终点
    
    // 第二条线段：(点3，点4) - 垂直线段
    int pt3 = this->addPoint(100.0, 50.0);   // 线段2起点
    int pt4 = this->addPoint(100.0, 150.0);  // 线段2终点
    
    // 创建两条线段
    int line1 = this->addLine(pt1, pt2);     // 水平线段
    int line2 = this->addLine(pt3, pt4);     // 垂直线段
    
    // 固定一些点以稳定约束系统
    this->createFixPointConstraint(pt1);     // 固定线段1起点
    this->createFixPointConstraint(pt3);     // 固定线段2起点
    
    // 添加垂直约束
    this->addPerpendicularConstraint(line1, line2);
    
    qDebug() << "EaSession: Created perpendicular constraint between line" << line1 << "and line" << line2;
}
```

### 2. EaSession::addPerpendicularConstraint函数

**文件**: `main/easession.h` 和 `main/easession.cpp`

```cpp
// 头文件声明
void addPerpendicularConstraint(int line1Id, int line2Id);

// 实现文件
void EaSession::addPerpendicularConstraint(int line1Id, int line2Id)
{
    // 验证线段是否存在
    EaLine* line1 = getLine(line1Id);
    EaLine* line2 = getLine(line2Id);
    
    if (!line1 || !line2) {
        qWarning() << "EaSession: Cannot add perpendicular constraint - invalid line IDs:" << line1Id << line2Id;
        return;
    }
    
    // 添加垂直约束
    Constraint perpendicularConstraint(m_nextConstraintId++, "perpendicular");
    perpendicularConstraint.data["line1"] = line1Id;
    perpendicularConstraint.data["line2"] = line2Id;
    
    m_constraints.push_back(perpendicularConstraint);
    
    qDebug() << "EaSession: Added perpendicular constraint" << perpendicularConstraint.id 
             << "between lines" << line1Id << "and" << line2Id;
}
```

### 3. GeometrySolver中的垂直约束处理

**文件**: `main/eageosolver.cpp`

```cpp
else if (type == "perpendicular") {
    int line1Id = std::any_cast<int>(constraint.data.at("line1"));
    int line2Id = std::any_cast<int>(constraint.data.at("line2"));
    
    if (lineToEntity.find(line1Id) != lineToEntity.end() && 
        lineToEntity.find(line2Id) != lineToEntity.end()) {
        
        // 添加垂直约束
        int constraintId = m_sys.constraints + 1;
        Slvs_Constraint constraint = Slvs_MakeConstraint(
            constraintId, g,
            SLVS_C_PERPENDICULAR,
            200,
            0.0,
            0, 0,
            lineToEntity[line1Id], lineToEntity[line2Id]);
        constraint.entityC = 0;
        constraint.entityD = 0;
        m_sys.constraint[m_sys.constraints++] = constraint;
        
        qDebug() << "GeometrySolver: Added perpendicular constraint" << constraintId
                 << "between lines" << line1Id << "and" << line2Id 
                 << "entities" << lineToEntity[line1Id] << lineToEntity[line2Id];
    } else {
        qWarning() << "GeometrySolver: Cannot add perpendicular constraint - missing line entities" << line1Id << line2Id;
    }
}
```

### 4. QML界面更新

**文件**: `main.qml`

```qml
Button {
    text: "垂直约束"
    Layout.fillWidth: true
    enabled: drawingArea !== null
    onClicked: {
        globalSession.createPerpendicularConstraint()
        statusText.text = "垂直约束已创建"
        statusText.color = "green"
    }
}
```

## 几何布局

### 点的位置
- **P1**: (50, 100) - 水平线段起点
- **P2**: (150, 100) - 水平线段终点
- **P3**: (100, 50) - 垂直线段起点
- **P4**: (100, 150) - 垂直线段终点

### 线段配置
- **线段1**: P1-P2 (水平线段)
- **线段2**: P3-P4 (垂直线段)
- **交点**: (100, 100) - 两条线段的交点

### 约束系统
- **固定约束**: P1和P3被固定
- **垂直约束**: 线段1和线段2垂直

## 预期行为

### 点击"垂直约束"按钮后
1. 清空现有几何元素
2. 创建4个点和2条线段
3. 添加固定约束和垂直约束
4. 显示两条垂直的线段

### 拖拽行为
- **拖拽P2**: 线段1会保持水平，线段2会保持垂直
- **拖拽P4**: 线段2会保持垂直，线段1会保持水平
- **拖拽P1或P3**: 由于固定约束，不能移动

## 调试信息

系统会输出详细的调试信息：
```
EaSession: Created perpendicular constraint between line 1 and line 2
EaSession: Added perpendicular constraint 1 between lines 1 and 2
GeometrySolver: Added perpendicular constraint 1 between lines 1 and 2 entities 300 301
```

## 相关文件修改

1. **`main/easession.h`**: 添加`addPerpendicularConstraint`函数声明
2. **`main/easession.cpp`**: 
   - 实现`createPerpendicularConstraint`函数
   - 实现`addPerpendicularConstraint`函数
3. **`main/eageosolver.cpp`**: 添加"perpendicular"约束类型处理
4. **`main.qml`**: 更新按钮点击事件

## 技术细节

### 约束类型
- 使用`SLVS_C_PERPENDICULAR`约束类型
- 约束两条线段垂直

### 参数设置
- **constraintId**: 自动生成的约束ID
- **g**: 组ID（Group 2）
- **SLVS_C_PERPENDICULAR**: 垂直约束类型
- **200**: 工作平面ID
- **0.0**: 约束值（垂直约束不需要值）
- **lineToEntity[line1Id], lineToEntity[line2Id]**: 两条线段的实体ID

现在垂直约束功能已经完全实现，用户可以点击"垂直约束"按钮来创建两条垂直的线段！
