#include "websocketmanager.h"

#include "lnk.h"

WebSocketManager::WebSocketManager(QObject *parent)
    : QThread(parent)
    , busy(true)
    , m_webSocket(NULL)
{

}

WebSocketManager::~WebSocketManager()
{
    if(m_webSocket)
    {
        delete m_webSocket;
        m_webSocket = NULL;
    }
}


void WebSocketManager::connectToClient()
{
    m_webSocket->open( QUrl(QString("ws://127.0.0.1:%1").arg(CLIENT_RPC_PORT)) );
}

void WebSocketManager::processRPC(QString _rpcId, QString _rpcCmd)
{
    busy = true;
    m_rpcId = _rpcId;
    m_webSocket->sendTextMessage(_rpcCmd);
}

void WebSocketManager::processRPCs(QString _rpcId, QString _rpcCmd)
{
    bool processed = false;

retry:
    if(!busy)
    {
        processRPC(_rpcId,_rpcCmd);
        processed = true;
    }

    if( !processed)
    {
        if(loopCount > 10)
        {
            m_buff.clear();
            m_rpcId.clear();
            loopCount = 0;
            return;
        }
        else
        {
            QThread::msleep(10);
            loopCount++;
            goto retry;
        }

    }
}

void WebSocketManager::run()
{
    m_webSocket = new QWebSocket;
    connect(m_webSocket,SIGNAL(connected()),this,SLOT(onConnected()));
    connect(m_webSocket,SIGNAL(textFrameReceived(QString,bool)),this,SLOT(onTextFrameReceived(QString,bool)),Qt::QueuedConnection);

    connectToClient();

    busy = false;

    exec();
}

void WebSocketManager::onConnected()
{
    qDebug() << "websocket connected" << m_webSocket->state();

    isConnected = true;
}

void WebSocketManager::onTextFrameReceived(QString _message, bool _isLastFrame)
{
    qDebug() << "message received: " << m_rpcId << _message;

    m_buff += _message;

    loopCount = 0;

    if(_isLastFrame)
    {

        QString result = m_buff.mid( QString("{\"id\":32800,\"jsonrpc\":\"2.0\",").size());
        result = result.left( result.size() - 1);

        UBChain::getInstance()->updateJsonDataMap(m_rpcId, result);
//        UBChain::getInstance()->rpcReceivedOrNotMapSetValue(m_rpcId, true);

        m_buff.clear();
        m_rpcId.clear();
        busy = false;
    }
}
