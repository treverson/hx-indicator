#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "wallet.h"
#include "commondialog.h"
#include "dialog/exportsetpwddialog.h"
#include "AES/aesencryptor.h"


ExportDialog::ExportDialog( QString name, QWidget *parent) :
    QDialog(parent),
    accoutName(name),
    ui(new Ui::ExportDialog)
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

    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->exportBtn->setStyleSheet(OKBTN_STYLE);
    ui->cancelBtn->setStyleSheet(CANCELBTN_STYLE);
    ui->closeBtn->setStyleSheet(CLOSEBTN_STYLE);

    ui->nameLabel->setText( tr("Export ") + accoutName + tr(" \'s private key to:"));


    ui->encryptCheckBox->setChecked(true);

}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::pop()
{
    move(0,0);
    exec();
}

void ExportDialog::on_pathBtn_clicked()
{
    QString file = QFileDialog::getExistingDirectory(this,tr( "Select the path"));
    if( !file.isEmpty())
    {
#ifdef WIN32
        file.replace("\\","/");
#endif
        if(ui->encryptCheckBox->isChecked())
        {
            ui->pathLineEdit->setText( file + "/"  + accoutName + ".upk");
        }
        else
        {
            ui->pathLineEdit->setText( file + "/"  + accoutName + ".key");
        }

    }
}

void ExportDialog::on_cancelBtn_clicked()
{
    CommonDialog commonDialog(CommonDialog::OkAndCancel);
//    commonDialog.setText(QString::fromLocal8Bit("您还没有备份私钥，如果数据丢失/损坏 您将没有任何途径找回您的账户。是否确定取消备份?"));
    commonDialog.setText(tr("You have not backup this wallet.If your wallet data is lost or corrupted, you will have no way to get your account back.Sure to cancel?"));
    if(commonDialog.pop())
    {
        close();
    }

}

void ExportDialog::getPrivateKey()
{
    UBChain::getInstance()->postRPC( "id-dump_private_key-" + accoutName, toJsonFormat( "dump_private_key", QJsonArray() << accoutName << "0" ));
}

void ExportDialog::jsonDataUpdated(QString id)
{
    if( id == "id-dump_private_key-" + accoutName)
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
        qDebug() << "eeeeeeeeeeee " << id << result;

        ui->exportBtn->setEnabled(true);

        result.prepend("{");
        result.append("}");

        QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
        QJsonObject jsonObject = parse_doucment.object();
        QJsonArray array = jsonObject.take("result").toArray();
        if(array.size() > 0)
        {
            QJsonArray array2 = array.at(0).toArray();

            QString privateKey = array2.at(1).toString();
            QFile file( path);
            QString fName = file.fileName();
#ifdef WIN32
            fName.replace("\\","/");
#endif
            fName = fName.mid( fName.lastIndexOf("\\") + 1);

            if( file.exists())
            {
                CommonDialog tipDialog(CommonDialog::OkAndCancel);
                tipDialog.setText( fName + tr( " already exists!\nCover it or not?") );
                if ( !tipDialog.pop())  return;
            }

            if( !file.open(QIODevice::WriteOnly))
            {
                qDebug() << "privatekey file open " + fName + " ERROR";

                CommonDialog tipDialog(CommonDialog::OkOnly);
                tipDialog.setText( tr( "Wrong path!") );
                tipDialog.pop();
                return;
            }

            if(isEncrypt)
            {
                unsigned char key2[16] = {0};
                memcpy(key2,pwd.toLatin1().data(),pwd.toLatin1().size());
                AesEncryptor aes(key2);

                QString input = "privateKey=" + privateKey;
                QString output = QString::fromStdString( aes.EncryptString( input.toStdString()) );
qDebug() << "iiiiiiiiiiiii  " << input;
                file.resize(0);
                QTextStream ts( &file);
                ts << output.toUtf8();
                file.close();
            }
            else
            {
                QByteArray ba = privateKey.toUtf8();

                file.resize(0);
                QTextStream ts( &file);
                ts << ba.toBase64();
                file.close();
            }


            close();

            CommonDialog tipDialog(CommonDialog::OkOnly);
            //            tipDialog.setText( tr( "Export to ") + fName + tr(" succeeded!") + QString::fromLocal8Bit("请妥善保管您的私钥，绝对不要丢失或泄露给任何人!") );
            tipDialog.setText( tr( "Export to ") + fName + tr(" succeeded!") + tr("Please keep your private key properly.Never lose or leak it to anyone!") );
            tipDialog.pop();

            QString dirPath = path;
#ifdef WIN32
            dirPath.replace( "/", "\\");
            dirPath = dirPath.left( dirPath.lastIndexOf("\\") );

            QProcess::startDetached("explorer \"" + dirPath + "\"");
#else
            dirPath = dirPath.left( dirPath.lastIndexOf("/") );
            QProcess::startDetached("open \"" + dirPath + "\"");
#endif
        }
        else
        {
            qDebug() << "dump_private_key " + accoutName + " ERROR: " + result;
        }


        return;
    }
}

void ExportDialog::on_exportBtn_clicked()
{
    if( ui->encryptCheckBox->isChecked() && ui->pathLineEdit->text().endsWith(".upk") )
    {
        ExportSetPwdDialog exportSetPwdDialog;
        if( !exportSetPwdDialog.pop())  return;

        pwd = exportSetPwdDialog.pwd;
        path = ui->pathLineEdit->text();
        isEncrypt = ui->encryptCheckBox->isChecked();

        getPrivateKey();

        ui->exportBtn->setEnabled(false);
    }
    else if( !ui->encryptCheckBox->isChecked() && ui->pathLineEdit->text().endsWith(".key") )
    {
        path = ui->pathLineEdit->text();
        isEncrypt = ui->encryptCheckBox->isChecked();

        getPrivateKey();

        ui->exportBtn->setEnabled(false);
    }
    else
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("Wrong file format!"));
        commonDialog.pop();
        return;
    }


}

void ExportDialog::on_encryptCheckBox_stateChanged(int arg1)
{
    if(ui->encryptCheckBox->isChecked())
    {
        QDir dir;
#ifdef WIN32
        QString path = dir.currentPath().replace("\\", "/");
#else
        QString path = dir.currentPath();
#endif
        ui->pathLineEdit->setText( path + "/" + accoutName + ".upk");
    }
    else
    {
        QDir dir;
#ifdef WIN32
        QString path = dir.currentPath().replace("\\", "/");
#else
        QString path = dir.currentPath();
#endif
        ui->pathLineEdit->setText( path + "/" + accoutName + ".key");
    }
}

void ExportDialog::on_closeBtn_clicked()
{
    on_cancelBtn_clicked();
}
