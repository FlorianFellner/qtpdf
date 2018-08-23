TEMPLATE = app
TARGET = pdfviewer
QT += core gui widgets widgets-private pdfwidgets printsupport

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    pageselector.cpp \
    zoomselector.cpp \
    thumbnails.cpp

HEADERS += \
    mainwindow.h \
    pageselector.h \
    zoomselector.h \
    thumbnails.h \
    thumbnails_p.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    resources.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/pdf/pdfviewer
INSTALLS += target
