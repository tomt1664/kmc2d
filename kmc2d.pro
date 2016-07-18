QT += widgets
qtHaveModule(printsupport): QT += printsupport

HEADERS	    =   mainwindow.h \
		latsite.h \
		configscene.h \
		trans.h \
    cellsizedialog.h \
    expanddialog.h \
    curvedisplay.h
SOURCES	    =   mainwindow.cpp \
		latsite.cpp \
		main.cpp \
		trans.cpp \
		configscene.cpp \
    cellsizedialog.cpp \
    expanddialog.cpp \
    curvedisplay.cpp
RESOURCES   =	kmc2d.qrc


# install
target.path = /home/tt200/kmc2d/
INSTALLS += target
