QT += quick quickcontrols2

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        GeometrySolver.cpp \
        main/eadrawingarea.cpp

HEADERS += \
        GeometrySolver.h \
        main/eadrawingarea.h

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# ========== SolveSpaceLib集成配置 ==========
# 添加SolveSpaceLib头文件路径
INCLUDEPATH += $$PWD/SolveSpaceLib/libslvs/include

# 定义使用共享库
DEFINES += SLVS_LIB_SHARED

# 根据构建类型链接对应的库文件
CONFIG(debug, debug|release) {
    # Debug模式
    LIBS += -L$$PWD/SolveSpaceLib/build/Debug -llibslvs
    # 将DLL复制到输出目录
    QMAKE_POST_LINK += $$quote($$QMAKE_COPY $$shell_path($$PWD/SolveSpaceLib/build/Debug/libslvs.dll) $$shell_path($$OUT_PWD/debug/)$$escape_expand(\n\t))
} else {
    # Release模式
    LIBS += -L$$PWD/SolveSpaceLib/build/Release -llibslvs
    # 将DLL复制到输出目录
    QMAKE_POST_LINK += $$quote($$QMAKE_COPY $$shell_path($$PWD/SolveSpaceLib/build/Release/libslvs.dll) $$shell_path($$OUT_PWD/release/)$$escape_expand(\n\t))
}

# 添加依赖关系（确保先编译SolveSpaceLib）
PRE_TARGETDEPS += $$PWD/SolveSpaceLib/build/Debug/libslvs.lib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
