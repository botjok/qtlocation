/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QGEOTILECACHE_P_H
#define QGEOTILECACHE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLocation/qlocationglobal.h>

#include <QObject>
#include <QCache>
#include <QSet>

#include "qgeomappingmanager.h"

QT_BEGIN_NAMESPACE

class QGeoMappingManager;

class QGeoTileSpec;
class QGeoTile;

class TileDisk;
class TileMemory;
class TileTexture;

class QGLSceneNode;

class QPixmap;
class QThread;

class Q_LOCATION_EXPORT QGeoTileCache : public QObject
{
    Q_OBJECT
public:
    QGeoTileCache(const QString &directory = QString(), QObject *parent = 0);
    ~QGeoTileCache();

    void setMaxDiskUsage(int diskUsage);
    int maxDiskUsage() const;
    int diskUsage() const;

    void setMaxMemoryUsage(int memoryUsage);
    int maxMemoryUsage() const;
    int memoryUsage() const;

    void setMaxTextureUsage(int textureUsage);
    int maxTextureUsage() const;
    int textureUsage() const;

    void GLContextAvailable(QGLSceneNode *parentNode);

    bool contains(const QGeoTileSpec &spec) const;
    QGeoTile get(const QGeoTileSpec &spec);

    void update(const QGeoTileSpec &spec, const QGeoTile &tile);

    void evictFromDiskCache(TileDisk *td);
    void evictFromMemoryCache(TileMemory *tm);
    void evictFromTextureCache(TileTexture *tt);

    void insert(const QGeoTileSpec &spec, const QByteArray &bytes, QGeoMappingManager::CacheAreas areas = QGeoMappingManager::AllCaches);
    void handleError(const QGeoTileSpec &spec, const QString &errorString);

private:
    void loadTiles();

    TileDisk* addToDiskCache(const QGeoTileSpec &spec, const QString &filename);
    TileMemory* addToMemoryCache(const QGeoTileSpec &spec, const QPixmap &pixmap);
    TileTexture* addToTextureCache(const QGeoTileSpec &spec, const QPixmap &pixmap);

    static QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &directory);
    static QGeoTileSpec filenameToTileSpec(const QString &filename);

    QString directory_;

    QSet<QGeoTileSpec> keys_;
    QCache<QGeoTileSpec, TileDisk > diskCache_;
    QCache<QGeoTileSpec, TileMemory > memoryCache_;
    QCache<QGeoTileSpec, TileTexture > textureCache_;

    QList<QGeoTile> cleanupList_;
};

QT_END_NAMESPACE

#endif // QGEOTILECACHE_P_H