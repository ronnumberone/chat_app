QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/ChatItemWidget.cpp \
    src/ChatProtocol.cpp \
    src/ClientManager.cpp \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/ChatItemWidget.h \
    src/ChatProtocol.h \
    src/ClientManager.h \
    src/mainwindow.h

FORMS += \
    ui/ChatItemWidget.ui \
    ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
