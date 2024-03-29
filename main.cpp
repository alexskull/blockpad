#include "blockpad.h"
#include <QApplication>
#include <QFontDatabase>
#include <QDir>
#include "global.h"
#include "mainwidget.h"
#include "eventswaiting.h"
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QDebug>
#include "find/globalsearch.h"
#include <QProcess>

QString fileInit;


int main(int argc, char *argv[])
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime().addSecs(24)));
    QApplication a(argc, argv);
    qRegisterMetaType<findResults>("findResults");
    QCoreApplication::setOrganizationName("Bloc10");
    QCoreApplication::setOrganizationDomain("www.bloc10.com");
    QCoreApplication::setApplicationName("BlockPad");
#ifdef __linux__
    a.setWindowIcon(QIcon("://Icons/BlocPad.png"));
#endif
    if(argc > 1)
        fileInit = QString(argv[1]);
    //create application files directories
    {
        QDir().mkpath(Utilities::filesDirectory());
        //create defPathBlockpads
        if(!QDir(Utilities::filesDirectory()
                 + "/"+ defPathBlockpads).exists())
            QDir(Utilities::filesDirectory()).mkdir(defPathBlockpads);
    }
    //fonts
    {
        QDir dir("://Fonts");
        foreach(auto file, dir.entryList())
        {
            QFontDatabase::addApplicationFont("://Fonts/"+ file);
        }
        QString font;
        int fontSize = 0;
        //fill font and fontSize
        {
            QSettings settings;
            font = settings.value("Font").toString();
            if(font == "")
                font = "Roboto";
            if(fontSize == 0)
            {
            #if defined(WIN32) || defined(WIN64)
                fontSize = 13;
            #endif
            #ifdef __APPLE__
                fontSize = 15;
            #endif
            }
        }
        a.setFont (QFont (font, fontSize, appFontWeight));
    }
    EventsWaiting evWait;
    a.installEventFilter(&evWait);
    MainWidget wgt;
    wgt.show();

    //connect signals/slots
    {
        a.connect(&evWait, &EventsWaiting::StartLockScreen,
                  &wgt, &MainWidget::slotLockScreen);
        a.connect(&wgt, &MainWidget::sigScreenLock_Time,
                  &evWait, &EventsWaiting::slotSetTimeLockScreen);
    }   
    return a.exec();
}
