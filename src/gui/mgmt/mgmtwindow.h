#ifndef MGMTWINDOW_H
#define MGMTWINDOW_H

#include <QScrollArea>

#include <DMainWindow>
#include <DWidget>
#include <DStandardItem>
#include <DListView>

DWIDGET_USE_NAMESPACE

class ModelListWidget;
class LocalModelListWidget;
class WelcomeDialog;
class AddModelDialog;

class MgmtWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MgmtWindow(DWidget *parent = nullptr);
    ~MgmtWindow();

    void showEx(bool);

private slots:
    void onAddModel();
    void onThemeTypeChanged();

private:
    void initUI();
    void initConnect();

    bool showAddModelDialog(bool isWelcomeAdd = false);

    ModelListWidget *initModelListWidget();
    LocalModelListWidget *initLocalModelListWidget();
    DWidget *initAgreementWidget();
    DWidget *initProxyWidget();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

signals:
    void signalCloseWindow();

private:
    QWidget *m_pToastContent = nullptr;
    QScrollArea *m_pScrollArea = nullptr;

    ModelListWidget *m_pModelListWidget = nullptr;
    LocalModelListWidget *m_pLocalModelListWidget = nullptr;

    QSet<QWidget *> m_widgets;
    WelcomeDialog *m_pWelcomeDlg = nullptr;
    AddModelDialog *m_pAddDlg = nullptr;
    bool m_bIsWelcomeAdd = false;
};

#endif // MGMTWINDOW_H
