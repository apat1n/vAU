QT += sql
QT += core
QT += websockets

TARGET = server

CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/server.cpp \
    src/db_utils.cpp \
    src/server_processes.cpp \
    src/utils.cpp

HEADERS += \
    include/models/message.h \
    include/server.h \
    include/models/common.h \
    include/models/user.h \
    include/models/chat.h

INCLUDEPATH += include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
