/*
 * utility functions
 */
#pragma once

// define the uint type even if it should be handled by the compiler
#ifndef uint
typedef unsigned int uint;
#endif

#include <array>
#include <chrono>
#include <climits>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <regex>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <list>
#include <string>
#include <map>

#include "nlohmann/json.hpp"


// #define ENABLE_PROFILING

#ifdef ENABLE_PROFILING
#include <easy/profiler.h>
#define PROFILE_FUNC(color) EASY_FUNCTION(color)
#define PROFILE_BLOCK(name) EASY_BLOCK(name)
#define END_BLOCK() EASY_END_BLOCK
#define THREAD_SCOPE(name) EASY_THREAD_SCOPE(name)
#define START_PROFILING() info(0, verbose, "elapsed:", elapsed_ms(&deadline), "ms\tProfling mode : ON"); profiler::startListen(); EASY_PROFILER_ENABLE; EASY_BLOCK("main")
#define STOP_PROFILING() EASY_END_BLOCK; profiler::stopListen(); profiler::dumpBlocksToFile("code_profiling/profile.prof")
#else
#define PROFILE_FUNC(color)
#define PROFILE_BLOCK(name)
#define END_BLOCK() 
#define THREAD_SCOPE(name)
#define START_PROFILING()
#define STOP_PROFILING()
#endif

using Time = std::chrono::steady_clock;

void info(const int level, const int verbose);

template <typename Head, typename... Tail>
void info(const int level, const int verbose, Head&& head, Tail&&... tail)
{
  if (verbose < level) return;
  std::cout << head;
  info(level, verbose, std::forward<Tail>(tail)...);
}

// time manager
struct Deadline {
  const Time::time_point t_s;
  const double time_limit_ms;

  Deadline(double _time_limit_ms = 0);
  double elapsed_ms() const;
  double elapsed_ns() const;
};

double elapsed_ms(const Deadline* deadline);
double elapsed_ns(const Deadline* deadline);
bool is_expired(const Deadline* deadline);

float get_random_float(std::mt19937* MT, float from = 0, float to = 1);
int get_random_int(std::mt19937* MT, int from = 0, int to = 1);


struct Infos {    // store information
  int loop_count;
  int PIBT_calls;
  int PIBT_calls_active;
  int actions_count;
  int actions_count_active;

  void reset()
  {
    loop_count = 0;
    PIBT_calls = 0;
    PIBT_calls_active = 0;
    actions_count = 0;
    actions_count_active = 0;
  }
};