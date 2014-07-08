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
  DEFINES += REFLECT_MACRO_IMPL _VARIADIC_MAX=10
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
    ../include/Reflect/Cpp11/FunctionBuilderImpl.h \
    ../include/Reflect/Macro/FunctionBuilderImpl.h \
    ../include/Reflect/Macro/FunctionBuilderImplTemplate.h \
    ../include/Reflect/MethodInjectorBuilder.h \
    ../include/Reflect/Utils/ReturnPacker.h \
    ../include/Reflect/Utils/TupleEach.h \
    ../include/Reflect/CanCallHelper.h \
    ../include/Reflect/Utils/Exceptions.h \
    ../include/Reflect/FunctionSelector.h \
    ../include/Reflect/Cpp11/FunctionSelectorImpl.h \
    ../include/Reflect/Macro/FunctionSelectorImpl.h \
    ../include/Reflect/WrappedFunction.h \
    ../include/Crate/Type.h \
    ../include/Crate/EmbeddedTypes.h \
    ../include/Crate/BaseTraits.h \
    ../include/Crate/CopyTraits.h \
    ../include/Crate/ReferenceTraits.h \
    ../include/Crate/Traits.h \
    ../include/Crate/ReferenceNonCleanedTraits.h \
    ../include/Crate/DerivedTraits.h \
    ../include/Crate/Exceptions.h \
    ../example/Default/Builder.h \
    ../include/Reflect/Reflect.h \
    CrateTypeHelper.h
