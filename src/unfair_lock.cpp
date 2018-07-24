#include "unfair_lock.h"

namespace xi {

UnfairLock::UnfairLock() {
    m_lock = std::make_unique<QMutex>();
}

void UnfairLock::lock() {
    m_lock->lock();
}

void UnfairLock::unlock() {
    m_lock->unlock();
}

bool UnfairLock::tryLock() {
    return m_lock->tryLock();
}

} // namespace xi