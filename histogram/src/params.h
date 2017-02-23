
#ifndef PARAMS_H_
#define PARAMS_H_


#define KNOB_NUM_HIST        2
#define KNOB_HIST_SIZE       256

#define KNOB_NUM_WORK_ITEMS  1
#define KNOB_NUM_WORK_GROUPS 1

#define KNOB_SIMD            1
#define KNOB_COMPUTE_UNITS   1

#define KNOB_ACCUM_SMEM      0

#define KNOB_UNROLL_FACTOR   1



#define TOTAL_WORK_ITEMS (KNOB_NUM_WORK_ITEMS * KNOB_NUM_WORK_GROUPS)



#endif /* PARAMS_H_ */
