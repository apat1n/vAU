QT += sql
QT += core
QT += websockets

TARGET = server

CONFIG += c++17
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    server.cpp \
    db_utils.cpp

HEADERS += \
    models/message.h \
    server.h \
    models/common.h \
    models/user.h \
    models/chat.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
