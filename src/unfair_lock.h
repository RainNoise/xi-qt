#ifndef UNFAIRLOCK_H
#define UNFAIRLOCK_H

#include <QMutex>

#include <memory>


namespace xi {

class UnfairLock {
	friend class UnfairLocker;

public:
	UnfairLock();

	void lock();
	void unlock();
	bool tryLock();

protected:
	std::unique_ptr<QMutex> m_lock;
};


class UnfairLocker {
public:
	UnfairLocker(UnfairLock &mutex) {
		m_mutex = &mutex;
		m_mutex->lock();
	}
	~UnfairLocker() {
		m_mutex->unlock();
	}
protected:
	UnfairLock* m_mutex;
};

} // xi

#endif // UNFAIRLOCK_H