#pragma once

#include <vector>
#include <unordered_map>
#include <atomic>
#include <mutex>

typedef int ItemType;

class AbortTransactionException : public std::exception
{
};