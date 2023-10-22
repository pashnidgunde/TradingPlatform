#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>
#include <iostream>
#include "WireFormat.h"
#include <algorithm>

class MemoryMappedFile {

private:
    char *m_start = nullptr;
    char *m_current = nullptr;
    char *m_toSend = nullptr;
    int m_fd = -1;
    long long N = std::pow(2,10);

public:
    MemoryMappedFile(){
        if (!this->open()) {
            throw "Failed to open file";
        }
        m_current = m_start;
    }

    ~MemoryMappedFile() {
        int err = munmap(m_start,N);
        if (err != 0) {
            std::cerr << "Failed to unmap file";
        }
    }

    bool open() {
        m_fd = ::open("./.orders.bin", O_WRONLY);
        m_start = (char *) mmap(NULL, N * sizeof(Message), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, m_fd, 0);
        m_toSend = m_start;
        if (m_start == MAP_FAILED) {
            return false;
        }
        return true;
    }

    char *& nextReceive() {
        return m_current;
    }

    bool anyMoreToSend() {
        return (m_current != m_start) && (m_toSend < m_current);
    }

    char* & nextToSend() {
        return m_toSend;
    }

    void advanceReceive() {
        m_current += sizeof(Message);
    }

    void advanceSent() {
        m_toSend += sizeof(Message);
    }

};