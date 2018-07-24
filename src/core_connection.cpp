#include "core_connection.h"

#include <QFuture>

//! Startup and communication with the xi core thread.
//! TODO
//! 1. multi thread, queue, async

namespace xi {

CoreConnection::CoreConnection(QObject *parent) : QObject(parent) {
    m_rpcIndex = 0;
}

CoreConnection::~CoreConnection() {
}

void CoreConnection::init() {
    m_recvBuf = std::make_unique<QBuffer>();
    m_recvBuf->open(QBuffer::ReadWrite);

    //m_queue = std::make_shared<boost::lockfree::queue<QJsonObject>>();

    m_process.reset(new QProcess, [](QProcess *p) { p->close(); delete p; });
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    // QObject::connect: Parentheses expected, signal xi::ReadXiCoreThread::finished in e:\sw5cc\newxi\xi-qt\src\coreconnection.cpp:59
    //startCorePipeThread();
    connect(m_process.get(), &QProcess::readyReadStandardOutput, this, &CoreConnection::stdoutReceivedHandler);

    m_process->start("xi-core");
    m_process->waitForStarted();
}

void CoreConnection::startCorePipeThread() {
    ReadCoreStdoutThread *readThread = new ReadCoreStdoutThread(m_process);
    connect(readThread, &ReadCoreStdoutThread::resultReady, this, &CoreConnection::stdoutReceivedHandler);
    connect(readThread, &ReadCoreStdoutThread::finished, readThread, &ReadCoreStdoutThread::deleteLater);
    readThread->start();

    //WriteCoreStdinThread *writeThread = new WriteCoreStdinThread(m_process, m_queue);
    //writeThread->start();
    //connect(writeThread, &WriteCoreStdinThread::finished, writeThread, &WriteCoreStdinThread::deleteLater);
}

static QHash<QString, CoreConnection::NotificationType> notificationMap;

CoreConnection::NotificationType CoreConnection::notification(QString name) {
    if (notificationMap.size() == 0) {
        notificationMap["update"] = Notification_Update;
        notificationMap["scroll_to"] = Notification_ScrollTo;
        notificationMap["def_style"] = Notification_DefStyle;
        notificationMap["plugin_started"] = Notification_PluginStarted;
        notificationMap["plugin_stopped"] = Notification_PluginStopped;
        notificationMap["available_themes"] = Notification_AvailableThemes;
        notificationMap["theme_changed"] = Notification_ThemeChanged;
        notificationMap["available_plugins"] = Notification_AvailablePlugins;
        notificationMap["update_cmds"] = Notification_UpdateCmds;
        notificationMap["config_changed"] = Notification_ConfigChanged;
        notificationMap["alert"] = Notification_Alert;
    }
    return notificationMap.value(name, Notification_Unknown);
}

static QHash<QString, CoreConnection::OpsType> opsMap;

CoreConnection::OpsType CoreConnection::ops(QString name) {
    if (opsMap.size() == 0) {
        opsMap["invalidate"] = Ops_Invalidate;
        opsMap["ins"] = Ops_Ins;
        opsMap["copy"] = Ops_Copy;
        opsMap["update"] = Ops_Update;
        opsMap["skip"] = Ops_Skip;
    }
    return opsMap.value(name, Ops_Unknown);
}

void CoreConnection::sendNotification(const QString &method, const QJsonObject &params) {
    QJsonObject object;
    object["method"] = method;
    object["params"] = params;
    sendJson(object);
}

void CoreConnection::sendRequest(const QString &method, const QJsonObject &params, const ResponseHandler &handler) {
    m_pending[m_rpcIndex] = handler;
    QJsonObject object;
    object["id"] = m_rpcIndex++;
    object["method"] = method;
    object["params"] = params;
    sendJson(object);
}

void CoreConnection::sendEdit(const QString &viewId, const QString &method, const QJsonObject &params) {
    QJsonObject object;
    object["method"] = method;
    object["view_id"] = viewId;
    object["params"] = params;
    sendNotification("edit", object);
}

void CoreConnection::sendEditArray(const QString &viewId, const QString &method, const QJsonArray &params) {
    QJsonObject object;
    object["method"] = method;
    object["view_id"] = viewId;
    object["params"] = params;
    sendNotification("edit", object);
}

void CoreConnection::sendEditRequest(
    const QString &viewId, const QString &method, const QJsonObject &params, const ResponseHandler &handler) {
    QJsonObject object;
    object["method"] = method;
    object["view_id"] = viewId;
    object["params"] = params;
    sendRequest("edit", object, handler);
}

void CoreConnection::sendClientStarted(const QString &configDir, const QString &clientExtrasDir) {
    QJsonObject object;
    object["config_dir"] = configDir;
    object["client_extras_dir"] = clientExtrasDir;
    sendNotification("client_started", object);
}

void CoreConnection::sendNewView(const QString &filePath, const ResponseHandler &handler) {
    QJsonObject object;
    if (!filePath.isEmpty()) {
        object["file_path"] = filePath;
    }
    sendRequest("new_view", object, handler);
}

void CoreConnection::sendCloseView(const QString &viewId) {
    QJsonObject object;
    object["view_id"] = viewId;
    sendNotification("close_view", object);
}

void CoreConnection::sendInsert(const QString &viewId, const QString &chars) {
    QJsonObject object;
    object["chars"] = chars;
    sendEdit(viewId, "insert", object);
}

void CoreConnection::sendCopy(const QString &viewId, const ResponseHandler &handler) {
    QJsonObject object;
    sendEditRequest(viewId, "copy", object, handler);
}

void CoreConnection::sendCut(const QString &viewId, const ResponseHandler &handler) {
    QJsonObject object;
    sendEditRequest(viewId, "cut", object, handler);
}

void CoreConnection::sendSave(const QString &viewId, const QString &filePath) {
    QJsonObject object;
    object["view_id"] = viewId;
    object["file_path"] = filePath;
    sendNotification("save", object);
}

void CoreConnection::sendSetTheme(const QString &themeName) {
    QJsonObject object;
    object["theme_name"] = themeName;
    sendNotification("set_theme", object);
}

void CoreConnection::sendScroll(const QString &viewId, int64_t firstLine, int64_t lastLine) {
    QJsonArray object;
    object.append(firstLine);
    object.append(lastLine);
    sendEditArray(viewId, "scroll", object);
}

void CoreConnection::sendClick(const QString &viewId, int64_t line, int64_t column, int64_t modifiers, int64_t clickCount) {
    QJsonArray object;
    object.append(line);
    object.append(column);
    object.append(modifiers);
    object.append(clickCount);
    sendEditArray(viewId, "click", object);
}

void CoreConnection::sendDrag(const QString &viewId, int64_t line, int64_t column, int64_t modifiers) {
    QJsonArray object;
    object.append(line);
    object.append(column);
    object.append(modifiers);
    sendEditArray(viewId, "drag", object);
}

void CoreConnection::sendGesture(const QString &viewId, int64_t line, int64_t col, const QString &ty) {
    QJsonObject object;
    object["line"] = line;
    object["col"] = col;
    object["ty"] = ty;
    sendEdit(viewId, "gesture", object);
}

void CoreConnection::sendFind(const QString &viewId, const QString &chars, bool caseSensitive, const ResponseHandler &handler) {
    QJsonObject object;
    object["chars"] = chars;
    object["case_sensitive"] = caseSensitive;
    sendEditRequest(viewId, "find", object, handler);
}

void CoreConnection::sendFindNext(const QString &viewId, bool wrapAround, bool allowSame) {
    QJsonObject object;
    object["wrap_around"] = wrapAround;
    object["allow_same"] = allowSame;
    sendEdit(viewId, "find_next", object);
}

void CoreConnection::sendFindPrevious(const QString &viewId, bool wrapAround) {
    QJsonObject object;
    object["wrap_around"] = wrapAround;
    sendEdit(viewId, "find_previous", object);
}

//void CoreConnection::stdoutReceivedHandler(const QByteArray &threadBuf) {
//	/// thread
//
//	auto list = threadBuf.split('\n');
//	list.removeLast(); //empty
//
//	foreach(auto &bytesline, list) {
//		qDebug() << bytesline;
//		handleRaw(bytesline);
//	}
//}

// readAllStandardOutput
void CoreConnection::stdoutReceivedHandler() {
    qDebug() << "stdoutReceivedHandler";

    if (m_process->canReadLine()) {
        auto &buf = m_recvBuf->buffer();
        do {
            auto line = m_process->readLine();
            if (!line.endsWith('\n')) {
                buf.append(line);
            } else {
                if (buf.size() != 0) {
                    auto newLine = buf + line;
                    handleRaw(newLine);
                    buf.clear();
                } else {
                    handleRaw(line);
                }
            }
        } while (m_process->bytesAvailable());
    } else {
        auto &buf = m_recvBuf->buffer();

        buf.append(m_process->readAll());
        if (buf.size() == 0) {
            return;
        }

        auto list = buf.split('\n');
        list.removeLast(); //empty

        if (buf.at(buf.size() - 1) != '\n') {
            buf = list.last();
            list.removeLast(); //?
        } else {
            buf.clear();
        }

        foreach (auto &bytesline, list) {
            qDebug() << bytesline;
            handleRaw(bytesline);
        }
    }
}

// Bytes to json
void CoreConnection::handleRaw(const QByteArray &bytes) {
    auto doc = QJsonDocument::fromJson(bytes);
    if (doc.isNull()) {
        qFatal("malformed json %s", bytes);
    }
    auto json = doc.object();
    handleRpc(json);
}

void CoreConnection::handleRpc(const QJsonObject &json) {
    if (json["id"].isDouble()) { // number
        auto index = json["id"].toInt();
        if (json["result"].isString()) { // is response
            QJsonObject result;
            result["result"] = json["result"]; // copy?
            auto it = m_pending.find(index);
            if (it != m_pending.end()) {
                auto handler = it.value();
                m_pending.erase(it);
                if (handler.type() == ResponseHandler::Emit) {
                    result["user"] = handler.getEmitData();
                    emit RpcResponseReady(result);
                } else {
                    handler.invoke(result);
                }
            }
        } else {
            handleRequest(json);
        }
    } else {
        handleNotification(json);
    }
}

void CoreConnection::handleRequest(const QJsonObject &json) {
    // there are currently no core -> client requests in the protocol
    qDebug() << "Unexpected RPC Request: " << json;
}

void CoreConnection::handleNotification(const QJsonObject &json) {
    if (!json.contains("method") || !json.contains("params")) {
        qDebug() << "unknown json from core:" << json;
        return;
    }

    auto method = json["method"].toString();
    QJsonObject params = json["params"].toObject(); // QJsonObject
    auto viewIdentifier = params["view_id"].toString();

    switch (notification(method)) {
    case Notification_Update: {
        auto update = params["update"].toObject();
        emit updateReceived(viewIdentifier, update);
    } break;
    case Notification_ScrollTo: {
        auto line = params["line"].toInt();
        auto column = params["col"].toInt();
        emit scrollReceived(viewIdentifier, line, column);
    } break;
    case Notification_DefStyle: {
        emit defineStyleReceived(params);
    } break;
    case Notification_PluginStarted: {
        auto name = params["plugin"].toString();
        emit pluginStartedReceived(viewIdentifier, name);
    } break;
    case Notification_PluginStopped: {
        auto name = params["plugin"].toString();
        emit pluginStoppedReceived(viewIdentifier, name);
    } break;
    case Notification_AvailableThemes: {
        QVector<QString> themes;
        QJsonArray array = params["themes"].toArray();
        foreach (const QJsonValue &v, array) {
            themes.push_front(v.toString());
        }
        emit availableThemesReceived(themes);
    } break;
    case Notification_ThemeChanged: {
        auto name = params["name"].toString();
        auto themeObj = params["theme"].toObject();
        auto theme = Theme(themeObj);
        emit themeChangedReceived(name, theme);
    } break;
    case Notification_AvailablePlugins: {
        QVector<QJsonObject> plugins;
        QJsonArray array = params["plugins"].toArray();
        foreach (const QJsonValue &v, array) {
            plugins.push_front(v.toObject());
        }
        emit availablePluginsReceived(viewIdentifier, plugins);
    } break;
    case Notification_UpdateCmds: {
        //            auto plugin = params["plugin"].toString();
        //            QVector<QJsonObject> cmds;
        //            QJsonArray array = params["cmds"].toArray();
        //            foreach (const QJsonValue &v, array) {
        //                cmds.push_front(v.toObject());
        //            }
        //            emit updateCommandsReceived(viewIdentifier, line, column);
    } break;
    case Notification_ConfigChanged: {
        //            auto line = params["line"].toInt();
        //            auto column = params["col"].toInt();
        //            emit configChangedReceived(viewIdentifier, line, column);
    } break;
    case Notification_Alert: {
        auto message = params["msg"].toString();
        emit alertReceived(message);
    } break;
    case Notification_Unknown:
    default: {
        qDebug() << "unknown notification: " << method;
    } break;
    }
}

void CoreConnection::sendJson(const QJsonObject &json) {
    qDebug() << "sendJson";
    qDebug() << json;

    //m_queue->bounded_push(json);

    QJsonDocument doc(json);
    QString stream(doc.toJson(QJsonDocument::Compact) + '\n');
    m_process->write(stream.toUtf8());
    m_process->waitForBytesWritten(); // async
}

ResponseHandler::ResponseHandler(CallbackType callback /*= nullptr*/) {
    m_type = Callback;
    m_callback = std::move(callback);
}

ResponseHandler::ResponseHandler(const ResponseHandler &handler) {
    m_callback = std::move(handler.m_callback);
    m_type = handler.m_type;
    m_userData = handler.m_userData;
}

ResponseHandler::ResponseHandler(const QJsonObject &data) {
    setEmitData(data);
}

void ResponseHandler::invoke(const QJsonObject &json) {
    if (m_callback) m_callback(json);
}

ResponseHandler &ResponseHandler::operator=(const ResponseHandler &handler) {
    m_callback = std::move(handler.m_callback);
    m_type = handler.m_type;
    m_userData = handler.m_userData;
    return *this;
}

xi::ResponseHandler::Type ResponseHandler::type() const {
    return m_type;
}

void ResponseHandler::type(Type type) {
    m_type = type;
}

void ResponseHandler::setEmitData(const QJsonObject &data) {
    m_type = Emit;
    m_userData = data;
}

QJsonObject ResponseHandler::getEmitData() const {
    return m_userData;
}

//void WriteCoreStdinThread::run() {
//	while (1) {
//		if (!m_queue) break;
//		QJsonObject json;
//		if (m_queue->pop(json)) {
//			QJsonDocument doc(json);
//			QString stream(doc.toJson(QJsonDocument::Compact) + '\n');
//			m_core->write(stream.toUtf8());
//			m_core->waitForBytesWritten(); // async
//		}
//		msleep(1);
//	}
//}

void ReadCoreStdoutThread::run() {
    while (1) {
        if (!m_core) break;
        if (m_core->canReadLine()) {
            auto line = m_core->readLine();
            emit resultReady(line);
        }
        //if (m_core->bytesAvailable() > 0) {
        //	auto buf = m_readBuffer.buffer();
        //	auto bytes = m_core->readAll();
        //	for (auto b : bytes) {
        //		buf.append(b);
        //		if (b == '\n') {
        //			QByteArray line(buf);
        //			buf.clear();
        //			emit resultReady(line);
        //		}
        //	}
        //}
        msleep(1);
    }
}

} // namespace xi
