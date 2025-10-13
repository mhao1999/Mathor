# EaDrawingArea 使用指南

## 🎯 简介

`EaDrawingArea` 是一个基于 `QQuickPaintedItem` 的高性能几何绘制组件，专为几何约束求解器和CAD应用设计。

## 📦 特性一览

✅ **高性能绘制** - C++ QPainter 原生渲染  
✅ **完整交互** - 拖拽、缩放、平移  
✅ **网格系统** - 可视网格 + 吸附功能  
✅ **坐标转换** - 世界坐标 ↔ 屏幕坐标  
✅ **信号通知** - 事件驱动的 QML 集成  
✅ **可视反馈** - 悬停高亮、选中状态  

---

## 🚀 快速开始

### 1. 在 QML 中导入

```qml
import Mathor.Drawing 1.0

DrawingArea {
    id: drawingArea
    anchors.fill: parent
}
```

### 2. 添加几何元素

```qml
// 添加点
drawingArea.addPoint(10, 20)    // 点1：坐标 (10, 20)
drawingArea.addPoint(50, 60)    // 点2：坐标 (50, 60)

// 连接点成线
drawingArea.addLine(1, 2)       // 连接点1和点2
```

### 3. 监听交互事件

```qml
DrawingArea {
    id: drawingArea
    
    onPointClicked: function(pointId, x, y) {
        console.log("点击了点", pointId)
    }
    
    onPointDragged: function(pointId, x, y) {
        console.log("正在拖拽点", pointId, "到", x, y)
    }
    
    onPointReleased: function(pointId, x, y) {
        console.log("释放了点", pointId, "最终位置", x, y)
    }
}
```

---

## 📚 API 参考

### 属性 (Properties)

| 属性 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `showGrid` | bool | true | 是否显示网格 |
| `gridSize` | double | 20.0 | 网格间距 |
| `snapToGrid` | bool | false | 是否吸附到网格 |
| `zoomLevel` | double | 1.0 | 缩放级别（0.1-10.0） |

### 方法 (Methods)

#### `addPoint(x, y)`
添加一个点到绘图区域。

**参数：**
- `x` (double): X 坐标（世界坐标）
- `y` (double): Y 坐标（世界坐标）

**返回：** 无（点会被赋予唯一 ID）

**示例：**
```qml
drawingArea.addPoint(100, 50)
```

---

#### `addLine(startId, endId)`
连接两个点创建线段。

**参数：**
- `startId` (int): 起点 ID
- `endId` (int): 终点 ID

**示例：**
```qml
drawingArea.addLine(1, 2)
```

---

#### `clear()`
清空所有几何元素。

**示例：**
```qml
drawingArea.clear()
```

---

#### `updatePointPosition(id, x, y)`
更新指定点的位置（用于求解器结果更新）。

**参数：**
- `id` (int): 点 ID
- `x` (double): 新的 X 坐标
- `y` (double): 新的 Y 坐标

**示例：**
```qml
// 求解器返回新位置后更新
drawingArea.updatePointPosition(1, 15.5, 22.3)
```

---

#### `screenToWorld(x, y)`
将屏幕坐标转换为世界坐标。

**返回：** `QPointF`

**示例：**
```qml
var worldPos = drawingArea.screenToWorld(mouseX, mouseY)
console.log("世界坐标:", worldPos.x, worldPos.y)
```

---

#### `worldToScreen(x, y)`
将世界坐标转换为屏幕坐标。

**返回：** `QPointF`

---

### 信号 (Signals)

#### `pointClicked(pointId, x, y)`
当点被点击时发出。

**参数：**
- `pointId` (int): 被点击的点 ID
- `x` (double): 点的 X 坐标
- `y` (double): 点的 Y 坐标

---

#### `pointDragged(pointId, x, y)`
当点被拖拽时发出（持续触发）。

**用途：** 实时更新 UI 或触发增量求解

---

#### `pointReleased(pointId, x, y)`
当点被释放时发出。

**用途：** 触发完整的约束求解

---

## 💡 使用模式

### 模式 1: 静态几何显示

```qml
DrawingArea {
    id: drawingArea
    
    Component.onCompleted: {
        // 绘制一个三角形
        addPoint(0, 0)
        addPoint(50, 0)
        addPoint(25, 40)
        
        addLine(1, 2)
        addLine(2, 3)
        addLine(3, 1)
    }
}
```

### 模式 2: 交互式编辑

```qml
DrawingArea {
    id: drawingArea
    
    snapToGrid: true  // 启用吸附
    
    onPointReleased: function(pointId, x, y) {
        // 点释放后，触发约束求解
        geometrySolver.solve()
    }
}
```

### 模式 3: 与求解器集成

```qml
RowLayout {
    DrawingArea {
        id: drawingArea
        Layout.fillWidth: true
        
        onPointDragged: function(pointId, x, y) {
            // 拖拽时显示实时位置
            statusLabel.text = `P${pointId}: (${x.toFixed(2)}, ${y.toFixed(2)})`
        }
        
        onPointReleased: function(pointId, x, y) {
            // 释放后求解约束
            solver.solveConstraints()
        }
    }
    
    GeometrySolver {
        id: solver
        
        onSolvingFinished: function(success) {
            if (success) {
                // 更新绘图区域的点位置
                var result = solver.getSolvedPoints()
                drawingArea.updatePointPosition(1, result.x1, result.y1)
                drawingArea.updatePointPosition(2, result.x2, result.y2)
            }
        }
    }
}
```

---

## 🎨 视图控制

### 网格设置

```qml
DrawingArea {
    showGrid: gridCheckBox.checked
    gridSize: gridSizeSlider.value  // 10-50
    snapToGrid: snapCheckBox.checked
}
```

### 缩放控制

```qml
DrawingArea {
    id: drawingArea
    
    // 滚轮自动缩放（内置）
    // 或手动控制：
    
    Slider {
        from: 0.1
        to: 10.0
        value: drawingArea.zoomLevel
        onValueChanged: drawingArea.zoomLevel = value
    }
}
```

### 视图重置

```qml
Button {
    text: "重置视图"
    onClicked: {
        drawingArea.zoomLevel = 1.0
        // panOffset 会自动处理
    }
}
```

---

## 🖱️ 交互操作

### 鼠标操作

| 操作 | 功能 |
|------|------|
| **左键点击点** | 选中点 |
| **左键拖拽点** | 移动点位置 |
| **中键/右键拖拽** | 平移视图 |
| **滚轮** | 缩放视图 |
| **悬停** | 高亮显示 |

### 视觉反馈

- 🟢 **绿色点**: 正常状态
- 🔴 **红色点**: 选中/悬停状态
- 🔵 **蓝色线**: 线段
- ⭕ **外圈**: 悬停时显示

---

## 🔧 高级用法

### 自定义样式

目前颜色在 C++ 中定义，未来可扩展为属性：

```cpp
// eadrawingarea.h
Q_PROPERTY(QColor pointColor READ pointColor WRITE setPointColor ...)
```

### 添加更多几何类型

在 `eadrawingarea.h` 中已经定义了 `ElementType` 枚举：

```cpp
enum ElementType {
    Point,
    Line,
    Circle,   // 未实现
    Arc       // 未实现
};
```

可以扩展实现圆和圆弧：

```cpp
void addCircle(double centerX, double centerY, double radius);
void addArc(int centerId, int startId, int endId);
```

### 导出绘图

利用 QPainter 可以绘制到任何设备：

```cpp
// 导出为图片
QPixmap pixmap(width, height);
QPainter painter(&pixmap);
paint(&painter);
pixmap.save("drawing.png");

// 打印
QPrinter printer;
QPainter painter(&printer);
paint(&painter);
```

---

## 🐛 常见问题

### Q: 点击时没有反应？

**A:** 确保设置了正确的尺寸：
```qml
DrawingArea {
    anchors.fill: parent  // 或设置 width/height
}
```

### Q: 网格显示不正常？

**A:** 检查 `gridSize` 和 `zoomLevel`：
```qml
gridSize: 20  // 不要太小或太大
```

### Q: 拖拽不流畅？

**A:** 确保没有在 `onPointDragged` 中执行耗时操作：
```qml
// ❌ 不好
onPointDragged: {
    solver.solve()  // 太慢！
}

// ✅ 好
onPointReleased: {
    solver.solve()  // 只在释放时求解
}
```

### Q: 如何获取所有点的坐标？

**A:** 目前需要在 C++ 中添加方法：
```cpp
Q_INVOKABLE QVariantList getAllPoints() const;
```

---

## 📊 性能建议

1. **批量添加**
   ```qml
   // 添加多个点时，暂时禁用自动更新
   // （未来可添加 beginUpdate()/endUpdate()）
   ```

2. **限制元素数量**
   - 点数 < 10,000: 流畅
   - 点数 > 10,000: 考虑分层或 LOD

3. **优化绘制**
   - 已启用 FramebufferObject
   - 已启用抗锯齿
   - 自动裁剪不可见元素（未来可添加）

---

## 🎯 完整示例

查看 `DrawingAreaDemo.qml` 获取完整可运行示例，包括：

- ✅ 视图控制面板
- ✅ 几何操作按钮
- ✅ 约束求解集成
- ✅ 快速示例（三角形、矩形等）
- ✅ 状态显示

**运行方式：**
```cpp
// 在 main.cpp 中加载
engine.load(QUrl("qrc:/DrawingAreaDemo.qml"));
```

---

## 📖 相关文档

- `CANVAS_VS_PAINTEDITEM.md` - 方案对比
- `INTEGRATION_README.md` - 求解器集成
- `QUICK_START.md` - 快速开始

---

## 🚀 下一步

1. **尝试基础示例**
   ```qml
   DrawingArea {
       Component.onCompleted: {
           addPoint(0, 0)
           addPoint(50, 50)
           addLine(1, 2)
       }
   }
   ```

2. **添加交互**
   ```qml
   onPointReleased: {
       console.log("可以在这里添加约束求解")
   }
   ```

3. **集成到你的应用**
   - 替换 Canvas
   - 连接求解器
   - 添加 UI 控制

祝开发顺利！💪

