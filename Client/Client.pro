QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/StartWidget.cpp \
    src/RecordItemWidget.cpp \
    src/FileItemWidget.cpp \
    src/ClientChatWidget.cpp \
    src/ChatItemWidget.cpp \
    src/ChatProtocol.cpp \
    src/ClientManager.cpp \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/StartWidget.h \
    src/RecordItemWidget.h \
    src/FileItemWidget.h \
    src/ClientChatWidget.h \
    src/ChatItemWidget.h \
    src/ChatProtocol.h \
    src/ClientManager.h \
    src/mainwindow.h

FORMS += \
    ui/StartWidget.ui \
    ui/RecordItemWidget.ui \
    ui/FileItemWidget.ui \
    ui/ClientChatWidget.ui \
    ui/ChatItemWidget.ui \
    ui/mainwindow.ui

LIBS += $$PWD/openssl/libcrypto.a -lws2_32 -lcrypt32
LIBS += $$PWD/openssl/libssl.a

INCLUDEPATH += $$PWD/openssl

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc
