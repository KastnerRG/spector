
typedef int data_t;

#define KNOB_WORK_ITEMS         2

#define KNOB_LOCAL_SORT_LOGSIZE 3
#define KNOB_LOCAL_USE_PTR      0
#define KNOB_SPECIAL_CASE_1     1


#define KNOB_WORK_GROUPS        1
#define KNOB_COMPUTE_UNITS      1

#define KNOB_UNROLL_LOCAL_COPY  1


#if KNOB_WORK_GROUPS > 1
#define GENERALIZED_START_SIZE
#endif

#define LOCAL_SORT_SIZE (1 << KNOB_LOCAL_SORT_LOGSIZE)
#define TOTAL_WORK_ITEMS (KNOB_WORK_ITEMS*KNOB_WORK_GROUPS)

#if KNOB_UNROLL_LOCAL_COPY < 0
#undef KNOB_UNROLL_LOCAL_COPY
#define KNOB_UNROLL_LOCAL_COPY
#endif


