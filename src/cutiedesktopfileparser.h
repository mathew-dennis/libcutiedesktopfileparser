#pragma once

#include <QObject>
#include <QStringList>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QVariantMap>

// ---- Top-level model class ---- 
class DesktopEntryModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ExecRole,
        IconRole,
        GenericRole
    };

    DesktopEntryModel(QObject *parent = nullptr)
        : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setEntries(const QList<QVariantMap> &entries);

private:
    QList<QVariantMap> m_entries;
};

// 2. The Optimized Proxy Model Layer
class AppFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_PROPERTY(QStringList favoriteKeys READ favoriteKeys WRITE setFavoriteKeys NOTIFY favoriteKeysChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)

public:
    explicit AppFilterProxyModel(QObject *parent = nullptr);

    QStringList favoriteKeys() const { return m_favoriteKeys; }
    void setFavoriteKeys(const QStringList &keys);

    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &query);

signals:
    void favoriteKeysChanged();
    void searchQueryChanged();

protected:
    // This internal Qt function optimizes item rejection/acceptance natively
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QStringList m_favoriteKeys;
    QString m_searchQuery;
};

// 3. Main Parser Class
class CutieDesktopFileParser : public QObject {
    Q_OBJECT
public:
    explicit CutieDesktopFileParser(QObject *parent = nullptr);
    ~CutieDesktopFileParser();

    // Returns the clean master model
    Q_INVOKABLE DesktopEntryModel* fetchAllEntriesModel(const QStringList &paths = {}) const;

    static CutieDesktopFileParser* instance();
    static QObject* provider(QQmlEngine *engine, QJSEngine *scriptEngine);
};