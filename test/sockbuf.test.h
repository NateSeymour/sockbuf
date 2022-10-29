#ifndef SOCKBUF_SOCKBUF_TEST_H
#define SOCKBUF_SOCKBUF_TEST_H

#include <gtest/gtest.h>
#include <filesystem>
#include <cstdio>
#include <sys/syslimits.h>
#include "sockbuf.h"

class UnixSocketTest : public ::testing::Test
{
protected:
    std::filesystem::path socket_path;

    void SetUp() override
    {
        char path[PATH_MAX];
        std::tmpnam(path);

        this->socket_path = path;
    }

    void TearDown() override
    {
        std::filesystem::remove(this->socket_path);
    }
};

#endif //SOCKBUF_SOCKBUF_TEST_H
