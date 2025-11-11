#include "browser.h"
#include "tab.h"

#include <QIcon>
#include <QToolBar>
#include <QAction>
#include <QHBoxLayout>
#include <QTabWidget>
#include <qtoolbutton.h>
#include <QUrl>

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
    tabWidget->setCornerWidget(new QWidget, Qt::TopLeftCorner);
    auto *corner = qobject_cast<QWidget*>(tabWidget->cornerWidget());
    auto *layout = new QHBoxLayout(corner);
    layout->setContentsMargins(0,0,0,0);
    auto *btn = new QToolButton;
    btn->setDefaultAction(newTabAction);
    btn->setFixedSize(30, 30);
    layout->addWidget(btn);

    connect(newTabAction, &QAction::triggered, this, &BrowserWindow::addNewTab);

    addNewTab();
}

void BrowserWindow::addNewTab()
{
    createTab();
}

void BrowserWindow::createTab(const QUrl &url)
{
    auto *tab = new BrowserTab(this);
    int index = tabWidget->addTab(tab, "New tab");
    tabWidget->setCurrentIndex(index);
    if (!url.isEmpty()) {
        tab->setUrl(url);
    }

    connect(tab, &QWidget::windowTitleChanged, this, [this, index](const QString &) {
        tabWidget->setTabText(index, tabWidget->widget(index)->windowTitle());
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
        setWindowTitle(tab->windowTitle() + "Ruka");
    }
}