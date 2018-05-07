#include "minerpage.h"
#include "ui_minerpage.h"

#include <QPainter>
#include "wallet.h"
#include "registerdialog.h"
#include "locktominerdialog.h"
#include "foreclosedialog.h"
#include "commondialog.h"

#include "poundage/PageScrollWidget.h"
#include "ToolButtonWidget.h"

static const int ROWNUMBER = 4;
MinerPage::MinerPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MinerPage)
{
    ui->setupUi(this);

    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->lockBalancesTableWidget->installEventFilter(this);
    ui->lockBalancesTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->lockBalancesTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->lockBalancesTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->lockBalancesTableWidget->setFrameShape(QFrame::NoFrame);
    ui->lockBalancesTableWidget->setMouseTracking(true);
    ui->lockBalancesTableWidget->setShowGrid(false);//隐藏表格线

    ui->lockBalancesTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->lockBalancesTableWidget->horizontalHeader()->setFixedHeight(30);
    ui->lockBalancesTableWidget->horizontalHeader()->setVisible(true);
    ui->lockBalancesTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->lockBalancesTableWidget->setColumnWidth(0,140);
    ui->lockBalancesTableWidget->setColumnWidth(1,140);
    ui->lockBalancesTableWidget->setColumnWidth(2,80);
    ui->lockBalancesTableWidget->setColumnWidth(3,80);
    ui->lockBalancesTableWidget->setColumnWidth(4,80);
    ui->lockBalancesTableWidget->setColumnWidth(5,80);


    ui->incomeTableWidget->installEventFilter(this);
    ui->incomeTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->incomeTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->incomeTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->incomeTableWidget->setFrameShape(QFrame::NoFrame);
    ui->incomeTableWidget->setMouseTracking(true);
    ui->incomeTableWidget->setShowGrid(false);//隐藏表格线

    ui->incomeTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->incomeTableWidget->horizontalHeader()->setFixedHeight(30);
    ui->incomeTableWidget->horizontalHeader()->setVisible(true);
    ui->incomeTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->incomeTableWidget->setColumnWidth(0,140);
    ui->incomeTableWidget->setColumnWidth(1,140);
    ui->incomeTableWidget->setColumnWidth(2,80);

    ui->incomeRecordTableWidget->installEventFilter(this);
    ui->incomeRecordTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->incomeRecordTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->incomeRecordTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->incomeRecordTableWidget->setFrameShape(QFrame::NoFrame);
    ui->incomeRecordTableWidget->setMouseTracking(true);
    ui->incomeRecordTableWidget->setShowGrid(false);//隐藏表格线

    ui->incomeRecordTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->incomeRecordTableWidget->horizontalHeader()->setFixedHeight(30);
    ui->incomeRecordTableWidget->horizontalHeader()->setVisible(true);
    ui->incomeRecordTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->incomeRecordTableWidget->setColumnWidth(0,140);
    ui->incomeRecordTableWidget->setColumnWidth(1,140);


    pageWidget_income = new PageScrollWidget();
    ui->stackedWidget_income->addWidget(pageWidget_income);
    pageWidget_fore = new PageScrollWidget();
    ui->stackedWidget_fore->addWidget(pageWidget_fore);
    pageWidget_record = new PageScrollWidget();
    ui->stackedWidget_record->addWidget(pageWidget_record);
    connect(pageWidget_income,&PageScrollWidget::currentPageChangeSignal,this,&MinerPage::pageChangeSlot);
    connect(pageWidget_fore,&PageScrollWidget::currentPageChangeSignal,this,&MinerPage::pageChangeSlot);
    connect(pageWidget_record,&PageScrollWidget::currentPageChangeSignal,this,&MinerPage::pageChangeSlot);

    InitStyle();
    init();
}

MinerPage::~MinerPage()
{
    delete ui;
}

void MinerPage::refresh()
{
    fetchLockBalance();
    fetchAccountIncome();
    showIncomeRecord();
}

void MinerPage::jsonDataUpdated(QString id)
{
    if( id == "id-get_account_lock_balance-" + ui->accountComboBox->currentText())
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);

        result.prepend("{");
        result.append("}");

        QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
        QJsonObject jsonObject = parse_doucment.object();
        QJsonArray array = jsonObject.take("result").toArray();
        int size = array.size();
        ui->lockBalancesTableWidget->setRowCount(0);
        ui->lockBalancesTableWidget->setRowCount(size);

        for(int i = 0; i < size; i++)
        {
            QJsonObject object = array.at(i).toObject();
            QString minerId = object.take("lockto_miner_account").toString();
            QString assetId = object.take("lock_asset_id").toString();
            unsigned long long lockAmount = 0;
            QJsonValue value = object.take("lock_asset_amount");
            if(value.isString())
            {
                lockAmount = value.toString().toULongLong();
            }
            else
            {
                lockAmount = QString::number(value.toDouble(),'g',10).toULongLong();
            }

            ui->lockBalancesTableWidget->setItem(i,0,new QTableWidgetItem(UBChain::getInstance()->getMinerNameFromId(minerId)));

            AssetInfo assetInfo = UBChain::getInstance()->assetInfoMap.value(assetId);
            ui->lockBalancesTableWidget->setItem(i,1,new QTableWidgetItem(assetInfo.symbol));

            ui->lockBalancesTableWidget->setItem(i,2,new QTableWidgetItem(getBigNumberString(lockAmount,assetInfo.precision)));

            ui->lockBalancesTableWidget->setItem(i,3,new QTableWidgetItem(tr("add")));

            ToolButtonWidget *buttonAdd = new ToolButtonWidget();
            buttonAdd->setText(ui->lockBalancesTableWidget->item(i,3)->text());
            ui->lockBalancesTableWidget->setCellWidget(i,3,buttonAdd);
            connect(buttonAdd,&ToolButtonWidget::clicked,std::bind(&MinerPage::on_lockBalancesTableWidget_cellPressed,this,i,3));

            ui->lockBalancesTableWidget->setItem(i,4,new QTableWidgetItem(tr("foreclose")));
            ToolButtonWidget *buttonfore = new ToolButtonWidget();
            buttonfore->setInitGray(true);
            buttonfore->setText(ui->lockBalancesTableWidget->item(i,4)->text());
            ui->lockBalancesTableWidget->setCellWidget(i,4,buttonfore);
            connect(buttonfore,&ToolButtonWidget::clicked,std::bind(&MinerPage::on_lockBalancesTableWidget_cellPressed,this,i,4));

        }
        pageWidget_fore->SetTotalPage(calPage(ui->lockBalancesTableWidget));
        setTextCenter(ui->lockBalancesTableWidget);

        return;
    }

    if( id == "id-foreclose_balance_from_miner")
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);

        qDebug() << id << result;

        if(result.startsWith("\"result\":{"))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Foreclose balance from miner successfully!"));
            commonDialog.pop();
        }

        return;
    }

    if( id == "id-get_address_pay_back_balance-" + ui->accountComboBox->currentText())
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);

        result.prepend("{");
        result.append("}");

        QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
        QJsonObject jsonObject = parse_doucment.object();
        QJsonArray array = jsonObject.take("result").toArray();
        int size = array.size();
        ui->incomeTableWidget->setRowCount(0);
        ui->incomeTableWidget->setRowCount(size);

        for(int i = 0; i < size; i++)
        {
            QJsonObject object = array.at(i).toObject();
            QString assetId = object.take("asset_id").toString();
            unsigned long long amount = 0;
            QJsonValue value = object.take("amount");
            if(value.isString())
            {
                amount = value.toString().toULongLong();
            }
            else
            {
                amount = QString::number(value.toDouble(),'g',10).toULongLong();
            }

            AssetInfo assetInfo = UBChain::getInstance()->assetInfoMap.value(assetId);
            ui->incomeTableWidget->setItem(i,0,new QTableWidgetItem(assetInfo.symbol));

            ui->incomeTableWidget->setItem(i,1,new QTableWidgetItem(getBigNumberString(amount,assetInfo.precision)));

            ui->incomeTableWidget->setItem(i,2,new QTableWidgetItem(tr("obtain")));
            ToolButtonWidget *buttonInc = new ToolButtonWidget();
            buttonInc->setText(ui->incomeTableWidget->item(i,2)->text());
            ui->incomeTableWidget->setCellWidget(i,2,buttonInc);
            connect(buttonInc,&ToolButtonWidget::clicked,std::bind(&MinerPage::on_incomeTableWidget_cellPressed,this,i,2));


        }
        pageWidget_income->SetTotalPage(calPage(ui->incomeTableWidget));
        setTextCenter(ui->incomeTableWidget);

        return;
    }

    if( id == "id-obtain_pay_back_balance")
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);

        qDebug() << id << result;

        if(result.startsWith("\"result\":{"))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("The transaction of obtain-income has been sent out!"));
            commonDialog.pop();
        }

        return;
    }
}

void MinerPage::on_registerBtn_clicked()
{
    if(UBChain::getInstance()->getUnregisteredAccounts().isEmpty())
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("There are no unregistered accounts in the wallet!"));
        commonDialog.pop();
    }
    else
    {
        RegisterDialog registerDialog;
        registerDialog.pop();
    }

}

void MinerPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(248,249,253),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(248,249,253),Qt::SolidPattern));

    painter.drawRect(rect());
}

void MinerPage::init()
{
    if( UBChain::getInstance()->getRegisteredAccounts().isEmpty())
    {
        ui->tipLabel->show();
        ui->stackedWidget->hide();
        ui->incomeInfoBtn->hide();
        ui->forecloseInfoBtn->hide();
        ui->lockToMinerBtn->hide();
        ui->accountComboBox->hide();
        ui->accountLabel->hide();
    }
    else
    {
        ui->tipLabel->hide();
        ui->stackedWidget->show();
        ui->incomeInfoBtn->show();
        ui->forecloseInfoBtn->show();
        ui->lockToMinerBtn->show();
        ui->accountComboBox->show();
        ui->accountLabel->show();

        QStringList keys = UBChain::getInstance()->getRegisteredAccounts();
        ui->accountComboBox->addItems(keys);
    }

    ui->stackedWidget->setCurrentIndex(0);
    updateCheckState(0);
}

void MinerPage::fetchLockBalance()
{
    UBChain::getInstance()->postRPC( "id-get_account_lock_balance-" + ui->accountComboBox->currentText(),
                                     toJsonFormat( "get_account_lock_balance",QJsonArray() << ui->accountComboBox->currentText()));
}

void MinerPage::fetchAccountIncome()
{
    QString address = UBChain::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address;
    UBChain::getInstance()->postRPC( "id-get_address_pay_back_balance-" + ui->accountComboBox->currentText(),
                                     toJsonFormat( "get_address_pay_back_balance",QJsonArray() << address << ""));
}

void MinerPage::showIncomeRecord()
{
    QString address = UBChain::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address;
    TransactionTypeIds typeIds = UBChain::getInstance()->transactionDB.getAccountTransactionTypeIdsByType(address,TRANSACTION_TYPE_MINE_INCOME);

    int size = typeIds.size();
    ui->incomeRecordTableWidget->setRowCount(0);
    ui->incomeRecordTableWidget->setRowCount(size);

    for(int i = 0; i < size; i++)
    {
        QString transactionId = typeIds.at(size - i - 1).transactionId;
        TransactionStruct ts = UBChain::getInstance()->transactionDB.getTransactionStruct(transactionId);
        if(ts.type == -1)
        {
            qDebug() << "can not find transaction in transactionstruct db: " << transactionId;
            continue;
        }


        QJsonObject object = QJsonDocument::fromJson(ts.operationStr.toLatin1()).object();
        QJsonObject amountObject = object.take("pay_back_balance").toArray().at(0).toArray().at(1).toObject();
        unsigned long long amount = jsonValueToULL(amountObject.take("amount"));
        QString amountAssetId = amountObject.take("asset_id").toString();
        AssetInfo amountAssetInfo = UBChain::getInstance()->assetInfoMap.value(amountAssetId);

        ui->incomeRecordTableWidget->setItem(i,0, new QTableWidgetItem(QString::number(ts.blockNum)));
        ui->incomeRecordTableWidget->setItem(i,1, new QTableWidgetItem(getBigNumberString(amount, amountAssetInfo.precision) + " " + amountAssetInfo.symbol));

    }
    pageWidget_record->SetTotalPage(calPage(ui->incomeRecordTableWidget));
    setTextCenter(ui->incomeRecordTableWidget);
}

void MinerPage::InitStyle()
{
    ui->forecloseInfoBtn->setCheckable(true);
    ui->incomeInfoBtn->setCheckable(true);
    ui->incomeRecordBtn->setCheckable(true);

    setStyleSheet("QToolButton#forecloseInfoBtn,QToolButton#incomeInfoBtn,QToolButton#incomeRecordBtn{border:none;background:transparent;color:#C6CAD4;}"
                  "QToolButton#forecloseInfoBtn::checked,QToolButton#incomeInfoBtn::checked,QToolButton#incomeRecordBtn::checked{color:black;}"
                  "QToolButton#lockToMinerBtn{background-color:#5474EB; border:none;border-radius:10px;color: rgb(255, 255, 255);}"
                  "QToolButton#lockToMinerBtn::hover{background-color:#00D2FF;}"
                  "QToolButton#registerBtn{background-color:#E5E5E5; border:none;border-radius:10px;color: rgb(255, 255, 255);}"
                  "QToolButton#registerBtn::hover{background-color:#00D2FF;}"

                  "QTableView{background-color:#FFFFFF;border:none;border-radius:10px;}"
                  "QHeaderView{border:none;color:#C6CAD4;font-size:12pt;}"
                  "QHeaderView:section{height:40px;border:none;background-color:#FFFFFF;}"
                  "QTableView:item{min-height:40px;}");

}

void MinerPage::updateCheckState(int number)
{
    ui->incomeInfoBtn->setChecked(0 == number);
    ui->forecloseInfoBtn->setChecked(1 == number);
    ui->incomeRecordBtn->setChecked(2 == number);
}

unsigned int MinerPage::calPage(const QTableWidget * const table) const
{
    if(!table) return 0;
    int page = (table->rowCount()%ROWNUMBER==0 && table->rowCount() != 0) ?
                table->rowCount()/ROWNUMBER : table->rowCount()/ROWNUMBER+1;

}

void MinerPage::setTextCenter(QTableWidget * const table)
{
    if(!table) return;
    for(int i = 0;i < table->rowCount();++i)
    {
        for(int j = 0;j < table->columnCount();++j)
        {
            table->item(i,j)->setTextAlignment(Qt::AlignCenter);
        }
    }
}

void MinerPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    fetchLockBalance();
    fetchAccountIncome();
    showIncomeRecord();
}

void MinerPage::on_lockToMinerBtn_clicked()
{
    if(ui->accountComboBox->currentText().isEmpty())    return;

    LockToMinerDialog lockToMinerDialog(ui->accountComboBox->currentText());
    lockToMinerDialog.pop();
}

void MinerPage::on_lockBalancesTableWidget_cellPressed(int row, int column)
{
    if(column == 3)
    {
        LockToMinerDialog lockToMinerDialog(ui->accountComboBox->currentText());
        lockToMinerDialog.setMiner(ui->lockBalancesTableWidget->item(row,0)->text());
        lockToMinerDialog.setAsset(ui->lockBalancesTableWidget->item(row,1)->text());
        lockToMinerDialog.pop();
        return;
    }

    if(column == 4)
    {
        ForecloseDialog forecloseDialog;
        QString amountStr = forecloseDialog.pop();

        UBChain::getInstance()->postRPC( "id-foreclose_balance_from_miner",
                                         toJsonFormat( "foreclose_balance_from_miner",
                                                       QJsonArray() << ui->lockBalancesTableWidget->item(row,0)->text()
                                                       << ui->accountComboBox->currentText()
                                                       << amountStr << ui->lockBalancesTableWidget->item(row,1)->text()
                                                       << true ));

        return;
    }
}

void MinerPage::on_incomeTableWidget_cellPressed(int row, int column)
{
    if(column == 2)
    {
        CommonDialog commonDialog(CommonDialog::OkAndCancel);
        commonDialog.setText(tr("Sure to obtain all the current income?"));
        if(commonDialog.pop())
        {
            QString address = UBChain::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText()).address;
            UBChain::getInstance()->postRPC( "id-obtain_pay_back_balance",
                                             toJsonFormat( "obtain_pay_back_balance",
                                                           QJsonArray() << address
                                                           << ui->incomeTableWidget->item(row,1)->text()
                                                           << ui->incomeTableWidget->item(row,0)->text()
                                                           << true ));
        }

        return;
    }
}

void MinerPage::on_incomeInfoBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    updateCheckState(0);
}

void MinerPage::on_forecloseInfoBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    updateCheckState(1);
}

void MinerPage::on_incomeRecordBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    updateCheckState(2);
}

void MinerPage::pageChangeSlot(unsigned int page)
{
    QTableWidget *table = nullptr;
    if(sender() == pageWidget_income)
    {//当前收益翻页
        table = ui->incomeTableWidget;
    }
    else if(sender() == pageWidget_fore)
    {
        table = ui->lockBalancesTableWidget;
    }
    else if(sender() == pageWidget_record)
    {
        table = ui->incomeRecordTableWidget;
    }
    if(!table) return;

    for(int i = 0;i < table->rowCount();++i)
    {
        if(i < page*ROWNUMBER)
        {
            table->setRowHidden(i,true);
        }
        else if(page * ROWNUMBER <= i && i < page*ROWNUMBER + ROWNUMBER)
        {
            table->setRowHidden(i,false);
        }
        else
        {
            table->setRowHidden(i,true);
        }
    }
}
