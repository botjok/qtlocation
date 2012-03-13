/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "qgeotiledmapdata_p.h"
#include "qgeotiledmapdata_p_p.h"

#include "qgeotiledmappingmanagerengine.h"
#include "qgeotilecache_p.h"
#include "qgeotilespec.h"
#include "qgeoprojection_p.h"

#include "qgeocameratiles_p.h"
#include "qgeomapimages_p.h"
#include "qgeomapgeometry_p.h"
#include "qgeocoordinateinterpolator_p.h"
#include "qgeoprojection_p.h"
#include "qdoublevector2d_p.h"
#include "qgeocameracapabilities_p.h"

#include <QMutex>
#include <QMap>

#include <qnumeric.h>

#include <Qt3D/qglscenenode.h>
#include <Qt3D/qgeometrydata.h>
#include <Qt3D/qglbuilder.h>
#include <Qt3D/qglpainter.h>
#include <Qt3D/qgeometrydata.h>
#include <Qt3D/qglbuilder.h>
#include <Qt3D/qglcamera.h>
#include <Qt3D/qglsubsurface.h>

#include <cmath>

QT_BEGIN_NAMESPACE

class QGeoCoordinateInterpolator2D : public QGeoCoordinateInterpolator
{
public:
    QGeoCoordinateInterpolator2D();
    virtual ~QGeoCoordinateInterpolator2D();

    virtual QGeoCoordinate interpolate(const QGeoCoordinate &start, const QGeoCoordinate &end, qreal progress);
};

QGeoCoordinateInterpolator2D::QGeoCoordinateInterpolator2D() {}

QGeoCoordinateInterpolator2D::~QGeoCoordinateInterpolator2D() {}

QGeoCoordinate QGeoCoordinateInterpolator2D::interpolate(const QGeoCoordinate &start, const QGeoCoordinate &end, qreal progress)
{
    if (start == end) {
        if (progress < 0.5) {
            return start;
        } else {
            return end;
        }
    }

    QGeoCoordinate s2 = start;
    QGeoCoordinate e2 = end;
    QDoubleVector2D s = QGeoProjection::coordToMercator(s2);
    QDoubleVector2D e = QGeoProjection::coordToMercator(e2);

    double x = s.x();

    if (0.5 < qAbs(e.x() - s.x())) {
        // handle dateline crossing
        double ex = e.x();
        double sx = s.x();
        if (ex < sx)
            sx -= 1.0;
        else if (sx < ex)
            ex -= 1.0;

        x = (1.0 - progress) * sx + progress * ex;

        if (!qFuzzyIsNull(x) && (x < 0.0))
            x += 1.0;

    } else {
        x = (1.0 - progress) * s.x() + progress * e.x();
    }

    double y = (1.0 - progress) * s.y() + progress * e.y();

    QGeoCoordinate result = QGeoProjection::mercatorToCoord(QDoubleVector2D(x, y));
    result.setAltitude((1.0 - progress) * start.altitude() + progress * end.altitude());
    return result;
}

//------------------------
//------------------------

QGeoTiledMapData::QGeoTiledMapData(QGeoTiledMappingManagerEngine *engine, QObject *parent)
    : QGeoMapData(engine, parent)
{
    d_ptr = new QGeoTiledMapDataPrivate(this, engine);
    engine->registerMap(this);
    setCoordinateInterpolator(QSharedPointer<QGeoCoordinateInterpolator>(new QGeoCoordinateInterpolator2D()));
}

QGeoTiledMapData::~QGeoTiledMapData()
{
    delete d_ptr;
}

void QGeoTiledMapData::tileFetched(const QGeoTileSpec &spec)
{
    Q_D(QGeoTiledMapData);
    d->tileFetched(spec);
}

QGeoTileCache* QGeoTiledMapData::tileCache()
{
    Q_D(QGeoTiledMapData);
    return d->tileCache();
}

void QGeoTiledMapData::paintGL(QGLPainter *painter)
{
    Q_D(QGeoTiledMapData);
    d->paintGL(painter);
}

void QGeoTiledMapData::mapResized(int width, int height)
{
    Q_D(QGeoTiledMapData);
    d->resized(width, height);
}

void QGeoTiledMapData::changeCameraData(const QGeoCameraData &oldCameraData)
{
    Q_D(QGeoTiledMapData);
    d->changeCameraData(oldCameraData);
}

void QGeoTiledMapData::changeActiveMapType(const QGeoMapType mapType)
{
    Q_D(QGeoTiledMapData);
    d->changeActiveMapType(mapType);
}

QGeoCoordinate QGeoTiledMapData::screenPositionToCoordinate(const QPointF &pos, bool clipToViewport) const
{
    Q_D(const QGeoTiledMapData);
    if (clipToViewport) {
        int w = width();
        int h = height();

        if ((pos.x() < 0) || (w < pos.x()) || (pos.y() < 0) || (h < pos.y()))
            return QGeoCoordinate();
    }

    return d->screenPositionToCoordinate(pos);
}

QPointF QGeoTiledMapData::coordinateToScreenPosition(const QGeoCoordinate &coordinate, bool clipToViewport) const
{
    Q_D(const QGeoTiledMapData);
    QPointF pos = d->coordinateToScreenPosition(coordinate);

    if (clipToViewport) {
        int w = width();
        int h = height();

        if ((pos.x() < 0) || (w < pos.x()) || (pos.y() < 0) || (h < pos.y()))
            return QPointF(qQNaN(), qQNaN());
    }

    return pos;
}

void QGeoTiledMapData::updateTileRequests(const QSet<QGeoTileSpec> &tilesAdded, const QSet<QGeoTileSpec> &tilesRemoved)
{
    static_cast<QGeoTiledMappingManagerEngine*>(engine())->updateTileRequests(this, tilesAdded, tilesRemoved);
}

QGeoTiledMapDataPrivate::QGeoTiledMapDataPrivate(QGeoTiledMapData *parent, QGeoTiledMappingManagerEngine *engine)
    : map_(parent),
      cache_(engine->tileCache()),
      cameraTiles_(new QGeoCameraTiles()),
      mapGeometry_(new QGeoMapGeometry()),
      mapImages_(new QGeoMapImages(parent, engine->tileCache()))
{
    cameraTiles_->setMaximumZoomLevel(static_cast<int>(ceil(engine->cameraCapabilities().maximumZoomLevel())));
    cameraTiles_->setTileSize(engine->tileSize().width());
    cameraTiles_->setPluginString(map_->pluginString());

    mapGeometry_->setTileSize(engine->tileSize().width());
}

QGeoTiledMapDataPrivate::~QGeoTiledMapDataPrivate()
{
    // controller_ is a child of map_, don't need to delete it here

    delete mapImages_;
    delete mapGeometry_;
    delete cameraTiles_;

    // TODO map items are not deallocated!
    // However: how to ensure this is done in rendering thread?
}

QGeoTileCache* QGeoTiledMapDataPrivate::tileCache()
{
    return cache_;
}

void QGeoTiledMapDataPrivate::changeCameraData(const QGeoCameraData &oldCameraData)
{
    double lat = oldCameraData.center().latitude();

    if (mapGeometry_->verticalLock()) {
        QGeoCoordinate coord = map_->cameraData().center();
        coord.setLatitude(lat);
        map_->cameraData().setCenter(coord);
    }

    cameraTiles_->setCamera(map_->cameraData());
    visibleTiles_ = cameraTiles_->tiles();

    mapGeometry_->setCameraData(map_->cameraData());
    mapGeometry_->setVisibleTiles(visibleTiles_);

    if (mapImages_) {
        mapImages_->setVisibleTiles(visibleTiles_);

        //QSet<QGeoTileSpec> cachedTiles = mapImages_->cachedTiles();
        // TODO make this more efficient
        QSet<QGeoTileSpec> cachedTiles = visibleTiles_;

        typedef QSet<QGeoTileSpec>::const_iterator iter;
        iter i = cachedTiles.constBegin();
        iter end = cachedTiles.constEnd();
        for (; i != end; ++i) {
            QGeoTileSpec tile = *i;
            if (cache_->contains(tile))
                mapGeometry_->addTile(tile, cache_->get(tile));
        }

        if (!cachedTiles.isEmpty())
            map_->update();

    }
}

void QGeoTiledMapDataPrivate::changeActiveMapType(const QGeoMapType mapType)
{
    cameraTiles_->setMapType(mapType);
    visibleTiles_ = cameraTiles_->tiles();
}

void QGeoTiledMapDataPrivate::resized(int width, int height)
{
    cameraTiles_->setScreenSize(QSize(width, height));
    mapGeometry_->setScreenSize(QSize(width, height));
    map_->setCameraData(map_->cameraData());
}

void QGeoTiledMapDataPrivate::tileFetched(const QGeoTileSpec &spec)
{
    if (cache_->contains(spec))
        mapGeometry_->addTile(spec, cache_->get(spec));
    mapImages_->tileFetched(spec);
    map_->update();
}

void QGeoTiledMapDataPrivate::paintGL(QGLPainter *painter)
{
    mapGeometry_->paintGL(painter);
}

QGeoCoordinate QGeoTiledMapDataPrivate::screenPositionToCoordinate(const QPointF &pos) const
{
    return QGeoProjection::mercatorToCoord(mapGeometry_->screenPositionToMercator(pos));
}

QPointF QGeoTiledMapDataPrivate::coordinateToScreenPosition(const QGeoCoordinate &coordinate) const
{
    return mapGeometry_->mercatorToScreenPosition(QGeoProjection::coordToMercator(coordinate));
}

QT_END_NAMESPACE