/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
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

#ifndef QGEOSERVICEPROVIDER_P_H
#define QGEOSERVICEPROVIDER_P_H

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

#include "qgeoserviceprovider.h"

#include <QHash>

QT_BEGIN_NAMESPACE

class QGeoSearchManager;
class QGeoRoutingManager;
class QGeoMappingManager;

class QGeoServiceProviderFactory;

class QGeoServiceProviderPrivate
{
public:
    QGeoServiceProviderPrivate();
    ~QGeoServiceProviderPrivate();

    void loadPlugin(const QString &providerName, const QMap<QString, QVariant> &parameters);

    QGeoServiceProviderFactory *factory;

    QMap<QString, QVariant> parameterMap;

    QGeoSearchManager *searchManager;
    QGeoRoutingManager *routingManager;
    QGeoMappingManager *mappingManager;
    QPlaceManager *placeManager;

    QGeoServiceProvider::Error searchError;
    QGeoServiceProvider::Error routingError;
    QGeoServiceProvider::Error mappingError;
    QGeoServiceProvider::Error placeError;

    QString searchErrorString;
    QString routingErrorString;
    QString mappingErrorString;
    QString placeErrorString;

    QGeoServiceProvider::Error error;
    QString errorString;

    static QHash<QString, QGeoServiceProviderFactory*> plugins(bool reload = false);
    static void loadDynamicPlugins(QHash<QString, QGeoServiceProviderFactory*> *plugins);
    static void loadStaticPlugins(QHash<QString, QGeoServiceProviderFactory*> *plugins);
};

QT_END_NAMESPACE

#endif
