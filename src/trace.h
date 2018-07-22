#ifndef TRACE_H
#define TRACE_H

#include <QString>
#include <QHash>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QThread>
#include <QCoreApplication>
#include <QJsonDocument>

#include <chrono>
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>

#include "unfair_lock.h"


namespace xi {

enum class TraceCategory {
	Main,
	Rpc,
};
enum class TracePhase {
	Begin,
	End,
	Instant,
};

QString to_string(TraceCategory tc);
QString to_string(TracePhase tp);

struct TraceEntry {
	QString name;
	TraceCategory cat;
	TracePhase ph;
	qint64 abstime;
	qint64 tid;
	QString thread_name;
};

class Trace {
public:
	static Trace* shared();

	bool isEnabled();
	void setEnabled(bool enabled);	
	void trace(const QString& name, TraceCategory cat, TracePhase ph);
	QJsonDocument json();
	QJsonDocument snapshot();

private:
	UnfairLock m_mutex;
	const int BUF_SIZE = 100'000;
	std::unique_ptr<std::vector<TraceEntry>> m_buf;
	int m_n_entries = 0;
	bool m_enabled = false;

	Trace();
};

} // xi

#endif // TRACE_H