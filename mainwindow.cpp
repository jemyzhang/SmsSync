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

#include "LocalDataBase.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    ui->progressBar->setVisible(false);
    //sqlite_checkpwd();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::sqlite_checkpwd(){
    LocalDataBase db;
    if(db.connect()){
        UINT nReceived,nSent;
        db.GetSmsCount(nReceived,nSent);
    }
    return false;
}

int MainWindow::sqlite_export(){
    ui->btnImport->setEnabled(false);
    ui->labelResult->setText(tr("Connecting to Database..."));
    LocalDataBase ldb;
    bool ok = ldb.connect();
    uint nSuccess = 0;
    if(ok){

        UINT nSent,nReceived;
        bool rc = ldb.GetSmsCount(nReceived,nSent);
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
            QSqlDatabase ldbodbc = QSqlDatabase::addDatabase("QODBC");
            ldbodbc.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};DBQ=SMS.mdb;UID=;PWD=1a2b3c4d5c6d7e8f");
            bool okodbc = ldbodbc.open();
            bool bret;
            if(okodbc){
                uint ncount = 0;
                ui->progressBar->setVisible(true);
                QString s;
                while(ncount<nSize){
                    ui->progressBar->setValue(ncount+1);
                    SmsSimpleData_t smsData;
                    ldb.GetSms(ncount++,&smsData);
                    QString sqlcmdaction = "insert into SMS (";
                    QString sqlcmdvalue = "values(";
                    QString name = QString::fromStdWString(smsData.ContactName);
                    if(!name.isNull() && !name.isEmpty()){
                        sqlcmdaction += "名字,";
                        sqlcmdvalue = sqlcmdvalue + "'" + name + "',";
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
                    }
                    bool sendreceive = smsData.SendReceiveFlag;
                    sqlcmdaction += "类型";
                    sqlcmdvalue = sqlcmdvalue + "'" + (sendreceive ? "发" : "收") + "'";

                    QSqlQuery mquery(ldbodbc);
                    bool isok=mquery.exec(sqlcmdaction + ") " + sqlcmdvalue + ") ");
                    if(isok) nSuccess++;
                    //label显示
                    ui->labelResult->setText(
                            tr("Importing Records:").append(
                                    QString::number(ncount)).append("/").append(
                                            QString::number(nSize).append(" Imported:").append(
                                                    QString::number(nSuccess))));
                    //清除结果集
                    mquery.clear();
                    sqlcmdaction.clear();
                    sqlcmdvalue.clear();
                    smsData.Reset();
                }
                ldbodbc.close();
            }else{
                ui->labelResult->setText(ldbodbc.lastError().text());//tr("Could not open SMS.mdb"));
            }
        }else{
            ui->labelResult->setText(tr("No sms records found"));
        }
        ldb.disconnect();
    }else{
        ui->labelResult->setText(tr("Could not found or open sms.db"));
    }
    ui->progressBar->setVisible(false);
    ui->btnImport->setEnabled(true);
    return 0;
}

void MainWindow::on_btnImport_clicked()
{
    sqlite_export();
}
