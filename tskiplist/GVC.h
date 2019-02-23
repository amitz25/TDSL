#pragma once

#include "Utils.h"

class GVC
{
public:
    GVC() : version(0) {}

    virtual ~GVC() = default;

    unsigned int read() const
    {
        return version.load();
    }

    unsigned int addAndFetch()
    {
        return std::atomic_fetch_add<unsigned int>(&version, 1) + 1;
    }

private:
    std::atomic<unsigned int> version;
};
