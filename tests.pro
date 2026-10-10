include(Xplist.pro)
TARGET = XplistTests
QT += testlib
CONFIG -= app_bundle
CONFIG += testcase
DEFINES += XPLIST_SELF_USE XPLIST_TESTS
SOURCES -= main.cpp
SOURCES += tests/tst_xplist.cpp
QMAKE_INFO_PLIST =
