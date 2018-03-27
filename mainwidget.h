#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QSettings>
#include "blockpad.h"
#include "register.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
protected:
    void closeEvent(QCloseEvent *event);
private:
    Ui::MainWidget *ui;
    QSettings settings;
#ifdef __APPLE__
    void updateUpdateTools();
#endif
public slots:
    void slotLockScreen();
private slots:
    void slotSuccessRegistered();
    void slotSuccessUnlocked();
signals:
    void sigScreenLock_Time(int time);
};

#endif // MAINWIDGET_H
