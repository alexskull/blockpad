#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QDir>
#include <QIcon>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QProcess>
#include <QtConcurrent>

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
#ifdef __APPLE__
    QIcon icon = QIcon(QCoreApplication::applicationDirPath() +
                                      QLatin1String("/../Resources/BlockPad.icns"));
    setWindowIcon(icon);
#endif
    ui->stackedWidget->setCurrentWidget(ui->regist);
    //connect signals/slots
    {
        connect(ui->regist, &Register::successRegister,
                this, &MainWidget::slotSuccessRegistered);
        connect(ui->regist, &Register::successUnlocked,
                this, &MainWidget::slotSuccessUnlocked);
        connect(ui->blockPad, &BlockPad::sigScreenLock_Time,
                this, &MainWidget::sigScreenLock_Time);
    }
    bool b2FA = false;
    if(settings.value("2FA_On").type() != QVariant::Invalid)
    {
        b2FA = settings.value("2FA_On").toBool();
    }
    ui->regist->Init();
#if defined(WIN32) || defined(WIN64)
    int heigth = 310;
    if(b2FA)
        heigth = 380;
#endif
#ifdef __APPLE__
    int heigth = 325;
    if(b2FA)
        heigth = 395;
#endif
    ui->regist->setFixedSize(500, heigth);
    ui->stackedWidget->setFixedSize(500, heigth);
    adjustSize();
#ifdef __APPLE__
    if(settings.value("updateToolsVersion").toString()
            != defVersionApplication
       || !QFile(Utilities::filesDirectory()
            + "/UpdateBlockPad.app").exists())
        QtConcurrent::run(this, &MainWidget::updateUpdateTools);
#endif
}

#ifdef __APPLE__
    void MainWidget::updateUpdateTools()
    {
        //clean
        {
            QDir dir(Utilities::filesDirectory()+ "/UpdateBlockPad.app");
            dir.removeRecursively();
        }
        //remove new version to appPath
        {
            QProcess pros;
            pros.start("sh -c \"cp -R " +Utilities::applicationPath().replace(" ", "\\ ") +"/BlockPad.app/Contents/UpdateTools/UpdateBlockPad.app "
                       + Utilities::filesDirectory().replace(" ", "\\ ")  + "/UpdateBlockPad.app\"");
            pros.waitForFinished(10*60*1000);
        }
        settings.setValue("updateToolsVersion", defVersionApplication);
    }
#endif

void MainWidget::closeEvent(QCloseEvent *event)
{
    bool bClose = true;
    if(ui->blockPad->needSaving()
            &&
       ui->stackedWidget->currentWidget() == ui->blockPad)
    {
        auto button = QMessageBox::warning(this, "BlockPad", "You have unsaved data. Do you want to save them?",
                                 QMessageBox::Ok, QMessageBox::No, QMessageBox::Cancel);

        if(button == QMessageBox::Ok)
            ui->blockPad->slotSaveEncrypt();
        if(button == QMessageBox::Cancel)
        {
            event->ignore();
            bClose = false;
        }
    }

    if(bClose)
    {
        ui->blockPad->closeSeparateWgts();
        event->accept();
        //QWidget::closeEvent(event);
    }
}

void MainWidget::slotLockScreen()
{
    if(ui->stackedWidget->currentWidget() == ui->blockPad)
    {
        ui->stackedWidget->setCurrentWidget(ui->regist);
        ui->blockPad->closeSeparateWgts();
        ui->blockPad->activateWidgets(false);
        ui->regist->onLock();
        showNormal();
        ui->blockPad->setMinimumWidth(0);
        ui->blockPad->setMinimumHeight(0);
        bool b2FA = false;
        if(settings.value("2FA_On").type() != QVariant::Invalid)
        {
            b2FA = settings.value("2FA_On").toBool();
        }
    #if defined(WIN32) || defined(WIN64)
        int heigth = 200;
        if(b2FA)
            heigth = 250;
    #endif
    #ifdef __APPLE__
        int heigth = 225;
        if(b2FA)
            heigth = 265;
    #endif
        ui->regist->setFixedSize(400, heigth);
        ui->stackedWidget->setFixedSize(400, heigth);
        this->setFixedSize(400, heigth);
        adjustSize();
        setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                QSize(400,250),
                qApp->desktop()->availableGeometry()
            )
        );

    }
}

void MainWidget::slotSuccessRegistered()
{
    ui->stackedWidget->setCurrentWidget(ui->blockPad);
    ui->regist->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    ui->stackedWidget->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    ui->regist->setMinimumSize(0,0);
    ui->stackedWidget->setMinimumSize(0,0);
    ui->blockPad->setMinimumWidth(800);
    ui->blockPad->setMinimumHeight(400);
    setMinimumSize(0,0);
    showMaximized();
    ui->blockPad->Init();
    ui->blockPad->activateWidgets(true);
}


void MainWidget::slotSuccessUnlocked()
{
    ui->stackedWidget->setCurrentWidget(ui->blockPad);
    ui->regist->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    ui->stackedWidget->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
    ui->regist->setMinimumSize(0,0);
    ui->stackedWidget->setMinimumSize(0,0);
    ui->blockPad->setMinimumWidth(800);
    ui->blockPad->setMinimumHeight(400);
    ui->blockPad->activateWidgets(true);
    setMinimumSize(0,0);
    showMaximized();
}

MainWidget::~MainWidget()
{
    delete ui;
}
