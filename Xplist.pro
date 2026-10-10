#-------------------------------------------------
# Project created by QtCreator 2012-10-06T16:37:20
# 开始升级于2020.10.15
#-------------------------------------------------
QT       += core gui
QT       += xml
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
INCLUDEPATH += $$PWD
contains(CONFIG, arm64_self_use) {
    DEFINES += XPLIST_SELF_USE
    QMAKE_APPLE_DEVICE_ARCHS = arm64
    QMAKE_INFO_PLIST = $$PWD/Info.plist
}

ICON = my.icns
RC_FILE += myapp.rc
RC_FILE +=

TARGET = Xplist
TEMPLATE = app

TRANSLATIONS = translation_ru.ts \
               translation_cn.ts

SOURCES += main.cpp\
    MyTabBar.cpp \
    MyTabPopup.cpp \
    Plist.cpp \
    PlistDate.cpp \
    autoupdatedialog.cpp \
    codeeditor.cpp \
    comboboxdelegatebool.cpp \
    filesystemwatcher.cpp \
    mainwindow.cpp \
    domparser.cpp \
    dommodel.cpp \
    domitem.cpp \
    comboboxdelegate.cpp \
    lineeditdelegate.cpp \
    editortab.cpp \
    editortabswidget.cpp \
    commands.cpp \
    itemstate.cpp \
    myapp.cpp \
    myhighlighter.cpp \
    mytreeview.cpp \
    pugixml.cpp \
    recentfiles.cpp \
    plistio.cpp

HEADERS  += mainwindow.h \
    MyTabBar.h \
    MyTabPopup.h \
    Plist.hpp \
    PlistDate.hpp \
    autoupdatedialog.h \
    base64.hpp \
    codeeditor.h \
    comboboxdelegatebool.h \
    domparser.h \
    dommodel.h \
    domitem.h \
    comboboxdelegate.h \
    filesystemwatcher.h \
    lineeditdelegate.h \
    editortab.h \
    editortabswidget.h \
    commands.h \
    itemstate.h \
    myapp.h \
    myhighlighter.h \
    mytreeview.h \
    pugiconfig.hpp \
    pugixml.hpp \
    recentfiles.h \
    plistio.h \
    qtcompat.h

FORMS    += mainwindow.ui \
    autoupdatedialog.ui \
    editortab.ui

RESOURCES += \
    res.qrc

DISTFILES += \
    Info.plist \
    myapp.rc

mac {
    # Only include / compile these files on OS X
    OBJECTIVE_SOURCES +=
    HEADERS  +=

    # Additionally include Cocoa for OS X code

    LIBS += -framework Foundation -framework Cocoa
    INCLUDEPATH += /System/Library/Frameworks/Foundation.framework/Versions/C/Headers
}


CONFIG(debug,debug|release) {
    isEmpty(DESTDIR): DESTDIR = $$absolute_path($${_PRO_FILE_PWD_}/bin/debug)
} else {
    isEmpty(DESTDIR): DESTDIR = $$absolute_path($${_PRO_FILE_PWD_}/bin/release)
}

