#include <sys/mman.h>

class MemoryMappedFile {

private:

    char *m_buffer = nullptr;
    size_t m_offset = 0;
    size_t m_size = 0;

public:
    using Offset = size_t;

    MemoryMappedFile(char *buffer, size_t size, Offset offset = 0) :
            m_buffer(buffer),
            m_offset(offset),
            m_size(size) {
    }

    MemoryMappedFile() = delete;

    //int offset() const { return m_offset; }
    //char * buffer_ptr() { return m_buffer; }

    bool has_more() const { return m_offset < m_size; };

    template<typename T>
    void advance() {
        assert(m_offset + sizeof(T) <= m_size);
        m_buffer += sizeof(T);
        m_offset += sizeof(T);
    }

    template<typename T>
    T read() {
        /*
        assert(m_offset + sizeof(T) <= m_size);
        T temp = *(reinterpret_cast<T *>(m_buffer));
        m_buffer += sizeof(T);
        return temp;
        */

        // avoid creating temporary
        advance<T>();
        return *(reinterpret_cast<T *>(m_buffer - sizeof(T)));
    }

    template<typename T>
    T read(Offset offset) {
        assert(m_offset + offset <= m_size);
        return *(reinterpret_cast<T *>(m_buffer + offset));
    }

    void advance(Offset offset) {
        m_buffer += offset;
        m_offset += offset;
    }

    void open() {
        m_fd = open("./OUCHLMM2.incoming.packets", O_RDONLY, S_IRUSR);
        m_file_buffer = (char *) mmap(NULL, m_buffer_size, PROT_READ, MAP_PRIVATE, m_fd, 0);
    }

    int m_fd = -1;
    char *m_file_buffer = nullptr;
};