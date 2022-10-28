#include <sys/un.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "sockbuf.h"

using namespace nys::whisper;

std::shared_ptr<sockbuf> sockbuf::Empty()
{
    /*
     * Workaround to call the private constructor of `sockbuf` through std::make_shared
     * https://stackoverflow.com/a/25069711
     */
    struct sockbuf_constructor : public sockbuf {};
    return std::make_shared<sockbuf_constructor>();
}

std::shared_ptr<sockbuf> sockbuf::UnixSocket(const std::filesystem::path& path)
{
    auto socket = sockbuf::Empty();

    socket->domain = PF_LOCAL;
    socket->type = SOCK_STREAM;

    if(path.string().length() > 103)
    {
        throw std::runtime_error("Socket path too long!");
    }

    socket->size = sizeof(sockaddr_un);
    socket->address = std::shared_ptr<struct sockaddr>(static_cast<struct sockaddr*>(malloc(socket->size)), free);

    // Set the socket address
    memset(socket->address.get(), 0, sizeof(sockaddr_un));
    ((sockaddr_un*)socket->address.get())->sun_family = socket->domain;

    socket->length = strlen(path.c_str()) + 1;
    ((sockaddr_un*)socket->address.get())->sun_len = socket->length;

    memcpy(((sockaddr_un*)socket->address.get())->sun_path, path.c_str(), socket->length);

    // Create the socket with `socket()`
    socket->Create();

    return socket;
}

void sockbuf::Connect()
{

}

void sockbuf::Create()
{
    // Create socket
    this->fd = socket(this->domain, this->type, this->protocol);
    if(this->fd == -1)
    {
        throw socket_error(this);
    }

    // Make the socket non-blocking
    fcntl(this->fd, F_SETFL, O_NONBLOCK);
}

void sockbuf::Bind()
{
    // Bind to the socket
    if(bind(this->fd, this->address.get(), this->size) != 0)
    {
        throw socket_error(this);
    }

    this->bound = true;
}

void sockbuf::Listen()
{
    // Start listening
    if(listen(this->fd, 10) != 0)
    {
        throw socket_error(this);
    }
}

sockbuf::~sockbuf()
{
    if(this->fd != -1)
    {
        close(this->fd);
    }

    if(this->bound && this->type == PF_LOCAL)
    {
        unlink(this->address->sa_data);
    }
}

std::shared_ptr<sockbuf> sockbuf::Accept()
{
    auto socket = sockbuf::Empty();

    socket->address = this->address;
    socket->size = this->size;
    socket->length = this->length;
    socket->type = this->type;
    socket->domain = this->domain;
    socket->protocol = this->protocol;
    socket->fd = accept(this->fd, nullptr, nullptr);

    return socket;
}

std::streamsize sockbuf::xsputn(const char *s, std::streamsize n)
{
    std::streamsize total_bytes_written = 0;
    while(total_bytes_written < n)
    {
        ssize_t bytes_written = write(this->fd, s + total_bytes_written, n - total_bytes_written);

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
    ssize_t bytes_written = write(this->fd, &casted_ch, sizeof(char));

    if(bytes_written == -1)
    {
        throw socket_error(this);
    }

    return ch;
}

std::streamsize sockbuf::showmanyc()
{
    return ioctl(this->fd, FIONREAD);
}

std::streamsize sockbuf::xsgetn(char *s, std::streamsize n)
{
    return read(this->fd, s, n);
}

int sockbuf::underflow()
{
    char underflow_ch = 0;
    ssize_t bytes_read = recv(this->fd, &underflow_ch, sizeof(char), MSG_PEEK | MSG_DONTWAIT);

    if(bytes_read == -1)
    {
        throw socket_error(this);
    }

    return underflow_ch;
}

int sockbuf::uflow()
{
    char underflow_ch = 0;
    ssize_t bytes_read = read(this->fd, &underflow_ch, sizeof(char));

    if(bytes_read == -1)
    {
        throw socket_error(this);
    }

    return underflow_ch;
}

