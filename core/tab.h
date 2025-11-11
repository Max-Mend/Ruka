#ifndef WEB_BROWSER_TAB_H
#define WEB_BROWSER_TAB_H

#include <qmetaobject.h>
#include <QWidget>
#include <QWebEngineView>

class QLineEdit;
class QPushButton;

class BrowserTab : public QWidget
{
    Q_OBJECT
public:
    explicit BrowserTab(QWidget *parent = nullptr);
    void setUrl(const QUrl &url);
    void resize(int w, int h) { QWidget::resize(w, h); }
    void show() { QWidget::show(); }

private slots:
    void go();
    void updateUrl(const QUrl &url);
    void updateButtons();

private:
    QWebEngineView *webView;
    QLineEdit      *addressBar;
    QPushButton    *backBtn;
    QPushButton    *forwardBtn;
    QPushButton    *refreshBtn;
    QPushButton    *goBtn;
    QMetaMethod *titleChanged;
    QMetaMethod *onTitleChanged;
    QMetaMethod *onUrlChanged;

    void searchOrNavigate(const QString &text);
};

#endif