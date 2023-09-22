#pragma once

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>

#include <pthread.h>


static void pinThread(int cpu) {
    if (cpu < 0) {
        return;
    }
    ::cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (::pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == -1) {
        std::perror("pthread_setaffinity_rp");
        std::exit(EXIT_FAILURE);
    }
}

template<typename T>
struct isRigtorp : std::false_type {};

template<typename T>
auto bench(char const* name, long iters, int cpu1, int cpu2) {
    using namespace std::chrono_literals;
    using value_type = typename T::value_type;

    constexpr auto fifoSize = 131072;

    T q(fifoSize);
    auto t = std::jthread([&] {
        pinThread(cpu1);
        for (auto i = value_type{}; i < iters; ++i) {

            value_type val;
            if constexpr(isRigtorp<T>::value) {
                while (!q.front());
                val = *q.front();
                q.pop();
            } else {
                while (not q.pop(val)) {
                    ;
                }
            }

            if (val != i) {
                throw std::runtime_error("invalid value");
            }
        }
    });

    pinThread(cpu2);
    auto start = std::chrono::steady_clock::now();
    for (auto i = value_type{}; i < iters; ++i) {
        if constexpr(isRigtorp<T>::value) {
            while (not q.try_push(i)) {
                ;
            }

        } else {
            while (not q.push(i)) {
                ;
            }
        }
    }
    while (not q.empty()) {
      ;
    }
    auto stop = std::chrono::steady_clock::now();
    auto delta = stop - start;
    return (iters * 1s)/delta;
}

template<template<typename> class FifoT>
void bench(char const* name, int argc, char* argv[]) {
    int cpu1 = 1;
    int cpu2 = 2;
    if (argc == 3) {
       cpu1 = std::atoi(argv[1]);
       cpu2 = std::atoi(argv[2]);
    }

    constexpr auto iters = 400'000'000l;
    // constexpr auto iters = 100'000'000l;

    using value_type = std::int64_t;

    // warmup
    bench<FifoT<value_type>>(name, 1'000'000, cpu1, cpu2);

    auto opsPerSec = bench<FifoT<value_type>>(name, iters, cpu1, cpu2);
    std::cout << std::setw(7) << std::left << name << ": "
        << std::setw(10) << std::right << opsPerSec << " ops/s\n";
}
