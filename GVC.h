#pragma once

#include "Utils.h"

class GVC
{
public:
    GVC() : version(0) {}

    virtual ~GVC() {}

    unsigned int read() const
    {
        return version.load();
    }

    unsigned int addAndFetch()
    {
        return std::atomic_fetch_add(&version, 1) + 1;
    }

private:
    std::atomic<unsigned int> version;
};
