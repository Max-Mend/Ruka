#include "tab.h"
#include "search.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QPushButton>
#include <QWebEngineHistory>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEnginePage>

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

    backBtn    = new QPushButton(this);
    forwardBtn = new QPushButton(this);
    refreshBtn = new QPushButton(this);
    goBtn      = new QPushButton(this);
    addressBar = new QLineEdit(this);

    backBtn->setIcon(QIcon::fromTheme("go-previous"));
    forwardBtn->setIcon(QIcon::fromTheme("go-next"));
    refreshBtn->setIcon(QIcon::fromTheme("view-refresh"));
    goBtn->setIcon(QIcon::fromTheme("go-jump"));

    backBtn->setToolTip("Назад");
    forwardBtn->setToolTip("Вперед");
    refreshBtn->setToolTip("Оновити");
    goBtn->setToolTip("Перейти");
    addressBar->setPlaceholderText("Пошук Google або URL...");

    backBtn->setFlat(true);
    forwardBtn->setFlat(true);
    refreshBtn->setFlat(true);
    goBtn->setFlat(true);

    auto *nav = new QHBoxLayout();
    nav->addWidget(backBtn);
    nav->addWidget(forwardBtn);
    nav->addWidget(refreshBtn);
    nav->addWidget(addressBar);
    nav->addWidget(goBtn);
    nav->setContentsMargins(5, 5, 5, 5);
    nav->setSpacing(8);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(nav);
    mainLayout->addWidget(webView);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    webView->load(QUrl("https://www.google.com"));

    connect(backBtn,    &QPushButton::clicked, webView, &QWebEngineView::back);
    connect(forwardBtn, &QPushButton::clicked, webView, &QWebEngineView::forward);
    connect(refreshBtn, &QPushButton::clicked, webView, &QWebEngineView::reload);
    connect(goBtn,      &QPushButton::clicked, this, &BrowserTab::go);
    connect(addressBar, &QLineEdit::returnPressed, this, &BrowserTab::go);

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

    QUrl url = QUrl::fromUserInput(input);
    if (url.isValid() && !url.host().isEmpty()) {
        webView->load(url);
    } else {
        std::string query = input.toUtf8().toStdString();
        std::string searchUrl = search_url(query);
        webView->load(QUrl(QString::fromStdString(searchUrl)));
    }
}

void BrowserTab::go() { searchOrNavigate(addressBar->text()); }
void BrowserTab::updateUrl(const QUrl &url) { addressBar->setText(url.toString()); }
void BrowserTab::updateButtons()
{
    backBtn->setEnabled(webView->history()->canGoBack());
    forwardBtn->setEnabled(webView->history()->canGoForward());
}