// test/minunit.h
#ifndef MINUNIT_MINUNIT_H
#define MINUNIT_MINUNIT_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#include <Windows.h>
#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#define __func__ __FUNCTION__
#endif

#elif defined(__unix__) || defined(__unix) || defined(unix) ||                 \
    (defined(__APPLE__) && defined(__MACH__))

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE < 200112L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif

#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

#if defined(__MACH__) && defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#if __GNUC__ >= 5 && !defined(__STDC_VERSION__)
#define __func__ __extension__ __FUNCTION__
#endif

#else
#error "Unable to define timers for an unknown OS."
#endif

#define COLORIZE_SHORT_NAMES
#include <colorize.h>

#include <math.h>
#include <stdio.h>

#define MINUNIT_MESSAGE_LEN 1024
#define MINUNIT_EPSILON 1E-12

static int minunit_run = 0;
static int minunit_assert = 0;
static int minunit_fail = 0;
static int minunit_status = 0;

static double minunit_real_timer = 0;
static double minunit_proc_timer = 0;

static char minunit_last_message[MINUNIT_MESSAGE_LEN];

static void (*minunit_setup)(void) = NULL;
static void (*minunit_teardown)(void) = NULL;

#define MU_TEST(method_name) static void method_name(void)
#define MU_TEST_SUITE(suite_name) static void suite_name(void)

#define MU__SAFE_BLOCK(block)                                                  \
  do {                                                                         \
    block                                                                      \
  } while (0)

#define MU_RUN_SUITE(suite_name)                                               \
  MU__SAFE_BLOCK(suite_name(); minunit_setup = NULL; minunit_teardown = NULL;)

#define MU_SUITE_CONFIGURE(setup_fun, teardown_fun)                            \
  MU__SAFE_BLOCK(minunit_setup = setup_fun; minunit_teardown = teardown_fun;)

#define MU_RUN_TEST(test)                                                      \
  MU__SAFE_BLOCK(                                                              \
      if (minunit_real_timer == 0 && minunit_proc_timer == 0) {                \
        minunit_real_timer = mu_timer_real();                                  \
        minunit_proc_timer = mu_timer_cpu();                                   \
      } if (minunit_setup) (*minunit_setup)();                                 \
      minunit_status = 0; test(); minunit_run++; if (minunit_status) {         \
        minunit_fail++;                                                        \
        (void)colorize_printf(FG_RED | BOLD, "F");                             \
        (void)colorize_printf(FG_RED, "\n%s\n", minunit_last_message);         \
      }(void)fflush(stdout);                                                   \
      if (minunit_teardown)(*minunit_teardown)();)

#define MU_REPORT()                                                            \
  MU__SAFE_BLOCK(                                                              \
      double minunit_end_real_timer; double minunit_end_proc_timer;            \
      colorize_style_t status_style = minunit_fail > 0 ? FG_RED | BOLD : FG_GREEN | BOLD; \
      (void)colorize_printf(status_style, "\n\n%d tests, %d assertions, %d failures\n", minunit_run, \
             minunit_assert, minunit_fail);                                    \
      minunit_end_real_timer = mu_timer_real();                                \
      minunit_end_proc_timer = mu_timer_cpu();                                 \
      (void)colorize_printf(DIM, "\nFinished in %.8f seconds (real) %.8f seconds (proc)\n\n", \
             minunit_end_real_timer - minunit_real_timer,                      \
             minunit_end_proc_timer - minunit_proc_timer);)
#define MU_EXIT_CODE minunit_fail

#define mu_check(test)                                                         \
  MU__SAFE_BLOCK(                                                              \
      minunit_assert++; if (!(test)) {                                         \
        (void)snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN,              \
                       "%s failed:\n\t%s:%d: %s", __func__, __FILE__,          \
                       __LINE__, #test);                                       \
        minunit_status = 1;                                                    \
        return;                                                                \
      } else { (void)colorize_printf(FG_GREEN, "."); })

#define mu_fail(message)                                                       \
  MU__SAFE_BLOCK(minunit_assert++;                                             \
                 (void)snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN,     \
                                "%s failed:\n\t%s:%d: %s", __func__, __FILE__, \
                                __LINE__, message);                            \
                 minunit_status = 1; return;)

#define mu_assert(test, message)                                               \
  MU__SAFE_BLOCK(                                                              \
      minunit_assert++; if (!(test)) {                                         \
        (void)snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN,              \
                       "%s failed:\n\t%s:%d: %s", __func__, __FILE__,          \
                       __LINE__, message);                                     \
        minunit_status = 1;                                                    \
        return;                                                                \
      } else { (void)colorize_printf(FG_GREEN, "."); })

#define mu_assert_int_eq(expected, result)                                     \
  MU__SAFE_BLOCK(                                                              \
      int minunit_tmp_e; int minunit_tmp_r; minunit_assert++;                  \
      minunit_tmp_e = (expected); minunit_tmp_r = (result);                    \
      if (minunit_tmp_e != minunit_tmp_r) {                                    \
        (void)snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN,              \
                       "%s failed:\n\t%s:%d: %d expected but was %d",          \
                       __func__, __FILE__, __LINE__, minunit_tmp_e,            \
                       minunit_tmp_r);                                         \
        minunit_status = 1;                                                    \
        return;                                                                \
      } else { (void)colorize_printf(FG_GREEN, "."); })

#define mu_assert_double_eq(expected, result)                                  \
  MU__SAFE_BLOCK(                                                              \
      double minunit_tmp_e; double minunit_tmp_r; minunit_assert++;            \
      minunit_tmp_e = (expected); minunit_tmp_r = (result);                    \
      if (fabs(minunit_tmp_e - minunit_tmp_r) > MINUNIT_EPSILON) {             \
        int minunit_significant_figures = 1 - log10(MINUNIT_EPSILON);          \
        (void)snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN,              \
                       "%s failed:\n\t%s:%d: %.*g expected but was %.*g",      \
                       __func__, __FILE__, __LINE__,                           \
                       minunit_significant_figures, minunit_tmp_e,             \
                       minunit_significant_figures, minunit_tmp_r);            \
        minunit_status = 1;                                                    \
        return;                                                                \
      } else { (void)colorize_printf(FG_GREEN, "."); })

#define mu_assert_string_eq(expected, result)                                  \
  MU__SAFE_BLOCK(                                                              \
      const char *minunit_tmp_e = expected;                                    \
      const char *minunit_tmp_r = result; minunit_assert++;                    \
      if (!minunit_tmp_e) {                                                    \
        minunit_tmp_e = "<null pointer>";                                      \
      } if (!minunit_tmp_r) {                                                  \
        minunit_tmp_r = "<null pointer>";                                      \
      } if (strcmp(minunit_tmp_e, minunit_tmp_r) != 0) {                       \
        (void)snprintf(minunit_last_message, MINUNIT_MESSAGE_LEN,              \
                       "%s failed:\n\t%s:%d: '%s' expected but was '%s'",      \
                       __func__, __FILE__, __LINE__, minunit_tmp_e,            \
                       minunit_tmp_r);                                         \
        minunit_status = 1;                                                    \
        return;                                                                \
      } else { (void)colorize_printf(FG_GREEN, "."); })

static double mu_timer_real(void) {
#if defined(_WIN32)
  LARGE_INTEGER Time;
  LARGE_INTEGER Frequency;

  QueryPerformanceFrequency(&Frequency);
  QueryPerformanceCounter(&Time);

  Time.QuadPart *= 1000000;
  Time.QuadPart /= Frequency.QuadPart;

  return (double)Time.QuadPart / 1000000.0;

#elif (defined(__hpux) || defined(hpux)) ||                                    \
    ((defined(__sun__) || defined(__sun) || defined(sun)) &&                   \
     (defined(__SVR4) || defined(__svr4__)))
  return (double)gethrtime() / 1000000000.0;

#elif defined(__MACH__) && defined(__APPLE__)
  static double timeConvert = 0.0;
  if (timeConvert == 0.0) {
    mach_timebase_info_data_t timeBase;
    (void)mach_timebase_info(&timeBase);
    timeConvert =
        (double)timeBase.numer / (double)timeBase.denom / 1000000000.0;
  }
  return (double)mach_absolute_time() * timeConvert;

#elif defined(_POSIX_VERSION)
  struct timeval tm;
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
  {
    struct timespec ts;
#if defined(CLOCK_MONOTONIC_PRECISE)
    const clockid_t id = CLOCK_MONOTONIC_PRECISE;
#elif defined(CLOCK_MONOTONIC_RAW)
    const clockid_t id = CLOCK_MONOTONIC_RAW;
#elif defined(CLOCK_HIGHRES)
    const clockid_t id = CLOCK_HIGHRES;
#elif defined(CLOCK_MONOTONIC)
    const clockid_t id = CLOCK_MONOTONIC;
#elif defined(CLOCK_REALTIME)
    const clockid_t id = CLOCK_REALTIME;
#else
    const clockid_t id = (clockid_t)-1;
#endif
    if (id != (clockid_t)-1 && clock_gettime(id, &ts) != -1)
      return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
  }
#endif

  gettimeofday(&tm, NULL);
  return (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
#else
  return -1.0;
#endif
}

static double mu_timer_cpu(void) {
#if defined(_WIN32)
  FILETIME createTime;
  FILETIME exitTime;
  FILETIME kernelTime;
  FILETIME userTime;

  if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime,
                      &userTime) != 0) {
    ULARGE_INTEGER userSystemTime;
    memcpy(&userSystemTime, &userTime, sizeof(ULARGE_INTEGER));
    return (double)userSystemTime.QuadPart / 10000000.0;
  }

#elif defined(__unix__) || defined(__unix) || defined(unix) ||                 \
    (defined(__APPLE__) && defined(__MACH__))

#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
  {
    clockid_t id;
    struct timespec ts;
#if _POSIX_CPUTIME > 0
    if (clock_getcpuclockid(0, &id) == -1)
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
      id = CLOCK_PROCESS_CPUTIME_ID;
#elif defined(CLOCK_VIRTUAL)
    id = CLOCK_VIRTUAL;
#else
    id = (clockid_t)-1;
#endif
    if (id != (clockid_t)-1 && clock_gettime(id, &ts) != -1)
      return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
  }
#endif

#if defined(RUSAGE_SELF)
  {
    struct rusage rusage;
    if (getrusage(RUSAGE_SELF, &rusage) != -1)
      return (double)rusage.ru_utime.tv_sec +
             (double)rusage.ru_utime.tv_usec / 1000000.0;
  }
#endif

#if defined(_SC_CLK_TCK)
  {
    const double ticks = (double)sysconf(_SC_CLK_TCK);
    struct tms tms;
    if (times(&tms) != (clock_t)-1)
      return (double)tms.tms_utime / ticks;
  }
#endif

#if defined(CLOCKS_PER_SEC)
  {
    clock_t cl = clock();
    if (cl != (clock_t)-1)
      return (double)cl / (double)CLOCKS_PER_SEC;
  }
#endif

#endif

  return -1;
}

#ifdef __cplusplus
}
#endif

#endif