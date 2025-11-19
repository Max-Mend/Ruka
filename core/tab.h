#ifndef TAB_H
#define TAB_H

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
    void setAddressBarText(const QString &text);
    QWebEngineView* getWebView() const { return webView; }

private slots:
    void navigateToAddress();
    void showMenu();
    void updateUrl(const QUrl &url);
    void updateButtons();
    void jsSearch(const QString &query);
    void savePage();

private:
    void searchOrNavigate(const QString &text);

    QWebEngineView *webView;
    QPushButton *backBtn;
    QPushButton *forwardBtn;
    QPushButton *refreshBtn;
    QPushButton *menuBtn;
    QLineEdit *addressBar;
};

#endif // TAB_H