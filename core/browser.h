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

private slots:
    void addNewTab();
    void closeTab(int index);

    void updateTabTitle();

private:
    QTabWidget *tabWidget;
    void createTab(const QUrl &url = QUrl("https://www.google.com"));
};

#endif