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

TEMPLATE = app

INCLUDEPATH += ../include/

SOURCES += Lua.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

DEFINES += REFLECT_MACRO_IMPL

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
  INCLUDEPATH += /usr/local/clang-3.3/lib/c++/v1/
}

linux-g++ {
  QMAKE_CXXFLAGS += -std=c++0x
}

HEADERS += \
    lua.h \
    luaconf.h \
    ../include/Reflect/FunctionBuilder.h \
    ../include/Reflect/Type.h \
    ../include/Reflect/EmbeddedTypes.h \
    ../include/Reflect/Cpp11/FunctionBuilderImpl.h \
    ../include/Reflect/Macro/FunctionBuilderImpl.h \
    ../include/Reflect/Macro/FunctionBuilderImplTemplate.h
