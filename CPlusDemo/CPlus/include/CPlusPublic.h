//
//  CPlusRuntimePublic.h
//  O
//
//  Created by wangqinghai on 2018/3/16.
//  Copyright © 2018年 wangqinghai. All rights reserved.
//

#ifndef CPlusPublic_h
#define CPlusPublic_h

#import <CPlusRuntime/CPlusRuntimePublic.h>



/********************************* CObjectManager *********************************/

#pragma mark - CObjectManager

//int CObjectManagerKeyCreate(uint8_t * _Nonnull keyRef, CObjectManager_t * _Nonnull manager);
//CObjectManager_t const * _Nullable CObjectManagerGet(uint8_t key);
//CObjectManager_t const * _Nullable CObjectGetObjectManager(CObject_o * _Nonnull object);
//



/********************************* init module *********************************/

#pragma mark - init module

__attribute__((constructor(201)))
void CPlusModuleInit(void);


/********************************* CConstantObject *********************************/

typedef struct _CCWeakContainerFlag {
    uint32_t useFlag;

} CCWeakContainerFlag_t;


//static uint32_t const CCQueueInfoMaxSize = 0x1FFFFE;
#pragma pack(push)
#pragma pack(1)
typedef struct _CCQueueInfo {
    uint32_t size: 32;
    uint32_t offset: 32;
} CCQueueInfo_t;
typedef struct _CCQueue {
    _Atomic(uint_fast32_t) bufferSize;
    _Atomic(uint_fast64_t) info;
    void * _Nonnull content;
    _Atomic(uintptr_t) buffer[0];
} CCQueue_t;
#pragma pack(pop)

static int32_t CCQueueAdd(CCQueue_t * _Nonnull queue, void * _Nonnull object) {
    assert(queue);
    assert(object);
    
    CCQueueInfo_t info = {};
    CCQueueInfo_t newInfo = {};
    uint32_t bufferSize = atomic_load(&(queue->bufferSize));
    uint64_t infoValue = 0;
    uint64_t newInfoValue = 0;
    
    do {
        infoValue = atomic_load(&(queue->info));
        info = *(CCQueueInfo_t *)&infoValue;
        newInfo.size = info.size + 1;
        
#if DEBUG
        if (info.size > bufferSize) {
            abort();
        } else if (info.size == queue->bufferSize) {
            return -1;
        }
#else
        if (info.size >= bufferSize) {
            return -1;
        }
#endif
        //        info.size += 1;
        //        offset + index = realIndex
        newInfoValue = *(uint64_t *)&newInfo;
        
    } while (!atomic_compare_exchange_weak(&(queue->info), &infoValue, newInfoValue));
    
    uint32_t lastIndex = (newInfo.size + newInfo.offset - 1) % bufferSize;
    
    uintptr_t ptr = (uintptr_t)object;
    
    atomic_store(&(queue->buffer[lastIndex]), ptr);
    return 0;
}
static void * _Nullable CCQueueRemove(CCQueue_t * _Nonnull queue) {
    assert(queue);
    CCQueueInfo_t info = {};
    CCQueueInfo_t newInfo = {};
    uint32_t bufferSize = atomic_load(&(queue->bufferSize));
    uint64_t infoValue = 0;
    uint64_t newInfoValue = 0;
    uintptr_t ptr = 0;

    do {
        infoValue = atomic_load(&(queue->info));
        info = *(CCQueueInfo_t *)&infoValue;
        if (info.size == 0) {
            return NULL;
        }
        newInfo.offset += 1;

        if (newInfo.offset == bufferSize) {
            newInfo.offset = 0;
        }
        newInfo.size = info.size - 1;
        newInfoValue = *(uint64_t *)&newInfo;
        
        uint32_t firstIndex = (newInfo.size + newInfo.offset - 1) % bufferSize;
        ptr = atomic_load(&(queue->buffer[firstIndex]));
        if (ptr == 0) {
            return NULL;
        }
    } while (!atomic_compare_exchange_weak(&(queue->info), &infoValue, newInfoValue));
    
    return *(void **)&ptr;
}


//typedef struct _CCQueue {
//    uint32_t const bufferSize;
//    uint32_t size;
//    _Atomic(uint_fast32_t) offset;
//    _Atomic(uint_fast32_t) offset;
//
//    unsigned int in;    /* data is added at offset (in % size) */
//    unsigned int out;    /* data is extracted from off. (out % size) */
//    uintptr_t buffer[0];
//} CCQueue_T;


typedef struct __CCWeakContainer {
    
    _Atomic(uintptr_t) content;
    
    struct _CCWeakContainer * _Nullable prev;
    struct _CCWeakContainer * _Nullable next;
} _CCWeakContainer_o;

typedef struct _CCWeakContainer {
    uint8_t content[sizeof(_CCWeakContainer_o)];
} CCWeakContainer_t;

typedef struct {
    CCWeakContainer_t * _Nonnull (* _Nonnull init)(void * _Nonnull object);
    void * _Nullable (* _Nonnull retainedContent)(CCWeakContainer_t * _Nonnull object);
//    void (* _Nonnull didContentDealloc)(CCWeakContainer_o * _Nonnull object);
} CCWeakContainer_c;

extern CPType _Nonnull CPType_CCNull;
extern CPType _Nonnull CPType_CCString;
extern CPType _Nonnull CPType_CCData;



typedef struct {
    uint32_t __unused__;
} CCNull_o;

typedef struct {
    uint32_t privateValue;
    uint32_t const length;
    uint64_t const cUtf8StringLength;
    char const cUtf8String[0];
} CCString_o;

typedef struct {
    uint32_t privateValue;
    uint64_t const byteLength;
    uint8_t const byte[0];
} CCData_o;

typedef struct {
    int64_t high;
    uint64_t low;
} CCSInteger_o;

typedef struct {
    uint64_t high;
    uint64_t low;
} CCUInteger_o;

typedef float float32_t;
typedef double float64_t;

typedef struct {
    float32_t content;
} CCFloat32_o;

typedef struct {
    float64_t content;
} CCFloat64_o;

typedef struct {
    uint32_t const contentSize;
    uint8_t byte[0];
} CPMemory_o;

typedef struct __CCCustomDeinitMemory {
    void (* _Nullable deinit)(struct __CCCustomDeinitMemory * _Nonnull obj);
    uint32_t const contentSize;
    uint8_t byte[0];
} CCCustomDeinitMemory_o;
typedef CCNull_o const * CCNull;
typedef CCString_o const * CCString;
typedef CCData_o const * CCData;


//_Bool CCObjectContain(void * _Nonnull object);
//
CCNull_o const * _Nonnull CCNullShare(void);
CCString_o const * _Nonnull CCStringInit(char const * _Nullable cUtf8String);
CCString_o const * _Nullable CCStringInitWithUtf8Byte(uint8_t const * _Nullable byte, uint32_t byteLength);
CCData_o * _Nonnull CCStringGetUtf8Data(CCString_o const * _Nonnull string);


CCData_o * _Nullable CCDataInit(uint8_t const * _Nullable byte, uint64_t byteLength);

CCSInteger_o * _Nonnull CCSIntegerInit(int64_t high, uint64_t low);
CCSInteger_o * _Nonnull CCSIntegerInitWithSInt64(int64_t number);
CCSInteger_o * _Nonnull CCSIntegerInitWithUInt64(uint64_t number);

CCUInteger_o * _Nonnull CCUIntegerInit(uint64_t high, uint64_t low);
CCUInteger_o * _Nonnull CCUIntegerInitUInt64(uint64_t number);

CCFloat32_o * _Nonnull CCFloat32Init(float32_t number);
CCFloat64_o * _Nonnull CCFloat64Init(float64_t number);

CPMemory_o * _Nonnull CPMemoryInit(uint32_t byteLength);
CCCustomDeinitMemory_o * _Nonnull CCCustomDeinitMemoryInit(uint32_t byteLength, void (* _Nullable deinit)(struct __CCCustomDeinitMemory * _Nonnull obj));



/********************************* COString *********************************/

typedef struct {
    uint32_t bigchar;
} CStringScalar;

_Bool CStringScalarInit(CStringScalar * _Nonnull scalar, uint32_t code);
//返回 CStringScalar 个数
_Bool CStringCheckUtf8Byte(const uint8_t * _Nonnull utf8StringByte, size_t length, size_t * _Nullable scalarCountRef, size_t * _Nullable validByteCountRef);
_Bool CStringCheckUtf8String(const char * _Nonnull utf8String, size_t * _Nullable scalarCount, size_t * _Nullable validByteCountRef);
_Bool CStringCheckUtf8StringWithLength(const char * _Nonnull utf8String, size_t length, size_t * _Nullable scalarCount, size_t * _Nullable validByteCountRef);


#endif /* CObjectBase_h */
