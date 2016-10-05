#ifndef PARAMETERS_H_
#define PARAMETERS_H_


#define FILTER_LENGTH 32


#define KNOB_COEF_SHIFT   32
#define KNOB_NUM_PARALLEL 8

#define KNOB_UNROLL_FILTER_1         // iterations: (FILTER_LENGTH-1)  -- data shift reg 
#define KNOB_UNROLL_FILTER_2         // iterations: (NUM_COEF_LOADS-1) -- coef shift reg (outer)
#define KNOB_UNROLL_FILTER_3     1   // iterations: FILTER_LENGTH      -- computation (outer)
#define KNOB_UNROLL_COEF_SHIFT_1     // iterations: KNOB_COEF_SHIFT    -- coef shift reg (inner)
#define KNOB_UNROLL_COEF_SHIFT_2     // iterations: KNOB_COEF_SHIFT    -- load coef
#define KNOB_UNROLL_TOTAL        1   // iterations: totalInputLength   -- main loop
#define KNOB_UNROLL_PARALLEL_1       // iterations: KNOB_NUM_PARALLEL  -- load data
#define KNOB_UNROLL_PARALLEL_2       // iterations: KNOB_NUM_PARALLEL  -- computation (inner)
#define KNOB_UNROLL_PARALLEL_3       // iterations: KNOB_NUM_PARALLEL  -- write result


#define KNOB_NUM_WORK_ITEMS  64
#define KNOB_NUM_WORK_GROUPS 1

#define KNOB_SIMD            1
#define KNOB_COMPUTE_UNITS   1


#define NUM_COEF_LOADS (FILTER_LENGTH / KNOB_COEF_SHIFT)
#define TOTAL_WORK_ITEMS (KNOB_NUM_WORK_ITEMS*KNOB_NUM_WORK_GROUPS)


#endif /* PARAMETERS_H_ */
