#-------------------------------------------------
#
# Project created by QtCreator 2015-01-16T18:09:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtExampleApp
TEMPLATE = app

INCLUDEPATH += $$_PRO_FILE_PWD_/../../include
LIBS += -L$$_PRO_FILE_PWD_/../../$$CONFIG

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
