# 点在圆上约束映射关系修复

## 问题描述

在`GeometrySolver`中，`circleToEntity[constraint.id]`的映射关系设计有问题。使用`constraint.id`作为键来存储圆的实体ID是不合理的，因为：

1. 约束ID和圆实体ID是两个不同的概念
2. 一个约束可能对应多个圆，或者需要更清晰的映射关系
3. 约束ID是用于标识约束的，而不是用于标识几何实体的

## 问题代码

```cpp
// 问题代码：使用约束ID作为键
std::map<int, int> circleToEntity; // 圆ID到实体ID的映射
circleToEntity[constraint.id] = circleEntityIndex;

// 在约束处理中使用
circleToEntity[constraint.id] // 这里应该是圆的实体ID，不是约束ID
```

## 修复方案

### 1. 重新设计映射关系

使用圆心点ID作为键，因为每个圆心点对应一个圆：

```cpp
// 修复后：使用圆心点ID作为键
std::map<int, int> centerToCircleEntity; // 圆心点ID到圆实体ID的映射
centerToCircleEntity[centerPointId] = circleEntityIndex;
```

### 2. 避免重复创建圆

添加检查逻辑，确保每个圆心点只创建一个圆：

```cpp
// 检查是否已经为这个圆心创建了圆
if (centerToCircleEntity.find(centerPointId) == centerToCircleEntity.end() &&
    pointToEntity.find(centerPointId) != pointToEntity.end()) {
    // 创建圆实体
    // ...
    centerToCircleEntity[centerPointId] = circleEntityIndex;
}
```

### 3. 更新约束处理逻辑

在约束处理中使用正确的映射关系：

```cpp
// 修复后：使用圆心点ID查找圆实体
if (pointToEntity.find(pointId) != pointToEntity.end() && 
    centerToCircleEntity.find(centerPointId) != centerToCircleEntity.end()) {
    
    // 使用圆心点ID查找圆实体ID
    centerToCircleEntity[centerPointId]
}
```

## 修复后的优势

1. **逻辑清晰**: 圆心点ID到圆实体ID的映射关系更加直观
2. **避免重复**: 每个圆心点只创建一个圆实体
3. **易于维护**: 映射关系更加清晰，便于调试和维护
4. **扩展性好**: 可以轻松支持多个点在同一个圆上的情况

## 调试信息改进

修复后的调试信息更加清晰：

```cpp
qDebug() << "GeometrySolver: Added point on circle constraint" << constraintId
         << "for point" << pointId << "on circle with center" << centerPointId 
         << "entities" << pointToEntity[pointId] << centerToCircleEntity[centerPointId];
```

## 测试验证

修复后应该能够：
1. 正确创建圆实体
2. 正确建立点在圆上约束
3. 避免重复创建圆实体
4. 提供清晰的调试信息

## 相关文件

- `main/eageosolver.cpp`: 主要修复文件
- 第240行：重新设计映射关系
- 第400行：修复约束处理逻辑

这个修复确保了点在圆上约束功能的正确性和稳定性。
