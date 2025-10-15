# EaCircle类使用说明

## 功能概述

`EaCircle`类是一个用于在界面上绘制圆的几何图形类，继承自`EaShape`基类。它提供了圆的创建、绘制、选择和管理功能。

## 类结构

### 头文件：`geometry/eacircle.h`
```cpp
class EaCircle : public EaShape
{
public:
    EaCircle();
    EaCircle(EaPoint* center, double radius);
    
    // 核心功能
    bool onDrag(double x, double y) override;
    void onDraw(QPainter* painter) override;
    
    // 属性管理
    EaPoint* getCenter() const;
    double getRadius() const;
    void setCenter(EaPoint* center);
    void setRadius(double radius);
    
    // ID和选择管理
    int getId() const;
    void setId(int id);
    bool isSelected() const;
    void setSelected(bool selected);
};
```

### 实现文件：`geometry/eacircle.cpp`

## 主要功能

### 1. 构造函数
- **默认构造函数**: `EaCircle()` - 创建空的圆对象
- **带参数构造函数**: `EaCircle(EaPoint* center, double radius)` - 创建指定圆心和半径的圆

### 2. 绘制功能 (`onDraw`)
- 使用`QPainter`绘制圆形
- 支持选择状态的颜色变化（选中时显示红色，未选中时显示蓝色）
- 选中时额外绘制圆心点
- 使用`drawEllipse`方法绘制圆形

### 3. 拖拽功能 (`onDrag`)
- 圆本身不直接支持拖拽
- 通过拖拽圆心来改变圆的位置

### 4. 属性管理
- **圆心**: 通过`EaPoint`对象管理圆心位置
- **半径**: 使用`double`类型存储半径值
- **ID**: 用于唯一标识圆对象
- **选择状态**: 管理圆是否被选中

## 在EaSession中的集成

### 1. 添加圆的方法
```cpp
int EaSession::addCircle(int centerPointId, double radius)
```
- 验证圆心点是否存在
- 创建`EaCircle`对象
- 设置圆心和半径
- 分配唯一ID
- 发出`circleAdded`信号

### 2. 圆的管理功能
- `getCircle(int circleId)`: 根据ID获取圆对象
- `removeCircle(int circleId)`: 删除指定的圆
- `selectCircle(int circleId, bool selected)`: 选择/取消选择圆
- `getSelectedCircles()`: 获取所有被选中的圆

### 3. 信号系统
- `circleAdded(int circleId, int centerPointId, double radius)`: 圆被添加时发出
- `circleRemoved(int circleId)`: 圆被删除时发出
- `geometryChanged()`: 几何图形发生变化时发出
- `selectionChanged()`: 选择状态发生变化时发出

## 在约束系统中的应用

### 1. 点在圆上约束
在`createPtOnCircleConstraint()`方法中：
```cpp
// 创建圆心点
int centerPt = this->addPoint(100.0, 100.0);

// 创建圆上的点
int ptOnCircle = this->addPoint(130.0, 100.0);

// 创建圆实体用于界面显示
int circleId = this->addCircle(centerPt, 30.0);

// 添加约束
this->addPtOnCircleConstraint(ptOnCircle, centerPt, 30.0);
```

### 2. 与SolveSpace的集成
- `EaCircle`用于界面显示
- SolveSpace中的圆实体用于约束求解
- 两者通过圆心点ID关联

## 绘制特性

### 1. 颜色方案
- **未选中**: 蓝色 (`QColor(33, 150, 243)`)
- **选中**: 红色 (`QColor(244, 67, 54)`)
- **线宽**: 未选中2.0px，选中3.0px

### 2. 绘制细节
- 使用`QRectF`计算圆的边界矩形
- 圆心坐标：`(centerPos.x() - radius, centerPos.y() - radius)`
- 圆的大小：`2 * radius × 2 * radius`
- 选中时额外绘制红色圆心点

### 3. 性能优化
- 使用`painter->save()`和`painter->restore()`确保绘制状态隔离
- 只在必要时进行绘制操作

## 使用示例

### 1. 创建圆
```cpp
// 创建圆心点
int centerId = session->addPoint(100.0, 100.0);

// 创建圆
int circleId = session->addCircle(centerId, 50.0);
```

### 2. 选择圆
```cpp
// 选择圆
session->selectCircle(circleId, true);

// 取消选择
session->selectCircle(circleId, false);
```

### 3. 获取圆信息
```cpp
EaCircle* circle = session->getCircle(circleId);
if (circle) {
    EaPoint* center = circle->getCenter();
    double radius = circle->getRadius();
    bool selected = circle->isSelected();
}
```

## 扩展功能

### 1. 可能的扩展
- 添加圆的填充颜色
- 支持虚线样式
- 添加圆的标签显示
- 支持圆的旋转

### 2. 与其他几何图形的交互
- 与`EaPoint`的关联（圆心）
- 与`EaLine`的交互（切线、弦等）
- 与其他`EaCircle`的交互（相切、相交等）

## 注意事项

1. **圆心依赖**: 圆必须有一个有效的圆心点
2. **半径限制**: 半径应该为正数
3. **内存管理**: 使用智能指针管理圆对象的生命周期
4. **线程安全**: 绘制操作应在主线程中进行
5. **性能考虑**: 大量圆对象时注意绘制性能

这个`EaCircle`类为几何约束系统提供了完整的圆绘制和管理功能，与现有的点和线段系统完美集成。
