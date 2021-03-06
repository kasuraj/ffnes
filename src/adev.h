#ifndef _NES_ADEV_H_
#define _NES_ADEV_H_

// 包含头文件
#include "stdefine.h"

// 类型定义
typedef struct {
    BYTE  *lpdata;
    DWORD  buflen;
} AUDIOBUF;

typedef struct {
    void* (*create )(int bufnum, int buflen);
    void  (*destroy)(void *ctxt);
    void  (*dequeue)(void *ctxt, AUDIOBUF **ppab);
    void  (*enqueue)(void *ctxt);
} ADEV;

// 全局变量声明
extern ADEV DEV_WAVEOUT;

#endif
