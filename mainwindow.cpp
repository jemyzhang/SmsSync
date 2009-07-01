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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    ui->progressBar->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}
int MainWindow::sqlite_export(){
    ui->btnImport->setEnabled(false);
    ui->labelResult->setText(tr("Connecting to Database..."));
    QSqlDatabase ldb = QSqlDatabase::addDatabase("QSQLITE");
    ldb.setDatabaseName("sms.db");
    bool ok = ldb.open();
    uint nSuccess = 0;
    if(ok){
        QSqlQueryModel * qModel = new QSqlQueryModel;
        //PN,PhoneNumber,Content,TimeStamps,SendReceive
        qModel->setQuery("create temp table if not exists exec (name text,phonenumber text, content text,timestamps datetime,sendreceive numeric)",ldb);
        qModel->setQuery("insert into exec (name,phonenumber,content,timestamps,sendreceive) "
                         "select contacts_v1.Name as name, sms_v1.PN, sms_v1.content as content ,sms_v1.timestamps as timestamps,sms_v1.sendreceive as sendreceive "
                         "from contacts_v1,sms_v1 where (contacts_v1.PhoneNumber =  sms_v1.PN)",ldb);
        qModel->setQuery("insert into exec (phonenumber,content,timestamps,sendreceive) "
                         "select PN as phonenumber,content,timestamps,sendreceive "
                         "from sms_v1 where (select count(*) from contacts_v1 where contacts_v1.PhoneNumber=sms_v1.PN)=0");
        qModel->setQuery("select count(*) from exec",ldb);
        QSqlRecord sqlrecord = qModel->record(0);
        uint nSize = sqlrecord.value(0).toUInt();
        if(nSize > 0){
            ui->progressBar->setRange(0,nSize);

            qModel->setQuery("select * from exec",ldb);

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
                sqlrecord = qModel->record();
                uint ncount = 0;
                ui->progressBar->setVisible(true);
                QString s;
                while(ncount<nSize){
                    ui->labelResult->setText(
                            tr("Importing Records:").append(
                                    QString::number(ncount+1)).append("/").append(
                                            QString::number(nSize).append(":").append(
                                                    QString::number(nSuccess))));
                    ui->progressBar->setValue(ncount+1);
                    sqlrecord = qModel->record(ncount++);
                    QString sqlcmdaction = "insert into SMS (";
                    QString sqlcmdvalue = "values(";
                    QString name = sqlrecord.value(0).toString();
                    if(!name.isNull() && !name.isEmpty()){
                        sqlcmdaction += "名字,";
                        sqlcmdvalue = sqlcmdvalue + "'" + name + "',";
                    }
                    QString phonenumber = sqlrecord.value(1).toString();
                    if(!phonenumber.isNull() && !phonenumber.isEmpty()){
                        sqlcmdaction += "号码,";
                        sqlcmdvalue = sqlcmdvalue + "'" + phonenumber + "',";
                    }
                    QString content = sqlrecord.value(2).toString();
                    if(!content.isNull() && !content.isEmpty()){
                        sqlcmdaction += "内容,";
                        sqlcmdvalue = sqlcmdvalue + "'" + content + "',";
                    }
                    QString timestamps = sqlrecord.value(3).toString();
                    if(!timestamps.isNull() && !timestamps.isEmpty()){
                        sqlcmdaction += "时间,";
                        sqlcmdvalue = sqlcmdvalue + "'" + timestamps + "',";
                    }
                    QString sendreceive = sqlrecord.value(4).toString();
                    if(!sendreceive.isNull() && !sendreceive.isEmpty()){
                        sqlcmdaction += "类型";
                        sqlcmdvalue = sqlcmdvalue + "'" + (sendreceive == "1" ? "发" : "收") + "'";
                    }
                    QSqlQuery mquery(ldbodbc);
                    bool isok=mquery.exec(sqlcmdaction + ") " + sqlcmdvalue + ") ");
                    if(isok) nSuccess++;
                    //清除结果集
                    mquery.clear();
                    sqlcmdaction.clear();
                    sqlcmdvalue.clear();
                }
                ldbodbc.close();
            }else{
                ui->labelResult->setText(tr("Could not open SMS.mdb"));
            }
        }else{
            ui->labelResult->setText(tr("No sms records found"));
        }
        ldb.close();
    }else{
        ui->labelResult->setText(tr("Could not found sms.db"));
    }
    ui->progressBar->setVisible(false);
    ui->btnImport->setEnabled(true);
    return 0;
}

void MainWindow::on_btnImport_clicked()
{
    sqlite_export();
}
