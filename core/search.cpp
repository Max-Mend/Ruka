#include "search.h"
#include <QUrl>

std::string search_url(const std::string& query)
{
    if (query.empty()) {
        return "https://www.google.com";
    }

    QString encoded = QUrl::toPercentEncoding(QString::fromUtf8(query.c_str()));
    return "https://www.google.com/search?q=" + encoded.toStdString();
}