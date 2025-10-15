import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Mathor.Drawing 1.0
import Mathor.Solver 1.0

/**
 * EaDrawingArea 使用示例
 * 演示如何使用自定义绘图区域与几何求解器结合
 */
Window {
    width: 1200
    height: 800
    visible: true
    title: "Mathor - 几何绘制区域演示"
    
    // 几何求解器
    GeometrySolver {
        id: solver
        
        onSolvingFinished: function(success) {
            // 注意：现在点位置更新由EaSession.solveDragConstraint直接处理
            // 这里只处理状态显示
            if (success) {
                statusText.text = "求解成功! 自由度: " + solver.dof
                statusText.color = "green"
            } else {
                statusText.text = "求解失败: " + solver.lastError
                statusText.color = "red"
            }
        }
    }
    
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // 左侧：绘图区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#f5f5f5"
            
            DrawingArea {
                id: drawingArea
                focus: true
                anchors.fill: parent
                anchors.margins: 10
                
                showGrid: gridCheckBox.checked
                gridSize: gridSizeSlider.value
                snapToGrid: snapCheckBox.checked
                
                // 点被点击
                onPointClicked: function(pointId, x, y) {
                    console.log("点击点", pointId, "坐标:", x, y)
                    pointInfoText.text = "点 P" + pointId + ": (" +
                            x.toFixed(2) + ", " + y.toFixed(2) + ")"
                }
                
                // 点被拖拽
                onPointDragged: function(pointId, x, y) {
                    pointInfoText.text = "拖拽点 P" + pointId + ": (" +
                            x.toFixed(2) + ", " + y.toFixed(2) + ")"
                }
                
                // 点释放
                onPointReleased: function(pointId, x, y) {
                    console.log("释放点", pointId, "最终坐标:", x, y)
                }
            }
            
            // 浮动工具栏
            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 20
                width: 200
                height: infoColumn.height + 20
                color: "#ffffff"
                border.color: "#cccccc"
                border.width: 1
                radius: 5
                opacity: 0.9
                
                ColumnLayout {
                    id: infoColumn
                    anchors.centerIn: parent
                    width: parent.width - 20
                    spacing: 5
                    
                    Text {
                        text: "缩放: " + (drawingArea.zoomLevel * 100).toFixed(0) + "%"
                        font.pixelSize: 12
                    }
                    
                    Text {
                        id: pointInfoText
                        text: "将鼠标悬停在点上"
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }
            
            // 帮助文本
            Text {
                anchors.bottom: parent.bottom
                anchors.right: parent.right
                anchors.margins: 20
                text: "• 左键拖拽点\n• 中键/右键平移视图\n• 滚轮缩放"
                font.pixelSize: 11
                color: "#666666"
                horizontalAlignment: Text.AlignRight
            }
        }
        
        // 右侧：控制面板
        Rectangle {
            Layout.preferredWidth: 350
            Layout.fillHeight: true
            color: "white"
            border.color: "#e0e0e0"
            border.width: 1
            
            ScrollView {
                anchors.fill: parent
                
                ColumnLayout {
                    width: parent.parent.width
                    spacing: 15
                    // padding: 20
                    
                    // 标题
                    Text {
                        text: "控制面板"
                        font.pixelSize: 18
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#e0e0e0"
                    }
                    
                    // 视图设置
                    GroupBox {
                        title: "视图设置"
                        Layout.fillWidth: true
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8
                            
                            CheckBox {
                                id: gridCheckBox
                                text: "显示网格"
                                checked: true
                            }
                            
                            RowLayout {
                                Label {
                                    text: "网格大小:"
                                    Layout.preferredWidth: 80
                                }
                                Slider {
                                    id: gridSizeSlider
                                    from: 10
                                    to: 50
                                    value: 20
                                    stepSize: 5
                                    Layout.fillWidth: true
                                }
                                Label {
                                    text: gridSizeSlider.value.toFixed(0)
                                    Layout.preferredWidth: 30
                                }
                            }
                            
                            CheckBox {
                                id: snapCheckBox
                                text: "吸附到网格"
                                checked: false
                            }
                            
                            Button {
                                text: "重置视图"
                                Layout.fillWidth: true
                                onClicked: {
                                    drawingArea.zoomLevel = 1.0
                                }
                            }
                        }
                    }
                    
                    // 几何操作
                    GroupBox {
                        title: "几何操作"
                        Layout.fillWidth: true
                        
                        RowLayout {
                            anchors.fill: parent
                            spacing: 8
                            
                            // 第一列按钮
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                
                                Button {
                                    text: "固定点和距离约束"
                                    Layout.fillWidth: true
                                    onClicked: {
                                        globalSession.createConstraint1()
                                    }
                                }
                                
                                Button {
                                    text: "共点约束"
                                    Layout.fillWidth: true
                                    onClicked: {
                                        globalSession.createGongdianConstraint()
                                    }
                                }
                                
                                Button {
                                    text: "平行约束"
                                    Layout.fillWidth: true
                                    enabled: drawingArea !== null
                                    onClicked: {
                                        globalSession.createParallelConstraint()
                                    }
                                }

                                Button {
                                    text: "点在线上约束"
                                    Layout.fillWidth: true
                                    enabled: drawingArea !== null
                                    onClicked: {
                                        globalSession.createPtInLineConstraint()
                                    }
                                }

                                Button {
                                    text: "点在圆上约束"
                                    Layout.fillWidth: true
                                    enabled: drawingArea !== null
                                    onClicked: {
                                        globalSession.createPtOnCircleConstraint()
                                    }
                                }

                                Button {
                                    text: "竖直约束"
                                    Layout.fillWidth: true
                                    enabled: drawingArea !== null
                                    onClicked: {
                                        globalSession.createVerticalConstraint()
                                    }
                                }
                            }
                            
                            // 第二列按钮
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 8
                                
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

                                Button {
                                    text: "对称约束"
                                    Layout.fillWidth: true
                                    enabled: drawingArea !== null
                                    onClicked: {
                                        // TODO: 实现对称约束
                                        statusText.text = "对称约束功能待实现"
                                        statusText.color = "orange"
                                    }
                                }

                                Button {
                                    text: "直线夹角"
                                    Layout.fillWidth: true
                                    enabled: drawingArea !== null
                                    onClicked: {
                                        // TODO: 绘制直线夹角
                                        statusText.text = "对称约束功能待实现"
                                        statusText.color = "orange"
                                    }
                                }
                            }
                        }
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "#e0e0e0"
                    }
                    
                    Button {
                        text: "清空所有"
                        Layout.fillWidth: true
                        onClicked: {
                            globalSession.clear()
                            statusText.text = "已清空"
                        }
                    }
                    
                    // 约束求解
                    GroupBox {
                        title: "约束求解"
                        Layout.fillWidth: true
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8
                            
                            Text {
                                text: "先创建约束场景，再点击应用约束"
                                font.pixelSize: 11
                                color: "#666"
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                            
                            // RowLayout {
                            //     Label { text: "目标距离:" }
                            //     TextField {
                            //         id: targetDistanceField
                            //         text: "100.0"
                            //         validator: DoubleValidator {}
                            //         Layout.fillWidth: true
                            //     }
                            // }
                            
                            Button {
                                text: "测试拖拽约束"
                                Layout.fillWidth: true
                                highlighted: true
                                onClicked: {
                                    // 根据当前约束类型测试不同的拖拽场景
                                    var points = globalSession.getPoints()
                                    if (points.length === 0) {
                                        statusText.text = "请先创建约束场景"
                                        statusText.color = "orange"
                                        return
                                    }
                                    
                                    // 测试拖拽第一个点
                                    var draggedPointId = 1
                                    var newX = 100.0  // 新的X坐标
                                    var newY = 100.0  // 新的Y坐标
                                    
                                    // 调用EaSession的solveDragConstraint方法
                                    var success = globalSession.solveDragConstraint(draggedPointId, newX, newY)
                                    
                                    if (success) {
                                        statusText.text = "拖拽约束求解成功! 点" + draggedPointId + "移动到(" + newX + "," + newY + ")"
                                        statusText.color = "green"
                                    } else {
                                        statusText.text = "拖拽约束求解失败"
                                        statusText.color = "red"
                                    }
                                }
                            }
                            
                            Text {
                                id: statusText
                                text: "等待求解..."
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }
                    
                    // 信息
                    GroupBox {
                        title: "功能说明"
                        Layout.fillWidth: true
                        
                        Text {
                            text: "• 这是 QQuickPaintedItem 实现\n" +
                                  "• 使用 C++ QPainter 绘制\n" +
                                  "• 支持完整的鼠标交互\n" +
                                  "• 高性能、流畅的操作\n" +
                                  "• 适合 CAD/几何编辑应用"
                            wrapMode: Text.WordWrap
                            font.pixelSize: 11
                            width: parent.width
                        }
                    }
                    
                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }
}


