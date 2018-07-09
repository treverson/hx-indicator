#ifndef COLDHOTTRANSFERPAGE_H
#define COLDHOTTRANSFERPAGE_H

#include <QWidget>
#include <QMap>

namespace Ui {
class ColdHotTransferPage;
}

struct ColdHotTransaction
{
    QString trxId;
    QString expirationTime;
    QString withdrawAddress;
    QString depositAddress;
    QString amount;
    QString assetSymbol;
    QString assetId;
    QString guard;
    QString guardId;
    QString memo;
};

struct ColdHotCrosschainTransaction
{
    QString trxId;
    QString coldHotTrxId;
};

struct ColdHotSignTransaction
{
    QString trxId;
    QString coldHotCrosschainTrxId;
    QString guardAddress;
};

class ColdHotTransferPage : public QWidget
{
    Q_OBJECT

public:
    explicit ColdHotTransferPage(QWidget *parent = 0);
    ~ColdHotTransferPage();

    void init();

    void refresh();

    int currentType = 1;
    QMap<QString,ColdHotTransaction> coldHotTransactionMap;
    QMap<QString,ColdHotTransaction> pendingColdHotTransactionMap;
    QMap<QString,ColdHotCrosschainTransaction> coldHotCrosschainTransactionMap;
    QMap<QString,ColdHotSignTransaction> coldHotSignTransactionMap;
    QString lookupCrosschainTrxByColdHotTrxId(QString coldHotTrxId);
    QStringList lookupSignedGuardByCrosschainTrx(QString crosschainTrxId);

private slots:
    void jsonDataUpdated(QString id);

    void on_transferBtn_clicked();

    void on_coldHotTransactionTableWidget_cellClicked(int row, int column);

    void on_accountComboBox_currentIndexChanged(const QString &arg1);

    void on_coldHotTransactionTableWidget_cellPressed(int row, int column);

    void on_typeCurrentBtn_clicked();

    void on_typeWaitingBtn_clicked();

private:
    Ui::ColdHotTransferPage *ui;
    bool inited = false;

    void fetchColdHotTransaction();
    void showColdHotTransactions();
    void refreshColdHotTtransactionsState();

    void paintEvent(QPaintEvent*);
};

#endif // COLDHOTTRANSFERPAGE_H
