# sockbuf 

A simple wrapper to make UNIX socket programming more idiomatic in C++ by providing an interace through `basic_streambuf`.

## Examples

```c++
#include <nys/sockbuf.h>

using namespace nys::whisper;

auto client = sockbuf::UnixSocket("/tmp/comms.sock", SockMode::Client);
```