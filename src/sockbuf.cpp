#include <sys/un.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "sockbuf.h"

using namespace nys::whisper;

sockbuf sockbuf::UnixSocket(const std::filesystem::path& path, SockMode mode, uint32_t options)
{
    sockbuf socket;

    socket.options = options;
    socket.mode = mode;
    socket.domain = PF_LOCAL;
    socket.type = SOCK_STREAM;

    if(path.string().length() > 103)
    {
        throw std::runtime_error("Socket path too long!");
    }

    socket.size = sizeof(sockaddr_un);
    socket.address = std::shared_ptr<struct sockaddr>(static_cast<struct sockaddr*>(malloc(socket.size)), free);

    // Set the socket address
    memset(socket.address.get(), 0, sizeof(sockaddr_un));
    ((sockaddr_un*)socket.address.get())->sun_family = socket.domain;

    socket.length = strlen(path.c_str()) + 1;
    ((sockaddr_un*)socket.address.get())->sun_len = socket.length;

    memcpy(((sockaddr_un*)socket.address.get())->sun_path, path.c_str(), socket.length);

    // Create the socket with `socket()`
    socket.Create();

    return socket;
}

void sockbuf::Connect()
{
    if(connect(this->fd->get(), (struct sockaddr*)this->address.get(), this->size) != 0)
    {
        throw socket_error(this);
    }
}

void sockbuf::Create()
{
    // Create socket
    int sockfd = socket(this->domain, this->type, this->protocol);
    this->fd = std::make_unique<FileDescriptor>(sockfd);
    if(this->fd->get() == -1)
    {
        throw socket_error(this);
    }

    // socket options
    if(this->options & (uint32_t)SockOpts::NonBlocking)
    {
        fcntl(this->fd->get(), F_SETFL, O_NONBLOCK);
    }

    // Bind and listen or connect, depending on the socket type
    switch(this->mode)
    {
        case SockMode::Server:
        {
            this->Bind();
            this->Listen();
            break;
        }

        case SockMode::Client:
        {
            this->Connect();
            break;
        }
    }
}

void sockbuf::Bind()
{
    // Bind to the socket
    if(bind(this->fd->get(), this->address.get(), this->size) != 0)
    {
        throw socket_error(this);
    }

    this->bound = true;
}

void sockbuf::Listen()
{
    // Start listening
    if(listen(this->fd->get(), 10) != 0)
    {
        throw socket_error(this);
    }
}

sockbuf sockbuf::Accept()
{
    auto socket = *this;

    socket.fd = std::make_unique<FileDescriptor>(accept(this->fd->get(), nullptr, nullptr));

    return socket;
}

std::streamsize sockbuf::xsputn(const char *s, std::streamsize n)
{
    std::streamsize total_bytes_written = 0;
    while(total_bytes_written < n)
    {
        ssize_t bytes_written = write(this->fd->get(), s + total_bytes_written, n - total_bytes_written);

        if(bytes_written == -1)
        {
            throw socket_error(this);
        }

        total_bytes_written += bytes_written;
    }

    return total_bytes_written;
}

int sockbuf::overflow(int ch)
{
    char casted_ch = (char)ch;
    ssize_t bytes_written = write(this->fd->get(), &casted_ch, sizeof(char));

    if(bytes_written == -1)
    {
        throw socket_error(this);
    }

    return ch;
}

std::streamsize sockbuf::showmanyc()
{
    return ioctl(this->fd->get(), FIONREAD);
}

std::streamsize sockbuf::xsgetn(char *s, std::streamsize n)
{
    return read(this->fd->get(), s, n);
}

int sockbuf::underflow()
{
    char underflow_ch = 0;
    ssize_t bytes_read = recv(this->fd->get(), &underflow_ch, sizeof(char), MSG_PEEK | MSG_DONTWAIT);

    if(bytes_read == -1)
    {
        throw socket_error(this);
    }

    return underflow_ch;
}

int sockbuf::uflow()
{
    char underflow_ch = 0;
    ssize_t bytes_read = read(this->fd->get(), &underflow_ch, sizeof(char));

    if(bytes_read == -1)
    {
        throw socket_error(this);
    }

    return underflow_ch;
}

