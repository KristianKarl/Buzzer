TEMPLATE = app
QT = gui core uitools multimedia
CONFIG += qt release warn_on console
DESTDIR = bin
OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build
INCLUDEPATH+=X11 Xtst Xext
LIBS+= -lX11 -lXtst
FORMS = ui/mainwindow.ui
HEADERS = src/mainwindowimpl.h \
    src/joystick.h
SOURCES = src/mainwindowimpl.cpp src/main.cpp \
    src/joystick.cpp
