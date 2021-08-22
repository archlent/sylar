#include <memory>
#include <vector>
#include "mutex.h"
#include "iomanager.h"
#include "log.h"

namespace sylar {
    class FdCtx : public std::enable_shared_from_this<FdCtx> {
    public:
        FdCtx(int fd);
        ~FdCtx();

        bool init();
        bool isInit() const {return m_isInit; }
        bool isSocket() const { return m_isSocket; }
        bool isClose() const {return m_isClosed; } 

        void setUserNonblock(bool v) { m_useNonblock = v; }
        bool getUserNonblock() const { return m_useNonblock; }

        void setSysNonblock(bool v) { m_sysNonblock = v; }
        bool getSysNonblock() const { return m_sysNonblock; } 

        void setTimeout(int type, uint64_t v);
        uint64_t getTimeout(int type);
    private:
        bool m_isInit : 1;
        bool m_isSocket : 1;
        bool m_sysNonblock : 1;
        bool m_useNonblock : 1;
        bool m_isClosed : 1;
        int m_fd;

        uint64_t m_recvTimeOut;
        uint64_t m_sendTimeOut;
    };

    class FdManager {
    public:
        using RWMutexType = shared_mutex;
        FdManager();
        ptr<FdCtx> get(int fd, bool auto_create = false);
        void del(int fd);
    private:
        mutable RWMutexType m_mutex;
        std::vector<ptr<FdCtx>> m_datas;

    };

    using FdMsg = Singleton<FdManager>;
}