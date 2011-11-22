
INCLUDEPATH += mapsgl

QT += network qt3d

include(map2d/map2d.pri)

SOURCES += \
    mapsgl/cameradata.cpp \
    mapsgl/frustum.cpp \
    mapsgl/map.cpp \
    mapsgl/mapcontroller.cpp \
    mapsgl/mapsphere.cpp \
    mapsgl/projection.cpp \
    mapsgl/tilecache.cpp \
    mapsgl/tile.cpp \
    mapsgl/tilespec.cpp \
    mapsgl/maptype.cpp

PUBLIC_HEADERS += \
    mapsgl/cameradata.h \
    mapsgl/map.h \
    mapsgl/mapcontroller.h \
    mapsgl/tilecache.h \
    mapsgl/tile.h \
    mapsgl/tilespec.h \
    mapsgl/maptype.h

PRIVATE_HEADERS += \
    mapsgl/frustum_p.h \
    mapsgl/map_p.h \
    mapsgl/mapsphere_p.h \
    mapsgl/projection_p.h
