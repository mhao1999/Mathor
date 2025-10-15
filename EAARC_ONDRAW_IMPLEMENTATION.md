# EaArc类onDraw函数实现说明

## 用户请求

用户添加了一个`EaArc`类用来绘制圆弧，成员中的起始角度单位是角度，请完善`onDraw`函数。

## 实现内容

### EaArc类结构

**文件**: `geometry/eaarc.h`

```cpp
class EaArc : public EaShape
{
public:
    EaArc();
    EaArc(EaPoint* center, double startAngle, double endAngle, double radius);

    bool onDrag(double x, double y) override;
    void onDraw(QPainter* painter) override;

    // 圆心和半径管理
    EaPoint* getCenter() const { return m_center; }
    double getRadius() const { return m_radius; }
    void setCenter(EaPoint* center);
    void setRadius(double radius);

    double getStartAngle() const { return m_startAngle; }
    double getEndAngle() const { return m_endAngle; }
    void setStartAngle(double angle);
    void setEndAngle(double angle);

    // ID管理
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }

    // 选择状态
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected) { m_selected = selected; }

private:
    EaPoint* m_center = nullptr;
    double m_startAngle = 0.0;      // 起始角度（度）
    double m_endAngle = 90.0;       // 结束角度（度）
    double m_radius = 1.0;          // 半径
    int m_id = -1;
    bool m_selected = false;
};
```

### onDraw函数实现

**文件**: `geometry/eaarc.cpp`

```cpp
void EaArc::onDraw(QPainter* painter)
{
    if (!painter || !m_center) return;

    painter->save();

    // 选择颜色和线宽
    QColor color = m_selected ? QColor(244, 67, 54) : QColor(33, 150, 243); // 红色或蓝色
    double width = m_selected ? 3.0 : 2.0;

    QPen arcPen(color, width);
    painter->setPen(arcPen);

    // 获取圆心坐标
    QPointF centerPos(m_center->pos().x(), m_center->pos().y());

    // 计算圆弧的边界矩形
    QRectF arcRect(centerPos.x() - m_radius, centerPos.y() - m_radius, 
                   m_radius * 2, m_radius * 2);

    // 将角度从度转换为Qt的1/16度单位
    // Qt的drawArc使用1/16度作为单位，所以需要乘以16
    int startAngle16 = static_cast<int>(m_startAngle * 16);
    int spanAngle16 = static_cast<int>((m_endAngle - m_startAngle) * 16);

    // 绘制圆弧
    painter->drawArc(arcRect, startAngle16, spanAngle16);

    // 如果圆弧被选中，绘制圆心点
    if (m_selected) {
        painter->setPen(QPen(QColor(255, 0, 0), 4.0));
        painter->drawPoint(centerPos);
    }

    painter->restore();
}
```

## 技术细节

### 1. 角度转换

#### **输入角度单位**
- `m_startAngle` 和 `m_endAngle` 使用**度**作为单位
- 例如：0度、90度、180度、270度等

#### **Qt drawArc角度单位**
- Qt的`drawArc`方法使用**1/16度**作为单位
- 需要将度转换为1/16度：`角度 * 16`

#### **转换公式**
```cpp
int startAngle16 = static_cast<int>(m_startAngle * 16);
int spanAngle16 = static_cast<int>((m_endAngle - m_startAngle) * 16);
```

### 2. 圆弧绘制

#### **边界矩形计算**
```cpp
QRectF arcRect(centerPos.x() - m_radius, centerPos.y() - m_radius, 
               m_radius * 2, m_radius * 2);
```
- 以圆心为中心，创建边长为`2 * radius`的正方形
- 这个矩形定义了圆弧的边界

#### **drawArc参数**
- **arcRect**: 圆弧的边界矩形
- **startAngle16**: 起始角度（1/16度单位）
- **spanAngle16**: 跨越角度（1/16度单位）

### 3. 视觉效果

#### **颜色和线宽**
- **未选中**: 蓝色 (#2196F3)，线宽2.0
- **选中**: 红色 (#F44336)，线宽3.0

#### **选中状态指示**
- 当圆弧被选中时，在圆心位置绘制红色圆点
- 圆点大小：4.0像素

### 4. 角度系统

#### **Qt角度系统**
- Qt使用**逆时针**角度系统
- 0度指向**3点钟方向**（正X轴）
- 90度指向**12点钟方向**（负Y轴）
- 180度指向**9点钟方向**（负X轴）
- 270度指向**6点钟方向**（正Y轴）

#### **角度示例**
```cpp
// 从0度到90度的圆弧（第一象限）
EaArc arc(center, 0.0, 90.0, radius);

// 从90度到270度的半圆
EaArc arc(center, 90.0, 270.0, radius);

// 从0度到360度的完整圆
EaArc arc(center, 0.0, 360.0, radius);
```

## 使用示例

### 创建圆弧对象
```cpp
// 创建圆心点
EaPoint* center = new EaPoint(100.0, 100.0);

// 创建圆弧：从0度到90度，半径50
EaArc* arc = new EaArc(center, 0.0, 90.0, 50.0);
arc->setId(1);
arc->setSelected(false);
```

### 绘制圆弧
```cpp
QPainter painter(this);
arc->onDraw(&painter);
```

## 与其他几何元素的对比

### EaCircle vs EaArc
- **EaCircle**: 绘制完整圆形
- **EaArc**: 绘制部分圆弧

### EaLine vs EaArc
- **EaLine**: 绘制直线段
- **EaArc**: 绘制曲线段

### 共同特点
- 都继承自`EaShape`
- 都支持选择状态
- 都使用相同的颜色方案
- 都支持ID管理

## 注意事项

### 1. 角度范围
- 起始角度和结束角度可以是任意值
- 跨越角度 = 结束角度 - 起始角度
- 支持跨越角度大于360度的情况

### 2. 性能考虑
- 使用`painter->save()`和`painter->restore()`确保状态隔离
- 角度转换使用整数运算，避免浮点误差

### 3. 坐标系统
- 使用Qt的坐标系统（Y轴向下）
- 圆心坐标直接来自`EaPoint`对象

## 扩展功能

### 可能的增强
1. **箭头指示**: 在圆弧端点添加箭头
2. **角度标注**: 显示角度数值
3. **填充模式**: 支持填充圆弧
4. **虚线样式**: 支持不同的线型

现在`EaArc`类的`onDraw`函数已经完全实现，可以正确绘制圆弧了！
