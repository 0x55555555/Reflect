#-------------------------------------------------
#
# Project created by QtCreator 2014-02-25T15:06:40
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_reflecttesttest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += ../include/

SOURCES += ReflectTest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

win32-msvc2012 {
  # needs enabling for msvc2013, which has no mkspec yet...
  # QMAKE_CXXFLAGS += /FS
}

HEADERS += \
    ReflectTest.h \
    ../include/Reflect/FunctionBuilder.h \
    ../include/Reflect/Type.h \
    ../include/Reflect/EmbeddedTypes.h
