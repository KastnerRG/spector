#include <sys/time.h>

#define GET_TIME_INIT(num) struct timeval _timers[num]

#define GET_TIME_VAL(num) gettimeofday(&_timers[num], NULL)

#define TIME_VAL_TO_MS(num) (((double)_timers[num].tv_sec*1000.0) + ((double)_timers[num].tv_usec/1000.0))

#define ELAPSED_TIME_MS(e, s) (TIME_VAL_TO_MS(e) - TIME_VAL_TO_MS(s))
