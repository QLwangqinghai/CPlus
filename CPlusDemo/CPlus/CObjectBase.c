//
//  CObjectBase.c
//  O
//
//  Created by wangqinghai on 2018/3/16.
//  Copyright © 2018年 wangqinghai. All rights reserved.
//

#include "CPlusRuntimePrivate.h"

long CPGetFilePageSize(void) {
    return sysconf(_SC_PAGESIZE);
}
long CPGetMemoryPageSize(void) {
    return getpagesize();
}

//typedef struct {
//    void * _Nullable customInfo;
//    uint32_t customInfoSize;
//    CPInfoStorage_s * _Nonnull info;
//    void * _Nullable contentSize;
//    void * _Nonnull object;
//} CPInfoWrapper_s;

static inline void CPObjectCheckContentSize(CPType _Nonnull type, CPActiveInfo_s * _Nonnull activeInfo) {
    assert(type);
    assert(activeInfo);
    
#if CPMemoryHeaderAligent64
    if ((type->base.contentHasPadding == 1 && activeInfo->contentSize == 0) || (type->base.contentHasPadding == 0 && activeInfo->contentSize != 0)) {
        abort();
    }
#else
    if (type->base.contentHasPadding != activeInfo->contentSize) {
        abort();
    }
#endif
}
static inline size_t __CPObjectGetContentSize(void * _Nonnull sizeAddress) {
    assert(sizeAddress);
    size_t size = 0;
#if CPMemoryHeaderAligent64
    uint64_t v = *(uint64_t *)sizeAddress;
    size = (size_t)v;
#else
    uint32_t v = *(uint32_t *)sizeAddress;
    size = (size_t)v;
#endif
    return size;
}

static size_t const CPContentSizeOffset = CPInfoStoreSize + CPContentAligentBlock;

static inline CPInfoWrapper_s CPObjectGetInfoWrapper(CPObject _Nonnull obj) {
    CPInfoWrapper_s wrapper = {};
    wrapper.object = obj;
    wrapper.info = (CPInfoStorage_s *)((uint8_t *)obj - CPInfoStoreSize);
    CPType type = CPGetType(wrapper.info);
    CPActiveInfo_s activeInfo = CPGetActiveInfo(wrapper.info);
    size_t customInfoSize = (size_t)(type->base.customInfoSize * CPContentAligentBlock);
    CPObjectCheckContentSize(type, &activeInfo);

#if CPMemoryHeaderAligent64
    if (activeInfo.contentSize == 0) {
        wrapper.contentSize = CPAlignContentSize(type->base.contentBaseSize);
        wrapper.address = ((uint8_t *)obj - CPInfoStoreSize - customInfoSize);

    } else if (activeInfo.contentSize == CPMaxContentSizeInActiveInfo) {
        //有8字节长度信息
        wrapper.contentSize = __CPObjectGetContentSize((uint8_t *)obj - CPContentSizeOffset);
        wrapper.address = ((uint8_t *)obj - CPContentSizeOffset - customInfoSize);
    } else {
        wrapper.contentSize = activeInfo.contentSize * CPContentAligentBlock;
        wrapper.address = ((uint8_t *)obj - CPInfoStoreSize - customInfoSize);
    }
#else
    if (activeInfo.contentSize == 1) {
        //有4字节长度信息
        wrapper.contentSize = __CPObjectGetContentSize((uint8_t *)obj - CPContentSizeOffset);
        wrapper.address = ((uint8_t *)obj - CPContentSizeOffset - customInfoSize);
    } else {
        wrapper.contentSize = CPAlignContentSize(type->base.contentBaseSize);
        wrapper.address = ((uint8_t *)obj - CPInfoStoreSize - customInfoSize);
    }
#endif
    if (type->base.customInfoSize > 0) {
        wrapper.customInfo = wrapper.address;
        wrapper.customInfoSize = (uint32_t)customInfoSize;
    }
    return wrapper;
}

static inline void * _Nonnull CPObjectGetRealAddress(CPObject _Nonnull obj) {
    CPInfoStorage_s * info = (CPInfoStorage_s *)((uint8_t *)obj - CPInfoStoreSize);
    CPType type = CPGetType(info);
    CPActiveInfo_s activeInfo = CPGetActiveInfo(info);
    size_t customInfoSize = (size_t)(type->base.customInfoSize * CPContentAligentBlock);
    CPObjectCheckContentSize(type, &activeInfo);
    
#if CPMemoryHeaderAligent64
    if (activeInfo.contentSize == 0) {
        return ((uint8_t *)obj - CPInfoStoreSize - customInfoSize);
    } else if (activeInfo.contentSize == CPMaxContentSizeInActiveInfo) {
        //有8字节长度信息
        return ((uint8_t *)obj - CPContentSizeOffset - customInfoSize);
    } else {
        return ((uint8_t *)obj - CPInfoStoreSize - customInfoSize);
    }
#else
    if (activeInfo.contentSize == 1) {
        //有4字节长度信息
        return ((uint8_t *)obj - CPContentSizeOffset - customInfoSize);
    } else {
        return ((uint8_t *)obj - CPInfoStoreSize - customInfoSize);
    }
#endif
}

CPAllocedMemory_s CPBaseAlloc(struct __CPAlloctor const * _Nonnull alloctor, size_t size) {
    if (size == 0) {
        abort();
    }
    void * ptr = CBaseAlloc(size);
    if (NULL == ptr) {
        if(CPMemoryManagerDefault.oomHandler == NULL) {
            printf("alloc oom error!!!\n");
            abort();
        } else {
            CPMemoryManagerDefault.oomHandler(&CPMemoryManagerDefault, size);
            abort();
        }
    }
    CPAllocedMemory_s m = {ptr, size};
    return m;
}
void CPBaseDealloc(struct __CPAlloctor const * _Nonnull alloctor, void * _Nonnull ptr, size_t size) {
    assert(ptr);
    if (size == 0) {
        abort();
    }
    CBaseFree(ptr, size);
}

void * _Nonnull CPAllocInit(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentPaddingSize, _Bool autoDealloc, _Bool isStatic) {
    
    assert(alloctor);
    assert(type);
    
    size_t customInfoSize = type->base.customInfoSize * CPContentAligentBlock;
    size_t contentSize = type->base.contentBaseSize;
    if (type->base.contentHasPadding) {
        contentSize += contentPaddingSize;
    }
    size_t realContentSize = CPAlignContentSize(contentSize);
    
    void * ptr = NULL;
    
    if (realContentSize > CPMaxContentSize) {
        printf("alloc a too large size memory!!!\n");
        abort();
    }
    
    CPActiveInfo_s activeInfo = {};
    if (isStatic) {
        activeInfo.refrenceCount = CPRefrenceStaticFlag;
        activeInfo.autoDealloc = 0;
    } else {
        activeInfo.refrenceCount = 1;
        activeInfo.autoDealloc = autoDealloc;
    }
    
    if (type->base.contentHasPadding) {
        size_t contentBlockCountInHeader = realContentSize / CPContentAligentBlock;
        
        if (contentBlockCountInHeader >= CPMaxContentSizeInActiveInfo) {
            activeInfo.contentSize = CPMaxContentSizeInActiveInfo;
            
            size_t realSize = customInfoSize + CPContentSizeByteCount + sizeof(CPInfoStorage_s) + realContentSize;
            CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
            ptr = m.ptr;
            
            __CPStoreContentSize((uint8_t *)ptr + customInfoSize, realContentSize);
            CPInfoStorage_s * tmp = (CPInfoStorage_s *)((uint8_t *)ptr + customInfoSize + CPContentSizeByteCount);
            CPInfoStorageStore(tmp, type, &activeInfo);
            return ((uint8_t *)ptr + customInfoSize + CPContentSizeByteCount + CPInfoStoreSize);
        } else {
            activeInfo.contentSize = contentBlockCountInHeader;
            
            size_t realSize = customInfoSize + CPInfoStoreSize + realContentSize;
            CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
            ptr = m.ptr;
            
            CPInfoStorage_s * tmp = (CPInfoStorage_s *)((uint8_t *)ptr + customInfoSize);
            CPInfoStorageStore(tmp, type, &activeInfo);
            return ((uint8_t *)ptr + customInfoSize + CPInfoStoreSize);
        }
    } else {
        activeInfo.contentSize = 0;
        size_t realSize = customInfoSize + CPInfoStoreSize + realContentSize;
        CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
        ptr = m.ptr;
        CPInfoStorage_s * tmp = (CPInfoStorage_s *)((uint8_t *)ptr + customInfoSize);
        CPInfoStorageStore(tmp, type, &activeInfo);
        return ((uint8_t *)ptr + customInfoSize + CPInfoStoreSize);
    }
}




static inline void * _Nonnull __CPGetMemoryStoreInfo(CPObject _Nonnull obj, size_t * sizePtr) {
    assert(obj);
    
    CPInfoStorage_s * info = (CPInfoStorage_s *)((uint8_t *)obj - CPInfoStoreSize);
    CPType type = CPGetType(info);
    CPActiveInfo_s activeInfo = CPGetActiveInfo(info);
    size_t customInfoSize = (size_t)(type->base.customInfoSize * CPContentAligentBlock);
    CPObjectCheckContentSize(type, &activeInfo);
    
#if CPMemoryHeaderAligent64
    if (activeInfo.contentSize == 0) {
        size_t contentSize = CPAlignContentSize(type->base.contentBaseSize);
        void * ptr = ((uint8_t *)obj - CPInfoStoreSize - customInfoSize);
        if (sizePtr) {
            *sizePtr = contentSize + CPInfoStoreSize + customInfoSize;
        }
        return ptr;
    } else if (activeInfo.contentSize == CPMaxContentSizeInActiveInfo) {
        //有8字节长度信息
        size_t contentSize = __CPObjectGetContentSize((uint8_t *)obj - CPContentSizeOffset);
        void * ptr = ((uint8_t *)obj - CPContentSizeOffset - customInfoSize);
        if (sizePtr) {
            *sizePtr = contentSize + CPContentSizeOffset + customInfoSize;
        }
        return ptr;
    } else {
        size_t contentSize = activeInfo.contentSize * CPContentAligentBlock;
        void * ptr = ((uint8_t *)obj - CPInfoStoreSize - customInfoSize);
        if (sizePtr) {
            *sizePtr = contentSize + CPInfoStoreSize + customInfoSize;
        }
        return ptr;
    }
#else
    if (activeInfo.contentSize == 1) {
        //有4字节长度信息
        size_t contentSize = __CPObjectGetContentSize((uint8_t *)obj - CPContentSizeOffset);
        void * ptr = ((uint8_t *)obj - CPContentSizeOffset - customInfoSize);
        if (sizePtr) {
            *sizePtr = contentSize + CPContentSizeOffset + customInfoSize;
        }
        return ptr;
    } else {
        size_t contentSize = CPAlignContentSize(type->base.contentBaseSize);
        void * ptr = ((uint8_t *)obj - CPInfoStoreSize - customInfoSize);
        if (sizePtr) {
            *sizePtr = contentSize + CPInfoStoreSize + customInfoSize;
        }
        return ptr;
    }
#endif
}

void CPDealloc(struct __CPAlloctor const * _Nonnull alloctor, CPObject _Nonnull obj) {
    assert(obj);

    size_t size = 0;
    void * ptr = __CPGetMemoryStoreInfo(obj, &size);
    
    
    //release type
    
    
    
    
    CBaseFree(ptr, size);

}

size_t CPGetStoreSizeWithType(CPType _Nonnull type, size_t contentPaddingSize) {
    assert(type);
    
    size_t customInfoSize = type->base.customInfoSize * CPContentAligentBlock;
    size_t contentSize = type->base.contentBaseSize;
    if (type->base.contentHasPadding) {
        contentSize += contentPaddingSize;
    }
    size_t realContentSize = CPAlignContentSize(contentSize);
    
    size_t realSize = customInfoSize + sizeof(CPInfoStorage_s) + realContentSize;
    if (type->base.contentHasPadding) {
#if CPMemoryHeaderAligent64
        size_t contentBlockCountInHeader = realContentSize / CPContentAligentBlock;
        if (contentBlockCountInHeader >= CPMaxContentSizeInActiveInfo) {
            realSize += CPContentAligentBlock;
        }
#else
        realSize += CPContentAligentBlock;
#endif
        realSize += CPContentAligentBlock;
    }
    return realSize;
}

size_t CPGetStoreSize(void * _Nonnull obj) {
    size_t size = 0;
    __CPGetMemoryStoreInfo(obj, &size);

    return size;
}

static CPAlloctor_s const CPAlloctorDefault = {CPBaseAlloc, CPBaseDealloc, CPAllocInit, CPDealloc, NULL};
CPAlloctor_s const * _Nonnull CPAlloctorGetDefault(void) {
    return &CPAlloctorDefault;
}




CPInfoStorage_s * _Nullable CPReferenceCountAdd(CPInfoStorage_s * _Nonnull header) {
    assert(header);
    CPActiveInfo_s activeInfo = {};
    CPActiveInfo_s newValue = {};
    
    do {
        activeInfo = CPGetActiveInfo(header);
        if (activeInfo.deallocing == 1) {//dealloc obj
            printf("CPlus error, retain an dealloc obj.\n");
            return NULL;
        } else if (activeInfo.prepareDealloc == 1) {//prepare dealloc obj
            return NULL;
        } else if (activeInfo.refrenceCount == CPRefrenceStaticFlag) {//static obj
            return header;
        }
        newValue = activeInfo;
        newValue.refrenceCount += 1;
    } while (!CPCASSetActiveInfo(header, activeInfo, newValue));
    return header;
}

CPActiveInfo_s CPReferenceCountSubtract(CPInfoStorage_s * _Nonnull header) {
    assert(header);
    CPActiveInfo_s activeInfo = {};
    CPActiveInfo_s newValue = {};
    do {
        activeInfo = CPGetActiveInfo(header);
        if (activeInfo.deallocing == 1) {//dealloc obj
            printf("CPlus error, release an dealloc obj.\n");
            abort();
            return activeInfo;
        } else if (activeInfo.prepareDealloc == 1) {//prepare dealloc obj
            printf("CPlus error, release an prepare dealloc obj.\n");
            abort();
            return activeInfo;
        } else if (activeInfo.refrenceCount == CPRefrenceStaticFlag) {//static obj
            return activeInfo;
        }
        if (activeInfo.refrenceCount == 0) {
            printf("CPlus error, release an error obj.\n");
            abort();
            return activeInfo;
        } else {
            newValue = activeInfo;
            newValue.refrenceCount -= 1;

            if (activeInfo.autoDealloc == 1 && newValue.refrenceCount == 0) {
                newValue.prepareDealloc = 1;
            }
        }
    } while (!CPCASSetActiveInfo(header, activeInfo, newValue));
    return newValue;
}

int32_t CPDefaultSetDeallocingFlag(CPInfoStorage_s * _Nonnull header) {
    assert(header);
    
    CPActiveInfo_s activeInfo = {};
    CPActiveInfo_s newValue = {};
    do {
        activeInfo = CPGetActiveInfo(header);
        if (activeInfo.deallocing == 1) {//dealloc obj
            printf("CPlus error when prepare dealloc, reason: a error object.\n");
            abort();
            return 1;
        } else if (activeInfo.prepareDealloc == 1) {//prepare dealloc obj
#if DEBUG
            if (activeInfo.refrenceCount > 0) {
                abort();
            }
#endif
            newValue = activeInfo;
            newValue.deallocing = 1;
        } else {
            return 1;
        }
    } while (!CPCASSetActiveInfo(header, activeInfo, newValue));
    return 0;
}






typedef uint32_t CObjectAutoreleasePoolFlag_t;

CObjectAutoreleasePoolFlag_t CObjectAutoreleasePoolPush(void);
void CObjectAutoreleasePoolPop(CObjectAutoreleasePoolFlag_t flag);

void CPAutoreleasePool(void(^_Nonnull block)(void)) {
    assert(block);
    CObjectAutoreleasePoolFlag_t flag = CObjectAutoreleasePoolPush();
    block();
    CObjectAutoreleasePoolPop(flag);
}

static inline CPInfoStorage_s * _Nonnull __CPGetInfo(void const * _Nonnull obj) {
    assert(obj);

    uint8_t const * mem = (uint8_t const *)obj;
    mem = mem - CPInfoStoreSize;
    return (CPInfoStorage_s *)mem;
}
CPInfoStorage_s * _Nonnull CPGetInfo(void const * _Nonnull obj) {
    return __CPGetInfo(obj);
}


CPObject _Nullable CPRetain(void const * _Nullable obj) {
    if (NULL == obj) {
        return NULL;
    }
    
    CPInfoStorage_s * info = CPGetInfo(obj);
    CPActiveInfo_s activeInfo = CPGetActiveInfo(info);
    CPInfoStorage_s * newHeader = CPReferenceCountAdd(info);
    CPType_s * type = CPGetType(info);
    if (newHeader) {
        if (activeInfo.refrenceCount > 0) {
            CPDidRetainDispatch(type, obj);
        }
        return obj;
    } else {
        return NULL;
    }
}
void CPRelease(void const * _Nullable obj) {
    if (NULL == obj) {
        return;
    }
    CPInfoStorage_s * info = CPGetInfo(obj);
    CPType_s * type = CPGetType(info);

//    CPActiveInfo_s activeInfo = CPGetActiveInfo(header);
    CPActiveInfo_s activeInfo = CPReferenceCountSubtract(info);

    CPDidReleaseDispatch(type, obj);
    
    if (activeInfo.prepareDealloc) {
        CPDidPrepareDeallocDispatch(type, obj);
        int32_t result = CPDefaultSetDeallocingFlag(info);
        if (result == 0) {
            CPWillDeallocDispatch(type, obj);
            CPDeinitDispatch(type, obj);
            CPAlloctor_s const * alloctor = (CPAlloctor_s *)(type->base.alloctor);
            if (NULL == alloctor) {
                alloctor = CPAlloctorGetDefault();
            }
            alloctor->dealloc(alloctor, obj);
        }
    } else {
        if (activeInfo.refrenceCount == 0) {
            CPDidResignActiveDispatch(type, obj);
        }
    }

}

void CObjectAutoreleasePoolAppend(CPObject _Nullable obj);
void CPAutorelease(void const * _Nullable obj) {
    if (NULL == obj) {
        return;
    }
    
    CObjectAutoreleasePoolAppend(obj);
}


//static CObjectManager_t CObjectManager = {
//    1,
//    &COMemoryManager,
//    NULL,
//    CODefaultRetain,
//    CODefaultRelease,
//    CODefaultPrepareDealloc,
//    CODefaultSetDeallocingFlag,
//    CODefaultDeinit,
//    CODefaultDidResignActive
//};


//static uint32_t const CObjectManagerKeyMax = 255;
//static CObjectManager_t const * CObjectManagerBuffer[256] = {
//    &CObjectManager,
//    NULL
//};
//
//static uint32_t CObjectManagerBufferCurrentCount = 1;



//int CObjectManagerKeyCreate(uint8_t * _Nonnull keyRef, CObjectManager_t * _Nonnull manager) {
//    assert(keyRef);
//    assert(manager);
//
//    static pthread_mutex_t CObjectManagerKeyCreateMenu = PTHREAD_MUTEX_INITIALIZER;
//    pthread_mutex_lock(&CObjectManagerKeyCreateMenu);
//    uint8_t key = CObjectManagerBufferCurrentCount;
//    if (key > CObjectManagerKeyMax) {
//        printf("CObjectManagerKeyCreate too much key!\n");
//        pthread_mutex_unlock(&CObjectManagerKeyCreateMenu);
//        return -1;
//    }
//    CObjectManagerBufferCurrentCount ++;
//    CObjectManagerBuffer[key] = manager;
//    *(uint8_t *)&(manager->identifier) = key;
//    pthread_mutex_unlock(&CObjectManagerKeyCreateMenu);
//    *keyRef = key;
//    return 0;
//}

//CObjectManager_t const * _Nullable CObjectManagerGet(uint8_t key) {
//    if (key <= CObjectManagerKeyMax) {
//        return CObjectManagerBuffer[key];
//    } else {
//        return NULL;
//    }
//}
//CObjectManager_t const * _Nullable CObjectGetObjectManager(CObject_o * _Nonnull object) {
//    assert(object);
//    return CObjectManagerGet(object->typeId.managerIdentifier);
//}
//
//COMemoryManager_t * _Nonnull CObjectManagerGetMemoryManager(CObjectManager_t const * _Nonnull manager) {
//    return (COMemoryManager_t *)(manager->memoryManager);
//}
//
//
//CObjectManager_t const * _Nonnull CObjectManagerGetDefault(void) {
//    return &CObjectManager;
//}
//
//
//COMemoryManager_t const * _Nonnull COMemoryManagerGetDefault(void) {
//    return &COMemoryManager;
//}

//CObject_o * _Nonnull CObjectManagerCreateObject(CObjectManager_t const * _Nonnull manager, uint32_t detailInfo, CPActiveInfo_s activeInfo, size_t contentSize) {
//    assert(manager);
//
//    if (manager->identifier == 0 && manager != &CObjectManager) {
//        printf("an error manager;\n");
//        abort();
//    }
//
//    COMemoryManager_t * memoryManager = CObjectManagerGetMemoryManager(manager);
//    return memoryManager->malloc(manager, detailInfo, activeInfo, contentSize);
//}


/********************************* COAutoreleasePool *********************************/

#pragma mark - COAutoreleasePool

static const uint32_t CObjectAutoreleasePoolPageCapacity = 2048;
static inline uint32_t CObjectAutoreleasePoolPageIndex(uint32_t location) {
    return location >> 11;
}
static inline uint32_t CObjectAutoreleasePoolItemIndexInPage(uint32_t location) {
    return location & 0x7FF;
}
typedef struct {
    CPObject _Nullable objects[CObjectAutoreleasePoolPageCapacity];
} CObjectAutoreleasePoolPage_t;
typedef struct {
    uint32_t count;
    uint32_t poolPageSize;
    CObjectAutoreleasePoolPage_t * _Nullable * _Nonnull poolPages;
} CObjectAutoreleasePool_t;
CObjectAutoreleasePool_t * _Nonnull CObjectAutoreleasePoolGet(void);

void __CObjectAutoreleasePoolPop(CObjectAutoreleasePool_t * _Nonnull pool, CObjectAutoreleasePoolFlag_t flag) {
    assert(pool);
    assert(flag < pool->count);
    uint32_t count = pool->count;
    pool->count = flag;
    
    for (uint32_t i=flag; i<count; i ++) {
        uint32_t pageIndex = CObjectAutoreleasePoolPageIndex(i);
        uint32_t itemIndex = CObjectAutoreleasePoolItemIndexInPage(i);
        
        if (pageIndex >= pool->poolPageSize) {
            abort();
        }
        CObjectAutoreleasePoolPage_t * page = pool->poolPages[pageIndex];
        if (NULL == page) {
            abort();
        }
        CPObject obj = page->objects[itemIndex];
        CPRelease(obj);
    }
}

void CObjectAutoreleasePoolDealloc(CObjectAutoreleasePool_t * _Nonnull pool) {
    __CObjectAutoreleasePoolPop(pool, 0);
    
    for (uint32_t index=0; index<pool->count; index ++) {
        CObjectAutoreleasePoolPage_t * page = pool->poolPages[index];
        free(page);
    }
    free(pool);
}

void CObjectAutoreleasePoolAppend(CPObject _Nullable obj) {
    CObjectAutoreleasePool_t * pool = CObjectAutoreleasePoolGet();
    
    uint32_t pageIndex = CObjectAutoreleasePoolPageIndex(pool->count);
    uint32_t itemIndex = CObjectAutoreleasePoolItemIndexInPage(pool->count);
    if (pool->count == UINT32_MAX) {
        abort();
    }
    if (pageIndex >= pool->poolPageSize) {
        uint64_t newPoolPageSize = pool->poolPageSize;
        newPoolPageSize *= 2;
        if (newPoolPageSize > UINT32_MAX) {
            newPoolPageSize = UINT32_MAX;
        }
        if (newPoolPageSize > pool->poolPageSize) {
            CObjectAutoreleasePoolPage_t ** poolPages = (CObjectAutoreleasePoolPage_t **)malloc(sizeof(CObjectAutoreleasePoolPage_t *) * newPoolPageSize);
            memcpy(poolPages, pool->poolPages, sizeof(CObjectAutoreleasePoolPage_t *) * pool->poolPageSize);
            free(pool->poolPages);
            pool->poolPages = poolPages;
            pool->poolPageSize = (uint32_t)newPoolPageSize;
        } else {
            abort();
        }
    }
    
    CObjectAutoreleasePoolPage_t * page = pool->poolPages[pageIndex];
    if (NULL == page) {
        page = (CObjectAutoreleasePoolPage_t *)malloc(sizeof(CObjectAutoreleasePoolPage_t));
        memset(page, 0, sizeof(CObjectAutoreleasePoolPage_t));
        pool->poolPages[pageIndex] = page;
    }
    
    page->objects[itemIndex] = obj;
    pool->count += 1;
}

CObjectAutoreleasePoolFlag_t CObjectAutoreleasePoolPush(void) {
    CObjectAutoreleasePool_t * pool = CObjectAutoreleasePoolGet();
    uint32_t location = pool->count;
    CObjectAutoreleasePoolAppend(NULL);
    return location;
}
void CObjectAutoreleasePoolPop(CObjectAutoreleasePoolFlag_t flag) {
    CObjectAutoreleasePool_t * pool = CObjectAutoreleasePoolGet();
    __CObjectAutoreleasePoolPop(pool, flag);
}


void __CObjectAutoreleasePoolShareKeyDealloc(void * _Nullable value) {
    if (value) {
        CObjectAutoreleasePoolDealloc((CObjectAutoreleasePool_t *)value);
    }
}

static pthread_key_t __CObjectAutoreleasePoolShareKey = 0;
void __CObjectAutoreleasesPoolShareKeyLoad(void) {
    if (0 == __CObjectAutoreleasePoolShareKey) {
        int result = pthread_key_create(&__CObjectAutoreleasePoolShareKey, __CObjectAutoreleasePoolShareKeyDealloc);
        assert(result == 0);
    }
}
static inline pthread_key_t CObjectAutoreleasePoolShareKey() {
    return __CObjectAutoreleasePoolShareKey;
}

CObjectAutoreleasePool_t * _Nonnull CObjectAutoreleasePoolGet() {
    void * data = pthread_getspecific(CObjectAutoreleasePoolShareKey());
    CObjectAutoreleasePool_t * object = (CObjectAutoreleasePool_t *)data;
    if (NULL == object) {
        CObjectAutoreleasePool_t * pool = (CObjectAutoreleasePool_t *)malloc(sizeof(CObjectAutoreleasePool_t));
        CObjectAutoreleasePoolPage_t * * poolPages = (CObjectAutoreleasePoolPage_t * *)malloc(sizeof(CObjectAutoreleasePoolPage_t *) * 8);
        memset(poolPages, 0, sizeof(CObjectAutoreleasePoolPage_t *) * 8);
        CObjectAutoreleasePoolPage_t * page = (CObjectAutoreleasePoolPage_t *)malloc(sizeof(CObjectAutoreleasePoolPage_t));
        memset(page, 0, sizeof(CObjectAutoreleasePoolPage_t));
        
        poolPages[0] = page;
        pool->poolPageSize = 8;
        pool->poolPages = poolPages;
        pool->count = 1;
        page->objects[0] = NULL;

        object = pool;
        int result = pthread_setspecific(CObjectAutoreleasePoolShareKey(), object);
        assert(result == 0);
    }
    return object;
}



#pragma mark - init
void __CObjectInit() {
    __CObjectAutoreleasesPoolShareKeyLoad();

}

__attribute__((constructor(201)))
void CPlusModuleInit(void) {
#if DEBUG
    printf("CPlusModuleInit\n");
#endif
    static pthread_once_t token = PTHREAD_ONCE_INIT;
    pthread_once(&token,&__CObjectInit);
    
#if DEBUG
    printf("CPlusModuleInit end\n");
#endif
}

__attribute__((destructor))
static void CObjectModuleDeinit(void) {
#if DEBUG
    printf("CObjectModuleDeinit\n");
#endif
}

