#ifndef CORECONNECTION_H
#define CORECONNECTION_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QThread>
#include <QBuffer>
#include <QMutex>

//#include "boost/lockfree/queue.hpp"

#include "theme.h"


namespace xi {

class ResponseHandler
{
public:
	using CallbackType = std::function<void(const QJsonObject &)>;
	enum Type {
		Callback,
		Emit,
	};
	explicit ResponseHandler(CallbackType callback = nullptr);
	explicit ResponseHandler(const QJsonObject& data) {
		setUserData(data);
	}

	ResponseHandler(const ResponseHandler &handler);
	void invoke(const QJsonObject &json);
	ResponseHandler& operator=(const ResponseHandler &handler);
	Type type() const {
		return m_type;
	}
	void type(Type type) {
		m_type = type;
	}
	void setUserData(const QJsonObject& data) {
		m_type = Emit;
		m_userData = data;
	}
	QJsonObject getUserData() const {
		return m_userData;
	}

private:
	CallbackType m_callback;
	Type m_type;
	QJsonObject m_userData;
};


class CoreConnection : public QObject
{
	Q_OBJECT
public:
	explicit CoreConnection(QObject *parent = nullptr);
	~CoreConnection();
	void init();

	enum NotificationType {
		Notification_Update = 0,
		Notification_ScrollTo,
		Notification_DefStyle,
		Notification_PluginStarted,
		Notification_PluginStopped,
		Notification_AvailableThemes,
		Notification_ThemeChanged,
		Notification_AvailablePlugins,
		Notification_UpdateCmds,
		Notification_ConfigChanged,
		Notification_Alert,
		Notification_Unknown,
	};
	static NotificationType notification(QString name);

	enum OpsType {
		Ops_Invalidate = 0,
		Ops_Ins,
		Ops_Copy,
		Ops_Update,
		Ops_Skip,
		Ops_Unknown,
	};
	static OpsType ops(QString name);

private:
	void startCorePipeThread();

	// protocal
public:
	void sendNotification(const QString &method, const QJsonObject &params);
	void sendRequest(const QString &method, const QJsonObject &params, const ResponseHandler &handler);
	void sendEdit(const QString &viewId, const QString &method, const QJsonObject &params);
	void sendEditArray(const QString &viewId, const QString &method, const QJsonArray &params);
	void sendEditRequest(const QString &viewId, const QString &method, const QJsonObject &params, const ResponseHandler &handler);
	void sendClientStarted(const QString &configDir, const QString &clientExtrasDir);
	void sendNewView(const QString &filePath, const ResponseHandler &handler);
	void sendCloseView(const QString &viewId);
	void sendInsert(const QString &viewId, const QString &chars);
	void sendCopy(const QString &viewId, const ResponseHandler &handler);
	void sendCut(const QString &viewId, const ResponseHandler &handler);
	void sendSave(const QString &viewId, const QString &filePath);
	void sendSetTheme(const QString &themeName);
	void sendScroll(const QString &viewId, int64_t firstLine, int64_t lastLine);
	void sendClick(const QString &viewId, int64_t line, int64_t column, int64_t modifiers, int64_t clickCount);
	void sendDrag(const QString &viewId, int64_t line, int64_t column, int64_t modifiers);
	void sendGesture(const QString &viewId, int64_t line, int64_t col, const QString &ty);
	void sendFind(const QString &viewId, const QString &chars, bool caseSensitive, const ResponseHandler &handler);
	void sendFindNext(const QString &viewId, bool wrapAround, bool allowSame);
	void sendFindPrevious(const QString &viewId, bool wrapAround);

	// QJsonObject sendRpc(const QString &method, const QJsonObject &params);
	// void sendRpcAsync();

private:
	void handleRaw(const QByteArray &bytes);
	void handleRpc(const QJsonObject &json);
	void handleRequest(const QJsonObject &json);
	void handleNotification(const QJsonObject &json);
	void sendJson(const QJsonObject &json);

	// protocal
signals:
	void updateReceived(const QString &viewId, const QJsonObject &update);
	void scrollReceived(const QString &viewId, int line, int column);
	void defineStyleReceived(const QJsonObject &params);
	void pluginStartedReceived(const QString &viewId, const QString &pluginName);
	void pluginStoppedReceived(const QString &viewId, const QString &pluginName);
	void availableThemesReceived(const QVector<QString> &themes);
	void themeChangedReceived(const QString &name, const Theme &theme);
	void availablePluginsReceived(const QString &viewId, const QVector<QJsonObject> &plugins);
	void updateCommandsReceived(const QString &viewId, const QVector<QString> &commands);
	void configChangedReceived(const QString &viewId, const QJsonObject &changes);
	void alertReceived(const QString &text);

	void RpcResponseReady(const QJsonObject &result);

public slots:
	//void stdoutReceivedHandler(const QByteArray &buf);
	void stdoutReceivedHandler();

private:
	std::shared_ptr<QProcess> m_process;
	//std::shared_ptr<boost::lockfree::queue<QJsonObject>> m_queue;
	QHash<int64_t, ResponseHandler> m_pending;
	int64_t m_rpcIndex;
	std::shared_ptr<QBuffer> m_recvBuf;
};

class ReadCoreStdoutThread : public QThread {
	Q_OBJECT
public:
	ReadCoreStdoutThread(const std::shared_ptr<QProcess> &core) {
		setObjectName("ReadCoreStdoutThread");
		m_core = core;
	}
	void run() override;

signals:
	void resultReady(QByteArray);

public slots:

private:
	std::shared_ptr<QProcess> m_core;
	QBuffer m_readBuffer;
};

//class WriteCoreStdinThread : public QThread {
//	Q_OBJECT
//public:
//	WriteCoreStdinThread(
//		const std::shared_ptr<QProcess> &core,
//		const std::shared_ptr<boost::lockfree::queue<QJsonObject>> &queue) {
//		m_core = core;
//		m_queue = queue;
//	}
//	void run() override;
//
//signals:
//	void resultReady(QByteArray);
//
//public slots:
//
//private:
//	std::shared_ptr<QProcess> m_core;
//	std::shared_ptr<boost::lockfree::queue<QJsonObject>> m_queue;
//};

} // xi

#endif // CORECONNECTION_H
