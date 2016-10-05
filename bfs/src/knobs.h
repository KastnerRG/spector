#define KNOB_UNROLL_FACTOR     8
#define KNOB_COMPUTE_UNITS_1   1
#define KNOB_COMPUTE_UNITS_2   1
#define KNOB_BRANCH            1
#define KNOB_MASK_TYPE         1

#if   KNOB_MASK_TYPE == 1
typedef char  mask_t;
#elif KNOB_MASK_TYPE == 2
typedef short mask_t;
#else
typedef int   mask_t;
#endif

#define KNOB_SIMD_1            1
#define KNOB_NUM_WORK_ITEMS_1  256
#define KNOB_NUM_WORK_ITEMS_2  256
#define KNOB_SIMD_2            2


