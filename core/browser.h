#ifndef BROWSER_H
#define BROWSER_H

#include <QMainWindow>
#include <QTabWidget>
#include <QUrl>

class BrowserTab;

class BrowserWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit BrowserWindow(QWidget *parent = nullptr);

public slots:
    void addNewTab();
    void createNewTabWithUrl(const QUrl &url);

private slots:
    void closeTab(int index);
    void updateTabTitle();

private:
    QTabWidget *tabWidget;
    void createTab(const QUrl &url = QUrl());
};

#endif