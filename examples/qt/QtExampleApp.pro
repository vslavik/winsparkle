#-------------------------------------------------
#
# Project created by QtCreator 2015-01-16T18:09:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtExampleApp
TEMPLATE = app

CONFIG(debug, debug|release) {
    LIB_DIR=Debug
} else {
    LIB_DIR=Release
}

contains(QMAKE_TARGET.arch, x86_64) {
    LIB_DIR=x64/$$LIB_DIR
}

INCLUDEPATH += $$_PRO_FILE_PWD_/../../include
LIBS += -L$$_PRO_FILE_PWD_/../../$$LIB_DIR -lWinSparkle

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += resources.qrc
