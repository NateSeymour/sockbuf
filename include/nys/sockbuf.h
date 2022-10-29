#ifndef WHISPER_IPC_SOCKBUF_H
#define WHISPER_IPC_SOCKBUF_H

#include <filesystem>
#include <cstdlib>
#include <memory>
#include <streambuf>
#include <exception>
#include <sys/socket.h>
#include <cerrno>

namespace nys
{
    enum class SockOpts : uint32_t
    {
        NonBlocking = 1
    };

    enum class SockMode
    {
        Client,
        Server
    };

    struct SocketProperties
    {
        std::shared_ptr<struct sockaddr> address;
        bool bound = false;
        size_t size = 0;
        size_t length = 0;
        int domain = 0;
        int type = 0;
        int protocol = 0; // Internet protocol

        uint32_t options = 0;
        SockMode mode = SockMode::Client;
    };

    class sockbuf : public std::streambuf
    {
    private:
        sockbuf() = default;
        void Create();
        void Bind();
        void Connect();
        void Listen();

        int fd_ = -1;

    protected:
        SocketProperties props;

        // Output operations
        std::streamsize xsputn(const char_type* s, std::streamsize n) override;
        int_type overflow(int_type ch) override;

        // Input operations
        std::streamsize showmanyc() override;
        std::streamsize xsgetn(char* s, std::streamsize n) override;
        int underflow() override;
        int uflow() override;

    public:
        [[nodiscard]] int fd() const
        {
            return this->fd_;
        }

        static sockbuf UnixSocket(const std::filesystem::path& path, SockMode mode = SockMode::Client, uint32_t options = 0);

        sockbuf Accept();

        sockbuf(const sockbuf& other) : props(other.props), fd_(dup(other.fd_)) {}
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
