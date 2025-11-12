#include <QApplication>
#include <QtWebEngineWidgets/QtWebEngineWidgets>
#include <QUrl>

#include "core/browser.h"

int main(int argc, char *argv[])
{
    qputenv("FONTCONFIG_PATH", "/etc/fonts");
    qputenv("FONTCONFIG_FILE", "/etc/fonts/fonts.conf");

    QApplication app(argc, argv);

    BrowserWindow browser;
    browser.resize(1200, 800);
    browser.show();

    return app.exec();
}