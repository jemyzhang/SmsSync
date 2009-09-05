#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QTextCodec>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QSqlError>
#include <QMessageBox>
#include <QTextCodec>

#include <QProcess>
#include "LocalDataBase.h"
#include "ui_password.h"

wchar_t g_password[256];
int g_password_len;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    ui->progressBar->setVisible(false);
//    sqlite_checkpwd();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::sqlite_checkpwd(){
    LocalDataBase ldb;
    bool nRet = false;
    wchar_t defaultpwd[6] = {0x10,0x15,0x13,0x18,0x08,0x01};
    g_password_len = 6;
    memcpy(g_password,defaultpwd,sizeof(wchar_t)*6);

    if(!ldb.checkpwd(NULL,0)){
        if(!ldb.checkpwd(defaultpwd,6)){
            Ui_PasswordWnd w;
            int rc = w.exec();
            if(rc == QDialog::Accepted){
                nRet = true;
            }
        }else{
            nRet = true;
        }
    }else{
        ldb.connect();
        ldb.encrypt(defaultpwd,6);
        ldb.disconnect();
        nRet = true;
    }
    return nRet;
}

int MainWindow::sqlite_export(){

    LocalDataBase ldb;
    uint nSuccess = 0;
    bool old_version;
    if(ldb.checkpwd(g_password,g_password_len)){
        old_version = false;
        if(ldb.oldTableExists()){
            QMessageBox::information(this,tr("Attention"),tr("The db verion is too old, please update the m8smsbackup to the latest version"));
            old_version = true;
        }
        UINT nSent,nReceived;
        if(old_version){
            ldb.GetSmsCount_v1(nReceived,nSent);
        }else{
            ldb.GetSmsCount(nReceived,nSent);
        }
        uint nSize = nSent + nReceived;
        if(nSize > 0){
            ui->progressBar->setRange(0,nSize);

            ldb.CreateTempSmsTable();

            if(!QFileInfo("SMS.mdb").exists()){//create db
                QFile file(":/mdb/SMS.mdb");
                bool nret = file.open(QIODevice::ReadOnly);
                QByteArray b = file.readAll();
                file.close();
                QFile fileout("SMS.mdb");
                fileout.open(QIODevice::ReadWrite);
                fileout.write(b);
                fileout.close();
            }
            QSqlDatabase ldbodbc = QSqlDatabase::addDatabase("QODBC","sms");
            ldbodbc.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=SMS.mdb;UID=;PWD=1a2b3c4d5c6d7e8f");
            bool okodbc = ldbodbc.open();
            if(okodbc){
                uint ncount = 0;
                ui->progressBar->setVisible(true);
                QString s;
                QSqlQuery mquery(ldbodbc);
                //mquery.exec("BEGIN TRANSACTION");
                while(ncount<nSize){
                    ui->progressBar->setValue(ncount+1);
                    ui->progressBar->update();
                    SmsSimpleData_t smsData;
                    if(old_version){
                        ldb.GetSms_v1(ncount++,&smsData);
                    }else{
                        ldb.GetSms(ncount++,&smsData);
                    }
                    QString sqlcmdCheckdup = "select count(*) from SMS where ";
                    QString sqlcmdaction = "insert into SMS (";
                    QString sqlcmdvalue = "values(";
                    QString name = QString::fromStdWString(smsData.ContactName);
                    bool bnext = false;
                    if(!name.isNull() && !name.isEmpty()){
                        sqlcmdaction += "名字,";
                        sqlcmdvalue = sqlcmdvalue + "'" + name + "',";
                        sqlcmdCheckdup += "名字=";
                        sqlcmdCheckdup = sqlcmdCheckdup + "'" + name + "'";
                        bnext = true;
                    }
                    QString phonenumber = QString::fromStdWString(smsData.MobileNumber);
                    if(!phonenumber.isNull() && !phonenumber.isEmpty()){
                        sqlcmdaction += "号码,";
                        sqlcmdvalue = sqlcmdvalue + "'" + phonenumber + "',";
                    }
                    QString content = QString::fromStdWString(smsData.Content);
                    if(!content.isNull() && !content.isEmpty()){
                        sqlcmdaction += "内容,";
                        sqlcmdvalue = sqlcmdvalue + "'" + content + "',";
                    }
                    QString timestamps = QString::fromStdWString(smsData.TimeStamp);
                    if(!timestamps.isNull() && !timestamps.isEmpty()){
                        sqlcmdaction += "时间,";
                        sqlcmdvalue = sqlcmdvalue + "'" + timestamps + "',";
                        if(bnext) sqlcmdCheckdup += " and ";
                        sqlcmdCheckdup += "format(时间,'yyyy-mm-dd hh:nn:ss')=";
                        sqlcmdCheckdup = sqlcmdCheckdup + "'" + timestamps + "'";
                    }
                    bool sendreceive = smsData.SendReceiveFlag;
                    sqlcmdaction += "类型";
                    sqlcmdvalue = sqlcmdvalue + "'" + (sendreceive ? "发" : "收") + "'";
                    if(bnext) sqlcmdCheckdup += " and ";
                    sqlcmdCheckdup += "类型=";
                    sqlcmdCheckdup = sqlcmdCheckdup + "'" + (sendreceive ? "发" : "收") + "'";

                    bool isok = mquery.exec(sqlcmdCheckdup);
                    int nDup = 0;
                    if(isok){
                        mquery.next();
                        nDup = mquery.value(0).toInt();
                    }
                    //mquery.clear();
                    if(nDup == 0){
                        isok=mquery.exec(sqlcmdaction + ") " + sqlcmdvalue + ") ");
                        if(isok){
                            nSuccess++;
                        }
                    }
                    //label显示
                    ui->labelResult->setText(
                            tr("Importing Records:").append(
                                    QString::number(ncount)).append("/").append(
                                            QString::number(nSize).append(" Imported:").append(
                                                    QString::number(nSuccess))));
                    //清除结果集
                    //mquery.clear();
                    sqlcmdaction.clear();
                    sqlcmdvalue.clear();
                    sqlcmdCheckdup.clear();
                    smsData.Reset();
                    QApplication::processEvents();
                }
                //mquery.exec("COMMIT");
                ldbodbc.close();
            }else{
                ui->labelResult->setText(ldbodbc.lastError().text());//tr("Could not open SMS.mdb"));
            }
        }else{
            ui->labelResult->setText(tr("No sms records found"));
        }
        ldb.disconnect();
        QSqlDatabase::removeDatabase("sms");
    }else{
        ui->labelResult->setText(tr("Could not found or open sms.db"));
    }
    ui->progressBar->setVisible(false);
    return 0;
}

void MainWindow::on_btnImport_clicked()
{
    ui->btnImport->setEnabled(false);
    ui->btnView->setEnabled(false);
    ui->labelResult->setText(tr("Connecting to Database..."));
    if(!sqlite_checkpwd()){
        ui->btnImport->setEnabled(true);
        ui->labelResult->setText(tr("Password invalid, abort..."));
        ui->btnImport->setEnabled(true);
        ui->btnView->setEnabled(true);
        return;
    }
    sqlite_export();
    ui->btnImport->setEnabled(true);
    ui->btnView->setEnabled(true);
}

void MainWindow::on_btnView_clicked()
{
    ui->btnImport->setEnabled(false);
    ui->btnView->setEnabled(false);
    if(!QFileInfo("smsviewer.exe").exists()){//create db
        QFile file(":/mdb/LookSMS.exe");
        bool nret = file.open(QIODevice::ReadOnly);
        QByteArray b = file.readAll();
        file.close();
        QFile fileout("smsviewer.exe");
        fileout.open(QIODevice::ReadWrite);
        fileout.write(b);
        fileout.close();
    }
    QProcess *process = new QProcess;
    process->start("smsviewer.exe");
    while(!process->waitForFinished(-1)){
        qApp->processEvents();
    }
    ui->btnView->setEnabled(true);
    ui->btnImport->setEnabled(true);
}
