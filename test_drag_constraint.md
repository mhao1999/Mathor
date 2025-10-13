# 拖拽约束功能测试指南

## 测试步骤

### 1. 启动应用程序
```bash
cd D:\qt\Mathor
qmake
nmake
# 运行生成的Mathor.exe
```

### 2. 创建测试场景
1. 点击"添加点1"按钮 - 在(10, 20)创建点1
2. 点击"添加点2"按钮 - 在(50, 60)创建点2
3. 点击"添加线段"按钮 - 连接点1和点2，并添加100.0的距离约束

### 3. 测试拖拽约束
1. **选择点2**：点击点2，应该看到点2被选中（变红色）
2. **拖拽点2**：按住鼠标左键并移动点2
3. **观察约束效果**：
   - 点2应该在以点1为圆心、半径为100.0的圆上移动
   - 线段长度应该保持100.0不变
   - 控制台应该显示约束求解的调试信息

### 4. 验证约束求解
- 检查控制台输出，应该看到：
  ```
  EaSession: solveDragConstraint called for point 2 to position [x] [y]
  EaSession: Constraint solving successful for point 2
  EaPoint: Drag with constraints successful for point 2
  ```

### 5. 测试约束失败情况
1. 尝试拖拽点2到距离点1超过100.0的位置
2. 系统应该自动调整点2的位置到约束允许的范围内

## 预期结果

### 成功情况
- 点2在拖拽时始终保持在距离点1为100.0的圆上
- 线段长度保持100.0不变
- 拖拽操作流畅，无卡顿
- 控制台显示约束求解成功信息

### 失败情况处理
- 如果约束求解失败，系统会回退到简单拖拽模式
- 控制台会显示相应的警告信息
- 应用程序不会崩溃

## 调试信息

### 正常拖拽时的控制台输出
```
EaSession: solveDragConstraint called for point 2 to position 60.0 70.0
EaSession: Constraint solving successful for point 2
EaPoint: Drag with constraints successful for point 2
EaSession: Constraint solving successful for point 2
```

### 约束求解失败时的控制台输出
```
EaSession: solveDragConstraint called for point 2 to position 60.0 70.0
EaSession: Constraint solving failed for point 2
EaPoint: Constraint solving failed, using simple drag for point 2
```

## 性能测试

### 拖拽响应性
- 拖拽操作应该实时响应鼠标移动
- 约束求解时间应该在可接受范围内（< 50ms）
- 界面不应该出现明显的卡顿

### 内存使用
- 长时间拖拽操作不应该导致内存泄漏
- 约束求解器应该正确释放资源

## 边界情况测试

### 1. 极值拖拽
- 拖拽到屏幕边缘
- 拖拽到负坐标区域
- 拖拽到极大坐标值

### 2. 快速拖拽
- 快速移动鼠标
- 连续拖拽操作
- 拖拽过程中改变约束

### 3. 多约束场景
- 添加多个距离约束
- 创建三角形约束系统
- 测试过约束情况

## 问题排查

### 常见问题
1. **约束不生效**：检查GeometrySolver是否正确设置
2. **拖拽卡顿**：检查约束求解性能
3. **位置不正确**：检查坐标转换和约束参数

### 调试技巧
1. 启用详细日志输出
2. 检查约束系统的一致性
3. 验证SolveSpaceLib的集成
