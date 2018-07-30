#ifndef CORECONNECTION_H
#define CORECONNECTION_H

#include <QBuffer>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QThread>
#include <QVector>

#include "theme.h"
#include "unfair_lock.h"

//#define ENABLE_IO_THREADS

#ifdef ENABLE_IO_THREADS
#include "boost/lockfree/spsc_queue.hpp"
#endif

namespace xi {

class ResponseHandler {
public:
    using Callback = std::function<void(const QJsonObject &)>;
    explicit ResponseHandler(Callback callback = nullptr);
    ResponseHandler(const ResponseHandler &handler);

    void invoke(const QJsonObject &json);
    ResponseHandler &operator=(const ResponseHandler &handler);

private:
    Callback m_callback;
};

class CoreConnection : public QObject {
    Q_OBJECT

    friend class ReadCoreWorker;
    friend class WriteCoreWorker;

public:
#ifdef ENABLE_IO_THREADS
    using CoreQueue = boost::lockfree::spsc_queue<QJsonObject, boost::lockfree::capacity<1024>>;
#endif

    explicit CoreConnection(QObject *parent = nullptr);
    ~CoreConnection();
    void init();

private:
    void startCorePipeThread();

public:
    void sendNotification(const QString &method, const QJsonObject &params);
    void sendRequest(const QString &method, const QJsonObject &params, const ResponseHandler &handler);
    void sendEdit(const QString &viewId, const QString &method, const QJsonObject &params);
    void sendEditArray(const QString &viewId, const QString &method, const QJsonArray &params);
    void sendEditRequest(const QString &viewId, const QString &method, const QJsonObject &params, const ResponseHandler &handler);
    void sendClientStarted(const QString &configDir, const QString &clientExtrasDir);
    void sendNewView(const QString &filePath, const ResponseHandler &handler);
    void sendCloseView(const QString &viewId);
    void sendPaste(const QString &viewId, const QString &chars);
    void sendInsert(const QString &viewId, const QString &chars);
    void sendCopy(const QString &viewId, const ResponseHandler &handler);
    void sendCut(const QString &viewId, const ResponseHandler &handler);
    void sendSave(const QString &viewId, const QString &filePath);
    void sendSetTheme(const QString &themeName);
    void sendScroll(const QString &viewId, qint64 firstLine, qint64 lastLine);
    void sendClick(const QString &viewId, qint64 line, qint64 column, qint64 modifiers, qint64 clickCount);
    void sendDrag(const QString &viewId, qint64 line, qint64 column, qint64 modifiers);
    void sendGesture(const QString &viewId, qint64 line, qint64 col, const QString &ty);
    void sendFind(const QString &viewId, const QString &chars, bool caseSensitive, const ResponseHandler &handler);
    void sendFindNext(const QString &viewId, bool wrapAround, bool allowSame);
    void sendFindPrevious(const QString &viewId, bool wrapAround);

    // QJsonObject sendRpc(const QString &method, const QJsonObject &params);
    // void sendRpcAsync();

private:
    void handleRawInner(const QByteArray &bytes);
    void handleRaw(const QByteArray &bytes);
    void handleRpc(const QJsonObject &json);
    void handleRequest(const QJsonObject &json);
    void handleNotification(const QJsonObject &json);
    void sendJson(const QJsonObject &json);

signals:
    void updateReceived(const QString &viewId, const QJsonObject &update);
    void scrollReceived(const QString &viewId, int line, int column);
    void defineStyleReceived(const QJsonObject &params);
    void pluginStartedReceived(const QString &viewId, const QString &pluginName);
    void pluginStoppedReceived(const QString &viewId, const QString &pluginName);
    void availableThemesReceived(const QStringList &themes);
    void themeChangedReceived(const QString &name, const QJsonObject &json);
    void availablePluginsReceived(const QString &viewId, const QJsonObject &plugins);
    void updateCommandsReceived(const QString &viewId, const QStringList &commands);
    void configChangedReceived(const QString &viewId, const QJsonObject &changes);
    void alertReceived(const QString &text);

    void jsonQueued();

public slots:
    void stdoutReceivedHandler();
    void stderrReceivedHandler();

private:
    std::shared_ptr<QProcess> m_process;
    QMutex m_processMutex;

    QHash<qint64, ResponseHandler> m_pending;
    qint64 m_rpcIndex;
    std::shared_ptr<QBuffer> m_recvBuf;

#ifdef ENABLE_IO_THREADS
    std::shared_ptr<CoreQueue> m_writeQueue;
    QThread m_readCoreThread;
    QThread m_writeCoreThread;
#endif
};

#ifdef ENABLE_IO_THREADS
class WriteCoreWorker : public QObject {
    Q_OBJECT
public:
    WriteCoreWorker(const std::shared_ptr<QProcess> &process, const std::shared_ptr<CoreConnection::CoreQueue> &queue, CoreConnection *connection) {
        setObjectName("WriteCoreWorker");
        m_process = process;
        m_queue = queue;
        m_connection = connection;
    }

    void doWork();

private:
    CoreConnection *m_connection;
    std::shared_ptr<QProcess> m_process;
    std::shared_ptr<CoreConnection::CoreQueue> m_queue;
};

class ReadCoreWorker : public QObject {
    Q_OBJECT
public:
    ReadCoreWorker(const std::shared_ptr<QProcess> &process, const std::shared_ptr<CoreConnection::CoreQueue> &queue, CoreConnection *connection) {
        setObjectName("WriteCoreWorker");
        m_process = process;
        m_queue = queue;
        m_connection = connection;
        connect(m_process.get(), &QProcess::readyReadStandardOutput, this, &ReadCoreWorker::doWork);
    }

    void doWork();

private:
    CoreConnection *m_connection;
    std::shared_ptr<QProcess> m_process;
    std::shared_ptr<CoreConnection::CoreQueue> m_queue;
};
#endif

} // namespace xi

#endif // CORECONNECTION_H
