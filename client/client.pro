QT += core gui
QT += websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/chat.cpp \
    src/client.cpp \
    src/createChat.cpp \
    src/client_processes.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/message.cpp \
    src/updateprofile.cpp \
    src/utils.cpp

HEADERS += \
    include/chat.h \
    include/client.h \
    include/createChat.h \
    include/mainwindow.h \
    include/message.h \
    include/updateprofile.h \
    include/user.h

FORMS += \
    ui/mainwindow.ui \
    ui/createChat.ui \
    ui/updateprofile.ui

INCLUDEPATH += include
UI_DIR = ui

RESOURCES = resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../../../Загрузки/style.qss \
    ../../../Загрузки/style0.qss
