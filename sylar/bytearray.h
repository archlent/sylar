#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>


#include "log.h"
#include "edian.h"

namespace sylar {
    class ByteArray {
    public:

        struct Node {
            Node(size_t s) : ptr(new char[s]), next(nullptr), size(s) {}
            Node() : ptr(nullptr), next(nullptr), size(0) {}
            ~Node() { 
               delete[] ptr;
               ptr = nullptr;
            }

            char* ptr;
            Node* next;
            size_t size;
        };

        ByteArray(size_t base_size = 4096);
        ~ByteArray();
        void writeFint8(int8_t value);
        void writeFuint8(uint8_t value);
        void writeFint16(int16_t value);
        void writeFuint16(uint16_t value);
        void writeFint32(int32_t value);
        void writeFuint32(uint32_t value);
        void writeFint64(int64_t value);
        void writeFuint64(uint64_t value);

        void writeInt32(int32_t value);
        void writeUint32(uint32_t value);
        void writeInt64(int64_t value);
        void writeUint64(uint64_t value);

        void writeFloat(float value);
        void writeDouble(double value);

        void writeStringF16(const std::string& value);
        void writeStringF32(const std::string& value);
        void writeStringF64(const std::string& value);
        void writeStringVint(const std::string& value);
        void writeStringWithoutLength(const std::string& value);

        int8_t readFint8();
        uint8_t readFuint8();
        int16_t readFint16();
        uint16_t readFuint16();
        int32_t readFint32();
        uint32_t readFuint32();
        int64_t readFint64();
        uint64_t readFuint64();

        int32_t readInt32();
        uint32_t readUint32();
        uint64_t readUint64();
        int64_t readInt64();

        float readFloat();
        double readDouble();

        std::string readStringF16();
        std::string readStringF32();
        std::string readStringF64();
        std::string readStringVint();
        
        void clear();
        void write(const void* buf, size_t size);
        void read(void* read, size_t size);
        void read(void* read, size_t size, size_t position) const   ;

        size_t getPosition() const { return m_position; }
        void setPosition(size_t v);

        bool writeToFile(std::string_view name);
        bool readFromFile(std::string_view name);

        size_t getBaseSize() const { return m_baseSize; }
        size_t getReadSize() const { return m_size - m_position; }

        bool isLittleEndian() const { return m_endian == SYLAR_LITTLE_ENDIAN; }
        void setLittleEndian(bool val) { m_endian = val ? SYLAR_LITTLE_ENDIAN : SYLAR_BIG_ENDIAN; }

        std::string toString() const;
        std::string toHexString() const;

        size_t getSize() const { return m_size; }
        uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;
        uint64_t getReadBuffers(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;
        uint64_t getWriteBuffers(std::vector<iovec>& buffers, uint64_t len);
    private:
        void addCapacity(size_t size);
        size_t getCapacity() const { return m_capacity - m_position; }

    private:
        size_t m_baseSize;                          // 内存块的大小
        size_t m_position;                          // 当前操作的位置
        size_t m_capacity;                          // 当前的总容量
        size_t m_size;                              // 当前的数据的大小
        int8_t m_endian;                            // 字节序
        Node* m_root;                               // 第一个内存指针
        Node* m_cur;                                // 当前操作的内存指针
    };
} // namespace sylar
