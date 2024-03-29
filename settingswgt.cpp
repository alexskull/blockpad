#include "settingswgt.h"
#include "ui_settingswgt.h"
#include <QFontDatabase>
#include "global.h"
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QDesktopWidget>

SettingsWgt::SettingsWgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWgt)
{
    ui->setupUi(this);
    QFontDatabase database;
    auto families = database.families();
    ui->comboBoxFonts->addItems(families);
    ui->comboBoxFonts->setCurrentText(qApp->font().family());
    Utilities::setAppFamilyFont(ui->labelSettingsTitle, 16,QFont::Bold);
    setAttribute(Qt::WA_DeleteOnClose);
    //load settings
    {
        if(settings.value("MakePasswordsHidden").type() != QVariant::Invalid)
        {
            auto allways = !settings.value("MakePasswordsHidden").toBool();
            ui->checkBoxPasswVisible->setChecked(allways);
            //ui->tableWidgetAccounts->slotAllwaysChecked(!allways);
        }

        if(settings.value("2FA_On").type() != QVariant::Invalid)
        {
            auto on = settings.value("2FA_On").toBool();
            ui->checkBox2FA_On->setChecked(on);
        }

        if(settings.value("Highlighting_Text").type() != QVariant::Invalid)
        {
            auto on = settings.value("Highlighting_Text").toBool();
            ui->checkBoxHighlightingCode->setChecked(on);
        }
        else
        {
            settings.setValue("Highlighting_Text", true);
            ui->checkBoxHighlightingCode->setChecked(true);
        }

        if(settings.value("FontSize").type() != QVariant::Invalid)
        {
            ui->spinBoxFontSize->setValue(settings.value("FontSize").toInt());
        }

        if(settings.value("LockScreen_Time").type() != QVariant::Invalid)
        {
            auto time = settings.value("LockScreen_Time").toInt();
            ui->spinBoxXMinutesLockScreen->setValue(time);
        }

        if(settings.value("Save_Cache").type() != QVariant::Invalid)
        {
            auto bOn = settings.value("Save_Cache").toBool();
            ui->checkBoxSaveCache->setChecked(bOn);
        }
        else
        {
            settings.setValue("Save_Cache", true);
            ui->checkBoxSaveCache->setChecked(true);
        }

        ui->checkBoxCheckUpdates->setChecked(!settings.value("noUpdating").toBool());
        settings.sync();
    }
//    //add pixmap to label
//    {
//        QPixmap p("://Icons/Block10_Logo.png");
//        int w = ui->labelLogo->width();
//        int h = ui->labelLogo->height();

//        ui->labelLogo->setPixmap(p.scaled(w,h,
//                                          Qt::KeepAspectRatio,
//                                          Qt::SmoothTransformation));
//    }
    ui->labelVersion->setText(defVersionApplication + QString(" (Qt 5.10)"));
#ifdef __linux__
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            QSize(this->size()),
            qApp->desktop()->availableGeometry()
        )
    );
#endif
    //signals-slots connects
    {
        connect(ui->comboBoxFonts, SIGNAL(currentIndexChanged(QString)),
                this, SLOT(slotFontAppChanged(QString)));
        connect(ui->checkBoxPasswVisible, &QCheckBox::toggled,
                this, &SettingsWgt::slotPasswordVisibleClicked);
        connect(ui->checkBox2FA_On, &QCheckBox::toggled,
                this, &SettingsWgt::slot2FA_On_Clicked);
        connect(ui->checkBoxSaveCache, &QCheckBox::toggled,
                this, &SettingsWgt::slotSaveCache_Clicked);
        connect(ui->checkBoxHighlightingCode, &QCheckBox::toggled,
                this, &SettingsWgt::slotCheckHighlightingText);
        connect(ui->checkBoxCheckUpdates, &QCheckBox::toggled,
                this, &SettingsWgt::slotCheckUpdatesStartUp);
        connect(ui->spinBoxXMinutesLockScreen, SIGNAL(valueChanged(int)),
                this, SLOT(slotLockScreen_Time_FinishEditing()));
        connect(ui->spinBoxFontSize, SIGNAL(valueChanged(int)),
                this, SLOT(slotFontSizeFinishEditing()));
    }

}

void SettingsWgt::slotSaveCache_Clicked(bool bOn)
{
    settings.setValue("Save_Cache", bOn);
    settings.sync();
    emit sigSaveCache(bOn);
}

void SettingsWgt::slotCheckHighlightingText(bool bCheck)
{
    settings.setValue("Highlighting_Text", bCheck);
    settings.sync();
    emit sigHighlightingCode(bCheck);
}

void SettingsWgt::slotCheckUpdatesStartUp(bool bCheck)
{
    settings.setValue("noUpdating", !bCheck);
    settings.sync();
}

void SettingsWgt::slotFontAppChanged(QString newFamily)
{
    foreach (QWidget *widget, QApplication::allWidgets())
    {
        QFont font = widget->font();
        font.setFamily(newFamily);
        widget->setFont(font);
    }
    int size = qApp->font().pointSize();
    qApp->setFont (QFont (newFamily, size, appFontWeight));
    settings.setValue("Font", newFamily);
    settings.sync();
}

void SettingsWgt::slotFontSizeFinishEditing()
{
    auto text = ui->spinBoxFontSize->text();
    int size = text.toInt();
    emit sigFontSizeChanged(size);
}

void SettingsWgt::slotPasswordVisibleClicked(bool bCheck)
{
    settings.setValue("MakePasswordsHidden", !bCheck);
    emit sigPasswordVisible(!bCheck);
    //ui->tableWidgetAccounts->slotAllwaysChecked(!bCheck);
}

void SettingsWgt::slot2FA_On_Clicked(bool bCheck)
{
    settings.setValue("2FA_On", bCheck);
    settings.sync();
}


void SettingsWgt::slotLockScreen_Time_FinishEditing()
{
    settings.setValue("LockScreen_Time", ui->spinBoxXMinutesLockScreen->value());
    settings.sync();
    emit sigScreenLock_Time(ui->spinBoxXMinutesLockScreen->value());
}

SettingsWgt::~SettingsWgt()
{
    delete ui;
}
