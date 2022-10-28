#ifndef WHISPER_IPC_SOCKBUF_H
#define WHISPER_IPC_SOCKBUF_H

#include <filesystem>
#include <cstdlib>
#include <memory>
#include <streambuf>
#include <exception>
#include <sys/socket.h>
#include <cerrno>

namespace nys::whisper
{
    class SocketAddress
    {

    };

    class sockbuf : public std::streambuf
    {
    private:
        sockbuf() = default;

        static std::shared_ptr<sockbuf> Empty();

    protected:
        // Output operations
        std::streamsize xsputn(const char_type* s, std::streamsize n) override;
        int_type overflow(int_type ch) override;

        // Input operations
        std::streamsize showmanyc() override;
        std::streamsize xsgetn(char* s, std::streamsize n) override;
        int underflow() override;
        int uflow() override;

    public:
        std::shared_ptr<struct sockaddr> address;
        bool bound = false;
        size_t size = 0;
        size_t length = 0;
        int domain = 0;
        int type = 0;
        int protocol = 0; // Internet protocol
        int fd = -1;

        static std::shared_ptr<sockbuf> UnixSocket(const std::filesystem::path& path);

        std::shared_ptr<sockbuf> Accept();
        void Bind();
        void Connect();
        void Create();
        void Listen();

        ~sockbuf();
    };

    class socket_error : public std::exception
    {
    private:
        const sockbuf* socket = nullptr;

    public:
        explicit socket_error(const sockbuf* socket) : socket(socket) {}

        [[nodiscard]] const char* what() const noexcept override
        {
            return strerror(errno);
        }
    };
}


#endif //WHISPER_IPC_SOCKBUF_H
