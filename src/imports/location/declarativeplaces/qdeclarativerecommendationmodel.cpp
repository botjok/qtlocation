#include "qdeclarativerecommendationmodel_p.h"
#include "qdeclarativegeoserviceprovider_p.h"

#include <QtDeclarative/QDeclarativeInfo>
#include <QtLocation/QGeoServiceProvider>

QT_USE_NAMESPACE

/*!
    \qmlclass RecommenadationModel QDeclarativeRecommenadationModel
    \brief The RecommenadationModel element provides access to recommendation.
    \inherits QAbstractListModel
    \ingroup qml-places

    RecommenadationModel provides a model of recommendation results from PlaceManager.
    The contents of the model is SearchResult list. User can add additional parameters
    which make search more satisfactory. At least position of search center
    should be set. Other parameters are start and limit of returned items, and bounding box
    for the items returned. Also user might set \l dym property if "did you mean"
    items should be returned.

    There are two ways of accessing the data: through model by using views and delegates,
    or alternatively via \l results list property. Of the two, the model access is preferred.

    At the moment only data role provided by the model is \c searchResult.

    Model might be use for different kind of requests. It mighe be used for search request
    with search term where user gives string with query. It might be used for category search.
    User might also use it for recomendation search (search similar places in requested area).
    In that case user pass place id as an input parameter.

    To use RecommenadationModel user need to create it in qml file and connect it to some view
    \code
    import places 1.0

    RecommenadationModel {
        id: searchModel
        searchArea: BoundingCircle {
            center: Coordinate {
                longitude: 53
                latitude: 100
            }
            radius:5000
        }
        start: 0
        limit: 15
    }

    ...
    resultModel.executeQuery()
    ...

    ListView {
        id: suggestionList
        model: suggestionModel
        delegate: Text {
            text: 'Name: ' + searchResult.place.name }
        }
    }
    \endcode

    \sa SuggestionModel, SupportedCategoryModel, {QPlaceManager}
*/
QDeclarativeRecommendationModel::QDeclarativeRecommendationModel(QObject *parent)
:   QAbstractListModel(parent), m_response(0), m_plugin(0), m_complete(false)
{
    QHash<int, QByteArray> roleNames;
    roleNames = QAbstractItemModel::roleNames();
    roleNames.insert(SearchResultRole, "searchResult");
    setRoleNames(roleNames);
}

QDeclarativeRecommendationModel::~QDeclarativeRecommendationModel()
{
}

// From QDeclarativeParserStatus
void QDeclarativeRecommendationModel::componentComplete()
{
    m_complete = true;
}


void QDeclarativeRecommendationModel::setPlugin(QDeclarativeGeoServiceProvider *plugin)
{
    if (m_plugin == plugin)
        return;

    reset(); // reset the model
    m_plugin = plugin;
    if (m_complete)
        emit pluginChanged();
    QGeoServiceProvider *serviceProvider = m_plugin->sharedGeoServiceProvider();
    QPlaceManager *placeManager = serviceProvider->placeManager();
    if (!placeManager || serviceProvider->error() != QGeoServiceProvider::NoError) {
        qmlInfo(this) << tr("Warning: Plugin does not support places.");
        return;
    }
}

QDeclarativeGeoServiceProvider* QDeclarativeRecommendationModel::plugin() const
{
    return m_plugin;
}

/*!
    \qmlsignal RecommenadationModel::queryFinished(const int &error)

    This handler is called when the request processing is finished.
    0 means that no error occurs during processing and new list is
    available.
*/

/*!
    \qmlproperty QDeclarativeListProperty<QDeclarativeSearchResult> RecommenadationModel::results

    This element holds the list of search results (place or "did you mean" strings)
    that the model currently has.
*/
QDeclarativeListProperty<QDeclarativeSearchResult> QDeclarativeRecommendationModel::results()
{
    return QDeclarativeListProperty<QDeclarativeSearchResult>(this,
                                                          0, // opaque data parameter
                                                          results_append,
                                                          results_count,
                                                          results_at,
                                                          results_clear);
}

void QDeclarativeRecommendationModel::results_append(QDeclarativeListProperty<QDeclarativeSearchResult> *prop,
                                                             QDeclarativeSearchResult* category)
{
    Q_UNUSED(prop);
    Q_UNUSED(category);
}

int QDeclarativeRecommendationModel::results_count(QDeclarativeListProperty<QDeclarativeSearchResult> *prop)
{
    return static_cast<QDeclarativeRecommendationModel*>(prop->object)->m_results.count();
}

QDeclarativeSearchResult* QDeclarativeRecommendationModel::results_at(QDeclarativeListProperty<QDeclarativeSearchResult> *prop,
                                                                          int index)
{
    QDeclarativeRecommendationModel* model = static_cast<QDeclarativeRecommendationModel*>(prop->object);
    QDeclarativeSearchResult *res = NULL;
    if (model->m_results.count() > index && index > -1) {
        res = model->m_results[index];
    }
    return res;
}

void QDeclarativeRecommendationModel::results_clear(QDeclarativeListProperty<QDeclarativeSearchResult> *prop)
{
    Q_UNUSED(prop)
}

void QDeclarativeRecommendationModel::convertResultsToDeclarative()
{
    qDeleteAll(m_results);
    m_results.clear();

    foreach (const QPlaceSearchResult& result, m_response->results()) {
        QDeclarativeSearchResult* declarative = new QDeclarativeSearchResult(result, this);
        m_results.append(declarative);
    }
}

int QDeclarativeRecommendationModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    if (m_response) {
        return m_response->results().count();
    } else {
        return 0;
    }
}

// Returns the stored under the given role for the item referred to by the index.
QVariant QDeclarativeRecommendationModel::data(const QModelIndex& index, int role) const
{
    QDeclarativeSearchResult *result = NULL;
    if (m_response && m_response->results().count() > index.row()) {
       result = m_results[index.row()];
    }

    switch (role) {
        case Qt::DisplayRole:
            if (result && result->type() == QDeclarativeSearchResult::Place) {
                return result->place()->name();
            } else if (result && result->type() == QDeclarativeSearchResult::DidYouMeanSuggestion) {
                return result->didYouMeanSuggestion();
            } else {
                return QString();
            }
        case SearchResultRole:
            if (result) {
                return QVariant::fromValue(result);
            } else {
                return QVariant();
            }
        }
    return QVariant();
}

/*!
    \qmlproperty string RecommenadationModel::placeId

    This element holds place id used in query.
*/

QString QDeclarativeRecommendationModel::placeId() const
{
    return m_queryParameters.searchTerm();
}

void QDeclarativeRecommendationModel::setPlaceId(const QString &placeId)
{
    if (m_queryParameters.searchTerm() == placeId) {
        return;
    }
    m_queryParameters.setSearchTerm(placeId);
    emit placeIdChanged();
}

/*!
    \qmlproperty GeoCoordinate SearchResultModel::searchArea

    This element holds the search area.

    Note: this property's changed() signal is currently emitted only if the
    whole element changes, not if only the contents of the element change.
*/
QDeclarativeGeoBoundingArea *QDeclarativeRecommendationModel::searchArea() const
{
    return m_searchArea;
}

void QDeclarativeRecommendationModel::setSearchArea(QDeclarativeGeoBoundingArea *searchArea)
{
    if (m_searchArea == searchArea)
        return;
    m_searchArea = searchArea;
    emit searchAreaChanged();
}

/*!
    \qmlproperty int RecommenadationModel::offset

    This element holds offset for items that would be returned.
    Less then 0 means that it is undefined.
*/
int QDeclarativeRecommendationModel::offset() const
{
    return m_queryParameters.offset();
}

void QDeclarativeRecommendationModel::setOffset(const int &offsetValue)
{
    if (m_queryParameters.offset() == offsetValue){
        return;
    }
    m_queryParameters.setOffset(offsetValue);
    emit offsetChanged();
}

/*!
    \qmlproperty int RecommenadationModel::limit

    This element holds limit of items that would be returned.
    Less then -1 means that limit is undefined.
*/
int QDeclarativeRecommendationModel::limit() const
{
    return m_queryParameters.limit();
}

void QDeclarativeRecommendationModel::setLimit(const int &limit)
{
    if (m_queryParameters.limit() == limit){
        return;
    }
    m_queryParameters.setLimit(limit);
    emit limitChanged();
}

/*!
    \qmlmethod RecommenadationModel::executeQuery()
    Parameter placeId should contain string for which search should be
    started.
    Updates the items represented by the model from the underlying proivider.
*/
void QDeclarativeRecommendationModel::executeQuery()
{
    if (!m_plugin) {
        qmlInfo(this) << "plugin not set.";
        return;
    }

    QGeoServiceProvider *serviceProvider = m_plugin->sharedGeoServiceProvider();
    if (!serviceProvider)
        return;

    QPlaceManager *placeManager = serviceProvider->placeManager();
    if (!placeManager) {
        qmlInfo(this) << tr("Places not supported by %1 Plugin.").arg(m_plugin->name());
        return;
    }

    cancelPreviousRequest();

    QGeoPlace target;
    target.setPlaceId(m_queryParameters.searchTerm());
    m_queryParameters.setSearchArea(m_searchArea->area());
    connectNewResponse(placeManager->recommendations(target, m_queryParameters));
}

/*!
    \qmlmethod RecommenadationModel::cancelRequest()
    Cancels ongoing request.
*/
void QDeclarativeRecommendationModel::cancelRequest()
{
    cancelPreviousRequest();
}

void QDeclarativeRecommendationModel::replyFinished()
{
    if (m_response && m_response->results().count()) {
        beginResetModel();
        convertResultsToDeclarative();
        endResetModel();
        emit resultsChanged();
    }
    emit queryFinished(0);
}

void QDeclarativeRecommendationModel::replyError(QPlaceReply::Error error,
                                                 const QString &errorString)
{
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    emit queryFinished(-1);
}

void QDeclarativeRecommendationModel::cancelPreviousRequest()
{
    if (m_response) {
        if (!m_response->isFinished()) {
            m_response->abort();
        }
        m_response->deleteLater();
        m_response = NULL;
    }
}

void QDeclarativeRecommendationModel::connectNewResponse(QPlaceSearchReply *newResponse)
{
    if (newResponse) {
        m_response = newResponse;
        m_response->setParent(this);
        connect(m_response, SIGNAL(finished()), this, SLOT(replyFinished()));
        connect(m_response, SIGNAL(error(QPlaceReply::Error,QString)),
                this, SLOT(replyError(QPlaceReply::Error,QString)));
    } else {
        emit queryFinished(-1);
    }
}
