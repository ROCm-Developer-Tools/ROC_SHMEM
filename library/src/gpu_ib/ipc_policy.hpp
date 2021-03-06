/******************************************************************************
 * Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *****************************************************************************/
#ifndef IPC_POLICY_HPP
#define IPC_POLICY_HPP

#include <hip/hip_runtime.h>

#include "config.h"



class GPUIBBackend;
class GPUIBContext;
/*
 * GPU support for IPC is ON.
 */
class IpcOnImpl
{
    public:
    uint32_t    shm_size;
    char        **ipc_bases;

    __host__ void ipcHostInit(int my_pe, char** heap_bases);
    __host__ uint32_t ipcDynamicShared();

    __device__ bool isIpcAvailable(int my_pe, int target_pe)
    {
        return (((my_pe / shm_size) == (target_pe / shm_size))? true:false);
    }
    __device__ void ipcGpuInit(GPUIBBackend* gpu_backend, GPUIBContext* ctx,
                               int thread_id);

    __device__ void ipcCopy(void *dst, void *src, size_t size);

    __device__ void ipcFence(){__threadfence();}

    template <typename T> __device__ T ipcAMOFetchAdd(T *val, T value){
        return atomicAdd(val, value);
    }
    template <typename T> __device__ T ipcAMOFetchCas(T *val, T cond, T value){
        return atomicCAS(val, cond, value);
    }
    template <typename T> __device__ void ipcAMOAdd(T *val, T value){
        T tmp = atomicAdd(val, value);
    }
    template <typename T> __device__ void ipcAMOCas(T *val, T cond, T value){
        T temp = atomicCAS(val, cond, value);
    }

};

/*
 * IPC disabled.
 */
class IpcOffImpl
{
    public:
    uint32_t    shm_size;
    char        **ipc_bases;

    __host__ void ipcHostInit(int my_pe, char** heap_bases){}
    __host__ uint32_t ipcDynamicShared(){return 0;}

    __device__ bool isIpcAvailable(int my_pe, int target_pe){return 0;}
    __device__ void ipcGpuInit(GPUIBBackend* roc_shmem_handle, GPUIBContext* ctx,
                               int thread_id){}
    __device__ void ipcCopy(void *dst, void *src, size_t size){}

    __device__ void ipcFence(){}

    template <typename T> __device__ T ipcAMOFetchAdd(T *val, T value){return T();}
    template <typename T> __device__ T ipcAMOFetchCas(T *val, T cond, T value){return T();}
    template <typename T> __device__ void ipcAMOAdd(T *val, T value){}
    template <typename T> __device__ void ipcAMOCas(T *val, T cond, T value){}

};

/*
 * Select which one of our IPC policies to use at compile time.
 */
#ifdef USE_IPC
typedef IpcOnImpl IpcImpl;
#else
typedef IpcOffImpl IpcImpl;
#endif

#endif //IPC_POLICY_HPP
