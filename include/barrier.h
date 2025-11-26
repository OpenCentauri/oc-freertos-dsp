/*
 * Memory barrier definitions for FreeRTOS on Xtensa
 */
#ifndef _BARRIER_H_
#define _BARRIER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Xtensa memory barrier instructions */
#define barrier() __asm__ __volatile__("" : : : "memory")

#ifdef __XTENSA__
#define mb()     __asm__ __volatile__("memw" : : : "memory")
#define rmb()    __asm__ __volatile__("memw" : : : "memory")
#define wmb()    __asm__ __volatile__("memw" : : : "memory")
#else
#define mb()     barrier()
#define rmb()    barrier()
#define wmb()    barrier()
#endif

#define smp_mb()  mb()
#define smp_rmb() rmb()
#define smp_wmb() wmb()

#define dsb()     mb()
#define dmb()     mb()
#define isb()     barrier()

#ifdef __cplusplus
}
#endif

#endif /* _BARRIER_H_ */
