#include "fd_manager.h"
#include "hook.h"
#include "singleton.h"

#include <sys/stat.h>


namespace sylar {
    FdCtx::FdCtx(int fd) : m_isInit(false), m_isSocket(false), m_sysNonblock(false), m_useNonblock(false),
            m_isClosed(false), m_fd(fd), m_recvTimeOut(-1), m_sendTimeOut(-1) {
        bool flag = init();
        assert(flag);
    }

    FdCtx::~FdCtx() = default;

    bool FdCtx::init() {
        if (m_isInit) {
            return true;
        }
        m_recvTimeOut = -1;
        m_sendTimeOut = -1;
        struct stat fd_stat;
        if (-1 == fstat(m_fd, &fd_stat)) {
            m_isInit = false;
            m_isSocket = false;
        } else {
            m_isInit = true;
            m_isSocket = S_ISSOCK(fd_stat.st_mode);
        }
        if (m_isSocket) {
            int flags = fcntl_f(m_fd, F_GETFL, 0);
            if (!(flags & O_NONBLOCK)) {
                fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
            }
            m_sysNonblock = true;
        } else {
            m_sysNonblock = false;
        }
        m_useNonblock = false;
        m_isClosed = false;
        return m_isInit;
    }

    void FdCtx::setTimeout(int type, uint64_t v) {
        if (type == SO_RCVTIMEO) {
            m_recvTimeOut = v;
        } else {
            m_sendTimeOut = v;
        }
    }
    uint64_t FdCtx::getTimeout(int type) {
        return type == SO_RCVTIMEO ? m_recvTimeOut : m_sendTimeOut;
    }


    FdManager::FdManager() {
        m_datas.resize(64);
    }

    ptr<FdCtx> FdManager::get(int fd, bool auto_create) {
        if (fd == -1) {
            return nullptr;
        }
        ReadLock lock(m_mutex);
        if ((int)m_datas.size() <= fd) {
            if (!auto_create) {
                return nullptr;
            }
        } else {
            if (m_datas[fd] || !auto_create) {
                return m_datas[fd];
            }
        }
        lock.unlock();
        WriteLock lock2(m_mutex);
        ptr<FdCtx> ctx(new FdCtx(fd));
        m_datas[fd] = ctx;
        return ctx;
    }

    void FdManager::del(int fd) {
        WriteLock lock(m_mutex);
        if((int)m_datas.size() <= fd) {
            return;
        }
        m_datas[fd].reset();
    }
} // namespace sylar