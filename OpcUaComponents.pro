include(open62541.pri)

QT       += core gui widgets uiplugin

TARGET = OpcUaComponents
CONFIG(debug, debug|release) {
    win32: TARGET = $$join(TARGET,,,d)
}
TEMPLATE = lib
CONFIG += plugin

DESTDIR = $$[QT_INSTALL_PLUGINS]/designer

DEFINES += QT_DEPRECATED_WARNINGS

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += \
    opcuaconnectionplugin.cpp \
    opcuaconnection.cpp \
    opccomponentsplugin.cpp \
    opcuamonitoreditemplugin.cpp \
    opcuamonitoreditem.cpp

HEADERS += \
    opcuaconnectionplugin.h \
    opcuaconnection.h \
    opccomponentsplugin.h \
    opcuamonitoreditemplugin.h \
    opcuamonitoreditem.h

OTHER_FILES += OpcUaComponents.json

header_files.path = $$[QT_INSTALL_HEADERS]
header_files.files = \
    opcuaconnection.h \
    opcuamonitoreditem.h

unix {
    target.path = /usr/lib
}
INSTALLS += target header_files

DISTFILES += \
    open62541.pri
