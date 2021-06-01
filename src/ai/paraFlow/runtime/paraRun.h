/* paraRun - stuff to help manage para operations on collections.  */

#ifndef PARARUN_H
#define PARARUN_H

void _pf_paraRunInit();
/* Initialize manager and worker threads who will proceed
 * to wait for work. */

enum paraRunType
/* How to do para run */
    {
    prtParaDo,
    prtParaAdd,
    prtParaMultiply,
    prtParaAnd,
    prtParaOr,
    prtParaMin,
    prtParaMax,
    prtParaArgMin,
    prtParaArgMax,
    prtParaGet,
    prtParaFilter
    };

void *_pf_paraRunArray(struct _pf_array *array, void *localVars, int localSize,
    void (*process)(_pf_Stack *stack, char *key, void *item, void *localVars),
    enum paraRunType prtType, int expTypeId);
/* Run process on each item in array. */

void *_pf_paraRunDir(struct _pf_dir *dir, void *localVars, int localSize,
    void (*process)(_pf_Stack *stack, char *key, void *item, void *localVars),
    enum paraRunType prtType, int expTypeId);
/* Run process on each item in dir. */

void *_pf_paraRunRange(long start, long end, void *localVars, int localSize,
    void (*process)(_pf_Stack *stack, char *key, void *item, void *localVars),
    enum paraRunType prtType, int expTypeId);
/* Run process on each item in range. */

#endif /* PARARUN_H */

