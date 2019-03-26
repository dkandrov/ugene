# include (U2View.pri)

UGENE_RELATIVE_DESTDIR = ''
MODULE_ID=U2View
include( ../../ugene_lib_common.pri )

QT += xml svg widgets printsupport

useWebKit() {
    QT += webkitwidgets
} else {
    QT += webengine webenginewidgets
}

DEFINES+= QT_FATAL_ASSERT BUILDING_U2VIEW_DLL
LIBS += -L../../$$out_dir()
LIBS += -lU2Core$$D -lU2Algorithm$$D -lU2Formats$$D -lU2Lang$$D -lU2Gui$$D

unix: QMAKE_CXXFLAGS += -Wno-char-subscripts

DESTDIR = ../../$$out_dir()

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

