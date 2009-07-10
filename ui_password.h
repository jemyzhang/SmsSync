#ifndef UI_PASSWORD_H
#define UI_PASSWORD_H

#include <QtGui/QDialog>

namespace Ui {
    class Ui_PasswordWnd;
}

class Ui_PasswordWnd : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(Ui_PasswordWnd)
public:
    explicit Ui_PasswordWnd(QWidget *parent = 0);
    virtual ~Ui_PasswordWnd();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::Ui_PasswordWnd *m_ui;


private slots:
    void on_btnCancel_clicked();
    void on_btnOK_clicked();
};

#endif // UI_PASSWORD_H
