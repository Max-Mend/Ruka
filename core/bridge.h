#ifndef WEB_BROUSER_BRIDGE_H
#define WEB_BROUSER_BRIDGE_H

#include <QObject>

class Bridge : public QObject
{
    Q_OBJECT

public:
    explicit Bridge(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE void search(const QString &query) {
        emit searchRequested(query);
    }

    signals:
        void searchRequested(const QString &query);
};

#endif