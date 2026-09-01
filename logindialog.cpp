#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle("注册&&登录");
    //窗口默认注册
    ui->tw_page->setCurrentIndex(1);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
#include<QMessageBox>
#include<QRegExp>
void LoginDialog::on_pb_login_register_clicked()
{
    //注册信息采集
    QString tel=ui->le_tel_register->text();
    QString password=ui->le_password_register->text();
    QString name=ui->le_name_register->text();
    QString confirm=ui->le_confirm_register->text();
    QString tmpname=name;
    //过滤
    //查看输入是否为空
    if(tel.isEmpty() || password.isEmpty() || name.isEmpty() || confirm.isEmpty() || tmpname.remove(" ").isEmpty())
    {
        QMessageBox::about(this,"提示","输入内容，不可为空");
        return;
    }
    //手机号是否合法--正则表达式
    QRegExp exp("^1[3456789][0-9]\{9\}$");
    bool res=exp.exactMatch(tel);
    if(!res)
    {
        QMessageBox::about(this,"提示","手机号非法");
        return;
    }
    //密码是否过长
    if(password.size()>20)
    {
        QMessageBox::about(this,"提示","密码过长，长度小于20");
        return;
    }
    //密码要与确认一致
    if(password!=confirm)
    {
        QMessageBox::about(this,"提示","两次输入密码不一致");
        return;
    }
    //昵称是否过长 --敏感词汇的过滤
    if(name.size()>10)
    {
        QMessageBox::about(this,"提示","昵称过长，不能超过10");
        return;
    }
    //发信号 Q_EMIT emit
    Q_EMIT SIG_registerCommit(tel,password,name);
}
void LoginDialog::on_pb_login_clicked()
{
    //注册信息采集
    QString tel=ui->le_tel->text();
    QString password=ui->le_password->text();
    //过滤
    //查看输入是否为空
    if(tel.isEmpty() || password.isEmpty())
    {
        QMessageBox::about(this,"提示","输入内容，不可为空");
        return;
    }
    //手机号是否合法--正则表达式
    QRegExp exp("^1[3456789][0-9]\{9\}$");
    bool res=exp.exactMatch(tel);
    if(!res)
    {
        QMessageBox::about(this,"提示","手机号非法");
        return;
    }
    //密码是否过长---- 敏感词汇的过滤
    if(password.size()>20)
    {
        QMessageBox::about(this,"提示","密码过长，长度小于20");
        return;
    }

    //发信号 Q_EMIT emit
    Q_EMIT SIG_loginCommit(tel,password);
}

void LoginDialog::on_pb_clear_register_clicked()
{
    ui->le_confirm_register->setText("");
    ui->le_name_register->setText("");
    ui->le_password_register->setText("");
    ui->le_tel_register->setText("");
}
void LoginDialog::on_pb_clear_clicked()
{
    ui->le_password->setText("");
    ui->le_tel->setText("");
}

