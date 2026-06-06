#include "cutiedesktopfileparser.h"
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>
#include <QQmlEngine>

// ------------------- DesktopEntryModel -------------------
int DesktopEntryModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_entries.size();
}

QVariant DesktopEntryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_entries.size())
        return QVariant();

    const QVariantMap &entry = m_entries.at(index.row());
    switch (role) {
    case NameRole: return entry.value("Desktop Entry/Name");
    case ExecRole: return entry.value("Desktop Entry/Exec");
    case IconRole: return entry.value("Desktop Entry/Icon");
    case GenericRole: return entry;
    default: return QVariant();
    }
}

QHash<int, QByteArray> DesktopEntryModel::roleNames() const {
    return {
        {NameRole, "name"},
        {ExecRole, "exec"},
        {IconRole, "icon"},
        {GenericRole, "entry"}
    };
}

void DesktopEntryModel::setEntries(const QList<QVariantMap> &entries) {
    beginResetModel();
    m_entries = entries;
    endResetModel();
}

// ------------------- AppFilterProxyModel -------------------
AppFilterProxyModel::AppFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    // Enable sorting support if you decide to sort alphabetically later
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

void AppFilterProxyModel::setFavoriteKeys(const QStringList &keys) {
    if (m_favoriteKeys != keys) {
        m_favoriteKeys = keys;
        emit favoriteKeysChanged();
        invalidateFilter(); // Tells Qt to instantly re-run filtering rules
    }
}

void AppFilterProxyModel::setSearchQuery(const QString &query) {
    if (m_searchQuery != query) {
        m_searchQuery = query;
        emit searchQueryChanged();
        invalidateFilter(); // Tells Qt to instantly re-run filtering rules
    }
}

bool AppFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    // 1. Get access to the item data using the exact Source Model roles
    QModelIndex sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    QString appName = sourceModel()->data(sourceIndex, DesktopEntryModel::NameRole).toString();

    // 2. Apply Favorites rule if the list isn't empty
    if (!m_favoriteKeys.isEmpty() && !m_favoriteKeys.contains(appName)) {
        return false;
    }

    // 3. Apply Case-Insensitive String Filter matching rule
    const QString cleanQuery = m_searchQuery.trimmed();
    if (!cleanQuery.isEmpty() && !appName.contains(cleanQuery, Qt::CaseInsensitive))
        return false;

    return true;
}

// ------------------- CutieDesktopFileParser -------------------
CutieDesktopFileParser::CutieDesktopFileParser(QObject *parent)
    : QObject(parent)
{
    qDebug() << "module - CutieDesktopFileParser : loaded.";
}

CutieDesktopFileParser::~CutieDesktopFileParser() {}

// Returns a new DesktopEntryModel populated with entries from the given paths
DesktopEntryModel* CutieDesktopFileParser::fetchAllEntriesModel(const QStringList &paths) const {
    auto *model = new DesktopEntryModel();

    QList<QVariantMap> entries;

    qDebug() << "module - CutieDesktopFileParser - fetchAllEntriesModel() : called";

    QStringList dataDirList = paths.isEmpty()
        ? QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)
        : paths;
    qDebug() << "module - CutieDesktopFileParser - fetchAllEntriesModel() : Application directories =" << dataDirList;

    for (const QString &directory : dataDirList) {
        QDir dir(directory);
        if (!dir.exists()) continue;

        QFileInfoList files = dir.entryInfoList(QStringList{"*.desktop"}, QDir::Files);
        for (const QFileInfo &fileInfo : files) {
            QSettings desktopFile(fileInfo.absoluteFilePath(), QSettings::IniFormat);
            QString type = desktopFile.value("Desktop Entry/Type").toString();
            if (type != "Application") continue;

            QString hidden = desktopFile.value("Desktop Entry/Hidden").toString();
            QString noDisplay = desktopFile.value("Desktop Entry/NoDisplay").toString();
            if (hidden == "true" || noDisplay == "true") continue;

            QVariantMap appData;
            for (const QString &key : desktopFile.allKeys())
                appData.insert(key, desktopFile.value(key));

            entries.append(appData);
        }
    }

    qDebug() << "module - CutieDesktopFileParser - fetchAllEntriesModel() : number of entries found =" << entries.size();
    model->setEntries(entries);
    return model;
}

AppFilterProxyModel* CutieDesktopFileParser::createFilterModel(const QStringList &paths) const
{
    auto *base = fetchAllEntriesModel(paths);
    auto *proxy = new AppFilterProxyModel();

    proxy->setSourceModel(base);

    base->setParent(proxy);

    return proxy;
}

// Singleton instance
CutieDesktopFileParser* CutieDesktopFileParser::instance() {
    static CutieDesktopFileParser instance;
    return &instance;
}

// QQmlEngine provider
QObject* CutieDesktopFileParser::provider(QQmlEngine *engine, QJSEngine *scriptEngine) {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return CutieDesktopFileParser::instance();
}