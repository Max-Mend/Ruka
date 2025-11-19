#include "tab.h"
#include "search.h"
#include "bridge.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QPushButton>
#include <QWebEngineHistory>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEnginePage>
#include <QWebChannel>
#include <QRegularExpression>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <QFileDialog>
#include <QMessageBox>

BrowserTab::BrowserTab(QWidget *parent) : QWidget(parent)
{
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setHttpUserAgent(
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
        "(KHTML, like Gecko) Chrome/128.0 Safari/537.36"
    );

    QWebEngineSettings *settings = profile->settings();
    settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);

    webView = new QWebEngineView(this);
    webView->setPage(new QWebEnginePage(profile, webView));

    QWebChannel *channel = new QWebChannel(this);
    webView->page()->setWebChannel(channel);

    QString bridgeName = QStringLiteral("bridge");
    Bridge *bridge = new Bridge(this);
    channel->registerObject(QStringLiteral("Qt"), bridge);

    connect(bridge, &Bridge::searchRequested, this, &BrowserTab::jsSearch);

    backBtn    = new QPushButton(this);
    forwardBtn = new QPushButton(this);
    refreshBtn = new QPushButton(this);
    menuBtn    = new QPushButton(this);
    addressBar = new QLineEdit(this);

    backBtn->setIcon(QIcon::fromTheme("go-previous"));
    forwardBtn->setIcon(QIcon::fromTheme("go-next"));
    refreshBtn->setIcon(QIcon::fromTheme("view-refresh"));
    menuBtn->setIcon(QIcon::fromTheme("application-menu"));

    backBtn->setToolTip("Back");
    forwardBtn->setToolTip("Forward");
    refreshBtn->setToolTip("Update");
    menuBtn->setToolTip("Menu");
    addressBar->setPlaceholderText("Search...");

    backBtn->setFlat(true);
    forwardBtn->setFlat(true);
    refreshBtn->setFlat(true);
    menuBtn->setFlat(true);

    auto *nav = new QHBoxLayout();
    nav->addWidget(backBtn);
    nav->addWidget(forwardBtn);
    nav->addWidget(refreshBtn);
    nav->addWidget(addressBar);
    nav->addWidget(menuBtn);
    nav->setContentsMargins(5, 5, 5, 5);
    nav->setSpacing(8);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(nav);
    mainLayout->addWidget(webView);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    connect(backBtn,    &QPushButton::clicked, webView, &QWebEngineView::back);
    connect(forwardBtn, &QPushButton::clicked, webView, &QWebEngineView::forward);
    connect(refreshBtn, &QPushButton::clicked, webView, &QWebEngineView::reload);
    connect(menuBtn,    &QPushButton::clicked, this, &BrowserTab::showMenu);  // Changed
    connect(addressBar, &QLineEdit::returnPressed, this, &BrowserTab::navigateToAddress);

    connect(webView, &QWebEngineView::urlChanged, this, &BrowserTab::updateUrl);
    connect(webView, &QWebEngineView::loadFinished, this, &BrowserTab::updateButtons);

    updateButtons();
}

void BrowserTab::setUrl(const QUrl &url)
{
    webView->setUrl(url);
}

void BrowserTab::searchOrNavigate(const QString &text)
{
    QString input = text.trimmed();
    if (input.isEmpty()) return;

    bool isUrl = false;
    QUrl url;

    if (input.startsWith("http://") || input.startsWith("https://") || input.startsWith("file://")) {
        isUrl = true;
        url = QUrl(input);
    }
    else if (input.startsWith("localhost") || input.startsWith("127.0.0.1") ||
             QRegularExpression("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}").match(input).hasMatch()) {
        isUrl = true;
        url = QUrl::fromUserInput(input);
    }
    else if (input.contains('.') && !input.contains(' ')) {
        QStringList parts = input.split('.');
        if (parts.size() >= 2 && parts.last().length() >= 2 && parts.last().length() <= 6) {
            bool validDomain = true;
            QRegularExpression domainRegex("^[a-zA-Z0-9-]+$");
            for (const QString &part : parts) {
                if (!domainRegex.match(part).hasMatch()) {
                    validDomain = false;
                    break;
                }
            }
            if (validDomain) {
                isUrl = true;
                url = QUrl::fromUserInput("https://" + input);
            }
        }
    }

    if (isUrl && url.isValid()) {
        webView->load(url);
    } else {
        std::string query = input.toUtf8().toStdString();
        std::string searchUrl = search_url(query);
        webView->load(QUrl(QString::fromStdString(searchUrl)));
    }
}

void BrowserTab::navigateToAddress()
{
    searchOrNavigate(addressBar->text());
}

void BrowserTab::showMenu()
{
    QMenu *menu = new QMenu(this);

    QAction *zoomInAction = menu->addAction(QIcon::fromTheme("zoom-in"), "Zoom In");
    connect(zoomInAction, &QAction::triggered, this, [this]() {
        webView->setZoomFactor(webView->zoomFactor() + 0.1);
    });

    QAction *zoomOutAction = menu->addAction(QIcon::fromTheme("zoom-out"), "Zoom Out");
    connect(zoomOutAction, &QAction::triggered, this, [this]() {
        webView->setZoomFactor(webView->zoomFactor() - 0.1);
    });

    QAction *zoomResetAction = menu->addAction(QIcon::fromTheme("zoom-original"), "Reset Zoom");
    connect(zoomResetAction, &QAction::triggered, this, [this]() {
        webView->setZoomFactor(1.0);
    });

    menu->addSeparator();

    QAction *copyUrlAction = menu->addAction(QIcon::fromTheme("edit-copy"), "Copy URL");
    connect(copyUrlAction, &QAction::triggered, this, [this]() {
        QGuiApplication::clipboard()->setText(webView->url().toString());
    });

    QAction *savePageAction = menu->addAction(QIcon::fromTheme("document-save"), "Save Page As...");
    connect(savePageAction, &QAction::triggered, this, &BrowserTab::savePage);

    QAction *printAction = menu->addAction(QIcon::fromTheme("document-print"), "Print...");
    connect(printAction, &QAction::triggered, this, [this]() {
        webView->page()->printToPdf("page.pdf");
    });

    menu->addSeparator();

    QAction *viewSourceAction = menu->addAction(QIcon::fromTheme("text-html"), "View Page Source");
    connect(viewSourceAction, &QAction::triggered, this, [this]() {
        QUrl sourceUrl = webView->url();
        sourceUrl.setScheme("view-source");
        webView->load(sourceUrl);
    });

    menu->addSeparator();

    QAction *clearHistoryAction = menu->addAction(QIcon::fromTheme("edit-clear"), "Clear History");
    connect(clearHistoryAction, &QAction::triggered, this, [this]() {
        webView->history()->clear();
    });

    QAction *clearCacheAction = menu->addAction(QIcon::fromTheme("edit-clear-all"), "Clear Cache");
    connect(clearCacheAction, &QAction::triggered, this, []() {
        QWebEngineProfile::defaultProfile()->clearHttpCache();
    });

    menu->exec(menuBtn->mapToGlobal(QPoint(0, menuBtn->height())));
    menu->deleteLater();
}

void BrowserTab::savePage()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Save Page",
        QDir::homePath() + "/page.html",
        "HTML Files (*.html *.htm);;All Files (*)");

    if (!fileName.isEmpty()) {
        webView->page()->save(fileName, QWebEngineDownloadRequest::CompleteHtmlSaveFormat);
    }
}

void BrowserTab::updateUrl(const QUrl &url)
{
    if (url.scheme() == "file" || url.scheme() == "qrc") {
        addressBar->setText("");
    } else {
        addressBar->setText(url.toString());
    }
}

void BrowserTab::updateButtons()
{
    backBtn->setEnabled(webView->history()->canGoBack());
    forwardBtn->setEnabled(webView->history()->canGoForward());
}

void BrowserTab::jsSearch(const QString &query)
{
    searchOrNavigate(query);
}

void BrowserTab::setAddressBarText(const QString &text)
{
    addressBar->setText(text);
}