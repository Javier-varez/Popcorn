#ifndef UNIQUE_LOCK_H_
#define UNIQUE_LOCK_H_

namespace OS
{
    template<class T>
    class UniqueLock
    {
    public:
        explicit inline UniqueLock(T& mutex) :
            m_mutex(mutex)
        {
            m_mutex.Lock();
        }

        inline ~UniqueLock()
        {
            m_mutex.Unlock();
        }
    private:
        T& m_mutex;
    };
}

#endif