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

SOURCES += ReflectTest.cpp \
    TyperTest.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"

#DEFINES += REFLECT_MACRO_IMPL

win32-msvc2012 {
  # needs enabling for msvc2013, which has no mkspec yet...
  # QMAKE_CXXFLAGS += /FS
}

macx-clang {
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
  QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
}

linux-clang {
  QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
  INCLUDEPATH += /usr/include/c++/4.6/ /usr/include/c++/4.6/x86_64-linux-gnu/32/
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
    ../include/Reflect/Macro/FunctionBuilderImplTemplate.h \
    ../include/Crate/BaseTraits.h \
    ../include/Crate/CopyTraits.h \
    ../include/Crate/ReferenceTraits.h \
    ../include/Crate/Traits.h \
    ../include/Crate/ReferenceNonCleanedTraits.h \
    ../include/Reflect/MethodInjectorBuilder.h \
    ../include/Reflect/ReturnPacker.h \
    ../include/Crate/DerivedTraits.h \
    ../example/Default/Builder.h \
    ../include/Reflect/TupleEach.h \
    ../include/Reflect/CanCallHelper.h \
    ../include/Crate/Exceptions.h \
    ../include/Reflect/Exceptions.h \
    ../include/Reflect/FunctionSelector.h \
    ../include/Reflect/Cpp11/FunctionSelectorImpl.h
