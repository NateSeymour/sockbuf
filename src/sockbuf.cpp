#include <sys/un.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "../include/nys/sockbuf.h"

using namespace nys;

sockbuf sockbuf::UnixSocket(const std::filesystem::path& path, SockMode mode, uint32_t options)
{
    sockbuf socket;

    socket.props.options = options;
    socket.props.mode = mode;
    socket.props.domain = PF_LOCAL;
    socket.props.type = SOCK_STREAM;

    if(path.string().length() > 103)
    {
        throw std::runtime_error("Socket path too long!");
    }

    socket.props.size = sizeof(sockaddr_un);
    socket.props.address = std::shared_ptr<struct sockaddr>(static_cast<struct sockaddr*>(malloc(socket.props.size)), free);

    // Set the socket address
    memset(socket.props.address.get(), 0, sizeof(sockaddr_un));
    ((sockaddr_un*)socket.props.address.get())->sun_family = socket.props.domain;

    socket.props.length = strlen(path.c_str()) + 1;
    ((sockaddr_un*)socket.props.address.get())->sun_len = socket.props.length;

    memcpy(((sockaddr_un*)socket.props.address.get())->sun_path, path.c_str(), socket.props.length);

    // Create the socket with `socket()`
    socket.Create();

    return socket;
}

void sockbuf::Connect()
{
    if(connect(this->fd_, (struct sockaddr*)this->props.address.get(), this->props.size) != 0)
    {
        throw socket_error(this);
    }
}

void sockbuf::Create()
{
    // Create socket
    this->fd_ = socket(this->props.domain, this->props.type, this->props.protocol);
    if(this->fd_ == -1)
    {
        throw socket_error(this);
    }

    // socket options
    if(this->props.options & (uint32_t)SockOpts::NonBlocking)
    {
        fcntl(this->fd_, F_SETFL, O_NONBLOCK);
    }

    // Bind and listen or connect, depending on the socket type
    switch(this->props.mode)
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
    if(bind(this->fd_, this->props.address.get(), this->props.size) != 0)
    {
        throw socket_error(this);
    }

    this->props.bound = true;
}

void sockbuf::Listen()
{
    // Start listening
    if(listen(this->fd_, 10) != 0)
    {
        throw socket_error(this);
    }
}

sockbuf sockbuf::Accept()
{
    sockbuf socket;

    socket.props = this->props;
    socket.fd_ = accept(this->fd_, nullptr, nullptr);

    return socket;
}

std::streamsize sockbuf::xsputn(const char *s, std::streamsize n)
{
    std::streamsize total_bytes_written = 0;
    while(total_bytes_written < n)
    {
        ssize_t bytes_written = write(this->fd_, s + total_bytes_written, n - total_bytes_written);

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
    ssize_t bytes_written = write(this->fd_, &casted_ch, sizeof(char));

    if(bytes_written == -1)
    {
        throw socket_error(this);
    }

    return ch;
}

std::streamsize sockbuf::showmanyc()
{
    return ioctl(this->fd_, FIONREAD);
}

std::streamsize sockbuf::xsgetn(char *s, std::streamsize n)
{
    return read(this->fd_, s, n);
}

int sockbuf::underflow()
{
    char underflow_ch = 0;
    ssize_t bytes_read = recv(this->fd_, &underflow_ch, sizeof(char), MSG_PEEK | MSG_DONTWAIT);

    if(bytes_read == -1)
    {
        throw socket_error(this);
    }

    return underflow_ch;
}

int sockbuf::uflow()
{
    char underflow_ch = 0;
    ssize_t bytes_read = read(this->fd_, &underflow_ch, sizeof(char));

    if(bytes_read == -1)
    {
        throw socket_error(this);
    }

    return underflow_ch;
}
