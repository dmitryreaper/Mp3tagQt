QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MP3Tag
TEMPLATE = app

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    form.cpp

HEADERS += \
        mainwindow.h \
    form.h

FORMS += \
        mainwindow.ui \
    form.ui

RESOURCES += \
    res.qrc
