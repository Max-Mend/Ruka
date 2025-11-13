#include "browser.h"
#include "tab.h"

#include <QIcon>
#include <QToolBar>
#include <QAction>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTabBar>
#include <QToolButton>
#include <QUrl>
#include <QShortcut>
#include <QKeySequence>

BrowserWindow::BrowserWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Ruka");
    setWindowIcon(QIcon::fromTheme("web-browser"));

    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    tabWidget->setDocumentMode(true);
    tabWidget->setElideMode(Qt::ElideRight);

    // Вкладки зліва
    tabWidget->setTabPosition(QTabWidget::North);
    tabWidget->tabBar()->setExpanding(false);

    setCentralWidget(tabWidget);

    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &BrowserWindow::closeTab);
    connect(tabWidget, &QTabWidget::currentChanged, this, &BrowserWindow::updateTabTitle);

    // Кнопка для нової вкладки
    auto *newTabBtn = new QToolButton(this);
    newTabBtn->setText("+");
    newTabBtn->setToolTip("Нова вкладка (Ctrl+T)");
    newTabBtn->setAutoRaise(true);
    newTabBtn->setFixedSize(30, 30);

    // Встановлюємо кнопку справа від вкладок
    tabWidget->setCornerWidget(newTabBtn, Qt::TopLeftCorner);

    connect(newTabBtn, &QToolButton::clicked, this, &BrowserWindow::addNewTab);

    // Гарячі клавіші
    auto *newTabShortcut = new QShortcut(QKeySequence::AddTab, this);
    connect(newTabShortcut, &QShortcut::activated, this, &BrowserWindow::addNewTab);

    auto *closeTabShortcut = new QShortcut(QKeySequence::Close, this);
    connect(closeTabShortcut, &QShortcut::activated, this, [this]() {
        if (tabWidget->currentIndex() >= 0) {
            closeTab(tabWidget->currentIndex());
        }
    });

    addNewTab();
}

void BrowserWindow::addNewTab()
{
    createTab();
}

void BrowserWindow::createNewTabWithUrl(const QUrl &url)
{
    createTab(url);
}

void BrowserWindow::createTab(const QUrl &url)
{
    auto *tab = new BrowserTab(this);
    int index = tabWidget->addTab(tab, "Home");
    tabWidget->setCurrentIndex(index);

    // Якщо URL не вказано, завантажуємо домашню сторінку
    if (url.isEmpty()) {
        // Спробуємо завантажити з qrc ресурсів, якщо не вийде - з файлової системи
        QUrl startUrl("qrc:/ui/html/index.html");
        if (!QFile::exists(":/ui/html/index.html")) {
            startUrl = QUrl::fromLocalFile(QDir::current().filePath("ui/html/index.html"));
        }
        tab->getWebView()->load(startUrl);

        // Очищаємо адресну стрічку для домашньої сторінки
        tab->setAddressBarText("");
    } else {
        tab->getWebView()->load(url);
    }

    // Підключаємось до сигналу titleChanged від QWebEngineView
    connect(tab->getWebView(), &QWebEngineView::titleChanged, this, [this, tab](const QString &title) {
        int idx = tabWidget->indexOf(tab);
        if (idx != -1) {
            // Для домашньої сторінки завжди показуємо "Home"
            QString tabTitle = (title == "Main" || title.isEmpty()) ? "Home" : title;
            tabWidget->setTabText(idx, tabTitle);
        }
    });
}

void BrowserWindow::closeTab(int index)
{
    if (tabWidget->count() == 1) {
        close();
        return;
    }
    QWidget *w = tabWidget->widget(index);
    tabWidget->removeTab(index);
    delete w;
}

void BrowserWindow::updateTabTitle()
{
    if (auto *tab = qobject_cast<BrowserTab*>(tabWidget->currentWidget())) {
        QString title = tab->getWebView()->title();
        setWindowTitle((title.isEmpty() ? "Ruka" : title + " - Ruka"));
    }
}