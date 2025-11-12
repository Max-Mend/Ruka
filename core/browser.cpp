#include <QIcon>
#include <QToolBar>
#include <QAction>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QToolButton>
#include <QUrl>
#include <QShortcut>
#include <QKeySequence>

#include "browser.h"
#include "tab.h"

BrowserWindow::BrowserWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Ruka");
    setWindowIcon(QIcon::fromTheme("web-browser"));

    tabWidget = new QTabWidget(this);
    tabWidget->setTabsClosable(true);
    tabWidget->setMovable(true);
    setCentralWidget(tabWidget);

    connect(tabWidget, &QTabWidget::tabCloseRequested, this, &BrowserWindow::closeTab);
    connect(tabWidget, &QTabWidget::currentChanged, this, &BrowserWindow::updateTabTitle);

    auto *newTabAction = new QAction("+", this);
    newTabAction->setToolTip("New tab");
    tabWidget->setCornerWidget(new QWidget(this), Qt::TopLeftCorner);
    auto *corner = qobject_cast<QWidget*>(tabWidget->cornerWidget());
    auto *layout = new QHBoxLayout(corner);
    layout->setContentsMargins(0, 0, 0, 0);
    auto *btn = new QToolButton(corner);
    btn->setDefaultAction(newTabAction);
    btn->setFixedSize(30, 30);
    layout->addWidget(btn);

    connect(newTabAction, &QAction::triggered, this, &BrowserWindow::addNewTab);

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