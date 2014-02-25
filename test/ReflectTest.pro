#-------------------------------------------------
#
# Project created by QtCreator 2014-02-25T15:06:40
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_reflecttesttest
CONFIG   += console testcase
CONFIG   -= app_bundle

DEFINES += REFLECT_MACRO_IMPL

TEMPLATE = app

INCLUDEPATH += ../include/

SOURCES += ReflectTest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

win32-msvc2012 {
  # needs enabling for msvc2013, which has no mkspec yet...
  # QMAKE_CXXFLAGS += /FS
}

macx-clang {
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
  QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
}

linux-clang {
  QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
}

linux-g++ {
  QMAKE_CXXFLAGS += -std=c++0x
}

HEADERS += \
    ReflectTest.h \
    ../include/Reflect/FunctionBuilder.h \
    ../include/Reflect/Type.h \
    ../include/Reflect/EmbeddedTypes.h \
    ../include/Reflect/Cpp11/FunctionBuilderImpl.h \
    ../include/Reflect/Macro/FunctionBuilderImpl.h \
    ../include/Reflect/Macro/FunctionBuilderImplTemplate.h
