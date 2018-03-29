﻿#include "namedialog.h"
#include "ui_namedialog.h"
#include "debug_log.h"
#include "lnk.h"

#include <QMovie>

NameDialog::NameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NameDialog)
{
    
    ui->setupUi(this);

//    UBChain::getInstance()->appendCurrentDialogVector(this);
    setParent(UBChain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->okBtn->setStyleSheet(OKBTN_STYLE);
    ui->cancelBtn->setStyleSheet(CANCELBTN_STYLE);
    ui->closeBtn->setStyleSheet(CLOSEBTN_STYLE);


    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    yesOrNO = true;

    ui->nameLineEdit->setPlaceholderText( tr("Beginning with letter,letters or numbers"));
    ui->nameLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);


    ui->okBtn->setText(tr("Ok"));
    ui->okBtn->setEnabled(false);

    ui->cancelBtn->setText(tr("Cancel"));

    QRegExp regx("[a-z][a-z0-9\-]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->nameLineEdit->setValidator( validator );

    ui->nameLineEdit->setFocus();

    gif = new QMovie(":/pic/pic2/loading2.gif");
    gif->setScaledSize( QSize(18,18));
    ui->gifLabel->setMovie(gif);
    gif->start();
    ui->gifLabel->hide();

    
}

NameDialog::~NameDialog()
{
    delete ui;
//    UBChain::getInstance()->removeCurrentDialogVector(this);
}

void NameDialog::on_okBtn_clicked()
{
    yesOrNO = true;
    close();
//    emit accepted();
}

void NameDialog::on_cancelBtn_clicked()
{
    yesOrNO = false;
    close();
//    emit accepted();
}

QString NameDialog::pop()
{
    ui->nameLineEdit->grabKeyboard();

//    QEventLoop loop;
//    show();
//    connect(this,SIGNAL(accepted()),&loop,SLOT(quit()));
//    loop.exec();  //进入事件 循环处理，阻塞

    move(0,0);
    exec();

    if( yesOrNO == true)
    {
        return ui->nameLineEdit->text();
    }
    else
    {
        return "";
    }
}

bool isExistInWallet(QString);

void NameDialog::on_nameLineEdit_textChanged(const QString &arg1)
{
    if( arg1.isEmpty())
    {
        ui->okBtn->setEnabled(false);
        ui->addressNameTipLabel2->setText("" );
        return;
    }

    QString addrName = arg1;

    if( 63 < addrName.size() )
    {

        ui->okBtn->setEnabled(false);
        ui->addressNameTipLabel2->setText("<body><font style=\"font-size:12px\" color=#ff224c>" + tr("More than 63 characters!") + "</font></body>" );

        return;
    }


    //检查地址名是否在钱包内已经存在
    if( isExistInWallet(addrName) )
    {
        ui->okBtn->setEnabled(false);
        ui->addressNameTipLabel2->setText("<body><font style=\"font-size:12px\" color=#ff224c>" + tr( "This name has been used") + "</font></body>" );
        return;
    }

    //检查地址名是否已经在链上注册了 blockchain_get_account(accountName)
//    RpcThread* rpcThread = new RpcThread;
//    connect(rpcThread,SIGNAL(finished()),rpcThread,SLOT(deleteLater()));
//    rpcThread->setWriteData( toJsonFormat( "id_blockchain_get_account_" + addrName, "blockchain_get_account", QStringList() << addrName ));
//    rpcThread->start();

    UBChain::getInstance()->postRPC( "id-get_account-" + addrName, toJsonFormat( "get_account", QStringList() << addrName ));
    ui->gifLabel->show();
}

void NameDialog::on_nameLineEdit_returnPressed()
{
    if( !ui->okBtn->isEnabled())  return;

    on_okBtn_clicked();
}

void NameDialog::jsonDataUpdated(QString id)
{
    if( id.startsWith("id-get_account-") )
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);

        // 如果跟当前输入框中的内容不一样，则是过时的rpc返回，不用处理
        if( id.mid(QString("id-get_account-").size()) != ui->nameLineEdit->text())  return;

        ui->gifLabel->hide();

        if( result.startsWith("\"result\":{\"id\":\"0.0.0\""))
        {
            ui->okBtn->setEnabled(true);
            ui->addressNameTipLabel2->setText("<body><font style=\"font-size:12px\" color=#2be683>" + tr( "The name is available") + "</font></body>" );
        }
        else
        {
            ui->okBtn->setEnabled(false);
            ui->addressNameTipLabel2->setText("<body><font style=\"font-size:12px\" color=#ff224c>" + tr( "This name has been used") + "</font></body>" );
        }

        return;
    }

}

void NameDialog::on_closeBtn_clicked()
{
    on_cancelBtn_clicked();
}
