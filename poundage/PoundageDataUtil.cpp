#include "PoundageDataUtil.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "wallet.h"

PoundageDataUtil::PoundageDataUtil()
{

}

bool PoundageDataUtil::ParseJsonObjToUnit(QJsonObject jsonObj,std::shared_ptr<PoundageUnit>& poundageUnit)
{
    if(!poundageUnit) return false;
    poundageUnit->poundageID = jsonObj.value("id").toString();
    poundageUnit->ownerAdress = jsonObj.value("owner_addr").toString();
    poundageUnit->chainType = jsonObj.value("chain_type").toString();
    poundageUnit->publishTime = QDateTime::fromString(jsonObj.value("time").toString(),"yyyy-MM-ddTHH:mm:ss");

    QJsonObject oriObj = jsonObj.value("asset_orign").toObject();
    int oriPre = UBChain::getInstance()->assetInfoMap[oriObj.value("asset_id").toString()].precision;
    poundageUnit->sourceCoinNumber =  oriObj.value("amount").toDouble()/pow(10,oriPre);
    poundageUnit->sourceCoinID = oriObj.value("asset_id").toString();

    QJsonObject tarObj = jsonObj.value("asset_target").toObject();
    int tarPre = UBChain::getInstance()->assetInfoMap[tarObj.value("asset_id").toString()].precision;
    poundageUnit->targetCoinNumber = (double)jsonValueToULL(tarObj.value("amount"))/pow(10,tarPre);
    poundageUnit->targetCoinID = tarObj.value("asset_id").toString();

    QJsonObject finishObj = jsonObj.value("asset_finished").toObject();
    poundageUnit->balanceNumber = poundageUnit->targetCoinNumber - finishObj.value("amount").toDouble()/pow(10,tarPre);

    poundageUnit->poundageFinished = jsonObj.value("finished").toBool();

    qDebug()<<poundageUnit->sourceCoinNumber << poundageUnit->targetCoinNumber;
    return true;
}

bool PoundageDataUtil::convertJsonToPoundage(const QString &jsonString,std::shared_ptr<PoundageSheet> &sheet)
{
    if(!sheet) sheet = std::make_shared<PoundageSheet>();
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(jsonString.toLatin1(),&json_error);
    if(json_error.error != QJsonParseError::NoError || !parse_doucment.isObject()) return false;
    QJsonObject jsonObject = parse_doucment.object();
    QJsonArray objectArr = jsonObject.take("result").toArray();
    foreach(QJsonValue val, objectArr){
        if(!val.isObject()) continue;
        QJsonObject unitObj = val.toObject();
        std::shared_ptr<PoundageUnit> unit = std::make_shared<PoundageUnit>();
        if(ParseJsonObjToUnit(unitObj,unit))
        {
            if(!sheet->isIdExisted(unit->poundageID))
            {
                sheet->push_back(unit);
            }
        }

    }
    return true;
}
