# -------------------------------------------------
# Project created by QtCreator 2009-06-25T16:20:38
# -------------------------------------------------
QT += sql
TARGET = SmsSync
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    LocalDataBase.cpp \
    mz_commonfunc.cpp \
    ui_password.cpp
HEADERS += mainwindow.h \
    ui_mainwindow.h \
    sqlite3/sqlite3.h \
    LocalDataBase.h \
    mz_commonfunc.h \
    ui_password.h \
    build/ui_ui_password.h \
    build/ui_mainwindow.h
FORMS += mainwindow.ui \
    ui_password.ui
RESOURCES += res.qrc
LIBS += libsqlite3
