#include "ui_password.h"
#include "ui_ui_password.h"
#include "LocalDataBase.h"
#include <QMessageBox>
extern wchar_t g_password[256];
extern int g_password_len;

Ui_PasswordWnd::Ui_PasswordWnd(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::Ui_PasswordWnd)
{
    m_ui->setupUi(this);
    m_ui->EditPassword->setEchoMode(QLineEdit::Password);
}

Ui_PasswordWnd::~Ui_PasswordWnd()
{
    delete m_ui;
}

void Ui_PasswordWnd::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void Ui_PasswordWnd::on_btnOK_clicked()
{
    LocalDataBase ldb;
    QString s = m_ui->EditPassword->text();
    wchar_t defaultpwd[6] = {0x10,0x15,0x13,0x18,0x08,0x01};
    if(s.isEmpty()){
        g_password_len = 6;
        memcpy(g_password,defaultpwd,sizeof(wchar_t)*6);
    }else{
        g_password_len = s.toWCharArray(g_password);
    }
    if(!ldb.checkpwd(g_password,g_password_len)){
        QMessageBox::critical(this,"Password","Password Invalid...");
    }else{
        done(QDialog::Accepted);
    }
}

void Ui_PasswordWnd::on_btnCancel_clicked()
{
    done(QDialog::Rejected);
}
