import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Mathor.Solver 1.0

Window {
    width: 800
    height: 600
    visible: true
    title: qsTr("Mathor - 几何求解器演示")
    
    // 创建一个本地的求解器实例
    GeometrySolver {
        id: solver
        
        onSolvingFinished: function(success) {
            if (success) {
                var points = solver.getSolvedPoints()
                resultText.text = "求解成功!\n\n" +
                    "点1: (" + points.x1.toFixed(2) + ", " + points.y1.toFixed(2) + ")\n" +
                    "点2: (" + points.x2.toFixed(2) + ", " + points.y2.toFixed(2) + ")\n" +
                    "自由度: " + solver.dof
                
                // 更新可视化
                canvas.requestPaint()
            } else {
                resultText.text = "求解失败: " + solver.lastError
            }
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15
        
        // 标题
        Text {
            text: "SolveSpaceLib 几何约束求解器"
            font.pixelSize: 24
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }
        
        // 输入区域
        GroupBox {
            title: "输入参数"
            Layout.fillWidth: true
            
            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 8
                
                Label { text: "点1 X:" }
                TextField {
                    id: x1Input
                    text: "10"
                    placeholderText: "X坐标"
                    validator: DoubleValidator {}
                    Layout.fillWidth: true
                }
                
                Label { text: "点1 Y:" }
                TextField {
                    id: y1Input
                    text: "20"
                    placeholderText: "Y坐标"
                    validator: DoubleValidator {}
                    Layout.fillWidth: true
                }
                
                Label { text: "点2 X:" }
                TextField {
                    id: x2Input
                    text: "50"
                    placeholderText: "X坐标"
                    validator: DoubleValidator {}
                    Layout.fillWidth: true
                }
                
                Label { text: "点2 Y:" }
                TextField {
                    id: y2Input
                    text: "60"
                    placeholderText: "Y坐标"
                    validator: DoubleValidator {}
                    Layout.fillWidth: true
                }
                
                Label { text: "目标距离:" }
                TextField {
                    id: distanceInput
                    text: "100.0"
                    placeholderText: "距离"
                    validator: DoubleValidator {}
                    Layout.fillWidth: true
                }
            }
        }
        
        // 求解按钮
        Button {
            text: "求解约束"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 200
            highlighted: true
            
            onClicked: {
                solver.solveSimple2DDistance(
                    parseFloat(x1Input.text),
                    parseFloat(y1Input.text),
                    parseFloat(x2Input.text),
                    parseFloat(y2Input.text),
                    parseFloat(distanceInput.text)
                )
            }
        }
        
        // 结果显示区域
        GroupBox {
            title: "求解结果"
            Layout.fillWidth: true
            
            ScrollView {
                anchors.fill: parent
                
                TextArea {
                    id: resultText
                    text: "点击'求解约束'按钮开始求解..."
                    readOnly: true
                    wrapMode: TextArea.WordWrap
                    font.family: "Consolas"
                    font.pixelSize: 12
                }
            }
        }
        
        // 可视化画布
        GroupBox {
            title: "可视化"
            Layout.fillWidth: true
            Layout.fillHeight: true
            
            Canvas {
                id: canvas
                anchors.fill: parent
                
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    
                    // 绘制网格
                    ctx.strokeStyle = "#e0e0e0"
                    ctx.lineWidth = 1
                    for (var i = 0; i < width; i += 20) {
                        ctx.beginPath()
                        ctx.moveTo(i, 0)
                        ctx.lineTo(i, height)
                        ctx.stroke()
                    }
                    for (var j = 0; j < height; j += 20) {
                        ctx.beginPath()
                        ctx.moveTo(0, j)
                        ctx.lineTo(width, j)
                        ctx.stroke()
                    }
                    
                    // 绘制求解后的点和连线
                    var points = solver.getSolvedPoints()
                    if (points.x1 !== undefined) {
                        var scale = 2
                        var offsetX = width / 2 - 50
                        var offsetY = height / 2
                        
                        var px1 = points.x1 * scale + offsetX
                        var py1 = -points.y1 * scale + offsetY  // Y轴翻转
                        var px2 = points.x2 * scale + offsetX
                        var py2 = -points.y2 * scale + offsetY
                        
                        // 绘制连线
                        ctx.strokeStyle = "#2196F3"
                        ctx.lineWidth = 2
                        ctx.beginPath()
                        ctx.moveTo(px1, py1)
                        ctx.lineTo(px2, py2)
                        ctx.stroke()
                        
                        // 绘制点1
                        ctx.fillStyle = "#4CAF50"
                        ctx.beginPath()
                        ctx.arc(px1, py1, 6, 0, Math.PI * 2)
                        ctx.fill()
                        
                        // 绘制点2
                        ctx.fillStyle = "#F44336"
                        ctx.beginPath()
                        ctx.arc(px2, py2, 6, 0, Math.PI * 2)
                        ctx.fill()
                        
                        // 标签
                        ctx.fillStyle = "#000000"
                        ctx.font = "12px Arial"
                        ctx.fillText("P1", px1 + 10, py1)
                        ctx.fillText("P2", px2 + 10, py2)
                    }
                }
            }
        }
    }
}
