#ifndef UNFAIR_LOCK_H
#define UNFAIR_LOCK_H

#include <QMutex>

#include <memory>

namespace xi {

class UnfairLock {
public:
    UnfairLock();

    void lock();
    void unlock();
    bool tryLock();

protected:
    std::unique_ptr<QMutex> m_lock;
};

} // namespace xi

#endif // UNFAIR_LOCK_H