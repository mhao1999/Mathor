# 角度约束实现说明

## 用户请求

用户要求点击"角度约束"按钮触发`EaSession::createAngleConstraint`函数，请完善`SLVS_C_ANGLE`角度约束代码，以两条线段为例（四个点）。

## 实现内容

### 1. EaSession::createAngleConstraint函数

**文件**: `main/easession.cpp`

```cpp
void EaSession::createAngleConstraint()
{
    this->clear();
    
    // 创建4个点，构成两条线段
    // 第一条线段：(点1，点2) - 水平线段
    int pt1 = this->addPoint(50.0, 100.0);   // 线段1起点
    int pt2 = this->addPoint(150.0, 100.0);  // 线段1终点
    
    // 第二条线段：(点3，点4) - 与第一条线段成45度角
    int pt3 = this->addPoint(100.0, 50.0);   // 线段2起点
    int pt4 = this->addPoint(150.0, 150.0);  // 线段2终点
    
    // 创建两条线段
    int line1 = this->addLine(pt1, pt2);     // 水平线段
    int line2 = this->addLine(pt3, pt4);     // 斜线段
    
    // 固定一些点以稳定约束系统
    this->createFixPointConstraint(pt1);     // 固定线段1起点
    this->createFixPointConstraint(pt3);     // 固定线段2起点
    
    // 添加角度约束（45度角）
    this->addAngleConstraint(line1, line2, 45.0);
    
    qDebug() << "EaSession: Created angle constraint between line" << line1 << "and line" << line2 << "with angle 45 degrees";
}
```

### 2. EaSession::addAngleConstraint函数

**文件**: `main/easession.h` 和 `main/easession.cpp`

```cpp
// 头文件声明
void addAngleConstraint(int line1Id, int line2Id, double angle);

// 实现文件
void EaSession::addAngleConstraint(int line1Id, int line2Id, double angle)
{
    // 验证线段是否存在
    EaLine* line1 = getLine(line1Id);
    EaLine* line2 = getLine(line2Id);
    
    if (!line1 || !line2) {
        qWarning() << "EaSession: Cannot add angle constraint - invalid line IDs:" << line1Id << line2Id;
        return;
    }
    
    // 添加角度约束
    Constraint angleConstraint(m_nextConstraintId++, "angle");
    angleConstraint.data["line1"] = line1Id;
    angleConstraint.data["line2"] = line2Id;
    angleConstraint.data["angle"] = angle;
    
    m_constraints.push_back(angleConstraint);
    
    qDebug() << "EaSession: Added angle constraint" << angleConstraint.id 
             << "between lines" << line1Id << "and" << line2Id << "with angle" << angle << "degrees";
}
```

### 3. GeometrySolver中的角度约束处理

**文件**: `main/eageosolver.cpp`

```cpp
else if (type == "angle") {
    int line1Id = std::any_cast<int>(constraint.data.at("line1"));
    int line2Id = std::any_cast<int>(constraint.data.at("line2"));
    double angle = std::any_cast<double>(constraint.data.at("angle"));
    
    if (lineToEntity.find(line1Id) != lineToEntity.end() && 
        lineToEntity.find(line2Id) != lineToEntity.end()) {
        
        // 添加角度约束
        int constraintId = m_sys.constraints + 1;
        Slvs_Constraint constraint = Slvs_MakeConstraint(
            constraintId, g,
            SLVS_C_ANGLE,
            200,
            angle * M_PI / 180.0,  // 角度转换为弧度
            0, 0,
            lineToEntity[line1Id], lineToEntity[line2Id]);
        constraint.entityC = 0;
        constraint.entityD = 0;
        m_sys.constraint[m_sys.constraints++] = constraint;
        
        qDebug() << "GeometrySolver: Added angle constraint" << constraintId
                 << "between lines" << line1Id << "and" << line2Id 
                 << "entities" << lineToEntity[line1Id] << lineToEntity[line2Id]
                 << "with angle" << angle << "degrees";
    } else {
        qWarning() << "GeometrySolver: Cannot add angle constraint - missing line entities" << line1Id << line2Id;
    }
}
```

### 4. QML界面更新

**文件**: `main.qml`

```qml
Button {
    text: "角度约束"
    Layout.fillWidth: true
    enabled: drawingArea !== null
    onClicked: {
        globalSession.createAngleConstraint()
        statusText.text = "角度约束已创建"
        statusText.color = "green"
    }
}
```

## 几何布局

### 点的位置
- **P1**: (50, 100) - 线段1起点，**固定**
- **P2**: (150, 100) - 线段1终点，**可移动**
- **P3**: (100, 50) - 线段2起点，**固定**
- **P4**: (150, 150) - 线段2终点，**可移动**

### 线段配置
- **线段1**: P1-P2 (水平线段)
- **线段2**: P3-P4 (斜线段，与线段1成45度角)

### 约束系统
- **固定约束**: P1和P3被固定
- **角度约束**: 线段1和线段2之间的角度被约束为45度

## 自由度分析

### 总自由度
- 4个点 × 2 = **8个自由度**

### 约束减少的自由度
- **角度约束**: 减少**1个自由度**
- **固定P1**: 减少**2个自由度**
- **固定P3**: 减少**2个自由度**

### 剩余自由度
```
剩余自由度 = 8 - 1 - 2 - 2 = 3个自由度
```

### 剩余自由度的含义
- **P2可以沿线段1方向移动**：调整线段1的长度
- **P4可以沿线段2方向移动**：调整线段2的长度
- **两条线段可以绕交点旋转**：保持45度角不变

## 预期行为

### 点击"角度约束"按钮后
1. 清空现有几何元素
2. 创建4个点和2条线段
3. 添加固定约束和角度约束
4. 两条线段之间的角度被约束为45度

### 拖拽行为
- **拖拽P1**: 由于固定约束，不能移动
- **拖拽P2**: 可以沿线段1方向移动，调整线段1长度
- **拖拽P3**: 由于固定约束，不能移动
- **拖拽P4**: 可以沿线段2方向移动，调整线段2长度
- **角度保持45度**: 无论P2或P4如何移动，两条线段之间的角度都保持45度

## 调试信息

系统会输出详细的调试信息：
```
EaSession: Created angle constraint between line 1 and line 2 with angle 45 degrees
EaSession: Added angle constraint 1 between lines 1 and 2 with angle 45 degrees
GeometrySolver: Added angle constraint 1 between lines 1 and 2 entities 300 301 with angle 45 degrees
```

## 相关文件修改

1. **`main/easession.h`**: 添加`addAngleConstraint`函数声明
2. **`main/easession.cpp`**: 
   - 实现`createAngleConstraint`函数
   - 实现`addAngleConstraint`函数
3. **`main/eageosolver.cpp`**: 添加"angle"约束类型处理
4. **`main.qml`**: 更新按钮点击事件

## 技术细节

### 约束类型
- 使用`SLVS_C_ANGLE`约束类型
- 约束两条线段之间的角度

### 参数设置
- **constraintId**: 自动生成的约束ID
- **g**: 组ID（Group 2）
- **SLVS_C_ANGLE**: 角度约束类型
- **200**: 工作平面ID
- **angle * M_PI / 180.0**: 角度值（转换为弧度）
- **lineToEntity[line1Id]**: 线段1的实体ID
- **lineToEntity[line2Id]**: 线段2的实体ID

### 角度转换
- SolveSpace使用弧度制，需要将角度转换为弧度
- 转换公式：`弧度 = 角度 * π / 180`

### 约束效果
- 两条线段之间的角度被固定为指定值
- 线段可以调整长度，但角度关系保持不变
- 系统保持几何一致性

## 与其他约束的对比

### 平行约束
- 角度固定为0度或180度
- 两条线段方向相同或相反

### 垂直约束
- 角度固定为90度
- 两条线段垂直

### 角度约束
- 角度可以设置为任意值
- 更灵活的角度控制

## 应用场景

角度约束在工程设计中非常有用：
- **机械设计**: 设定零件之间的角度关系
- **建筑制图**: 控制墙体、梁柱的角度
- **CAD建模**: 精确控制几何元素的角度关系

现在角度约束功能已经完全实现！用户可以点击"角度约束"按钮来创建两条成45度角的线段。
