//
//  CObjectPrivate.c
//  CObject
//
//  Created by wangqinghai on 2018/3/21.
//

#include "CPlusRuntimePrivate.h"
//
//CObject_o * _Nonnull COMemoryManagerMalloc(CObjectManager_t const * _Nonnull manager, uint32_t detailInfo, CPActiveInfo_s activeInfo, size_t contentSize) {
//    if (contentSize > CPMaxMemorySize) {
//        printf("COMemoryManagerMalloc error, size is to large, manager: %p, max size is \"COMemoryMaxSize\": %zu", manager, CPMaxMemorySize);
//        abort();
//    }
//    
//    CObject_o * ptr = (CObject_o *)CPCreate(sizeof(CObject_o) + contentSize);
//    if (NULL == ptr) {
//        printf("COMemoryManagerMalloc error, malloc error, manager: %p", manager);
//        abort();
//    }
//    
////    CPSetActiveInfo(ptr, activeInfo);
//    CPTypeIdentifier_s typeId = {};
//    typeId.managerIdentifier = manager->identifier;
//    typeId.detailInfo = detailInfo;
//    *(CPTypeIdentifier_s *)&(ptr->typeId) = typeId;
//    
//    return ptr;
//}
//void COMemoryManagerDealloc(CObjectManager_t const * _Nonnull manager, CObject_o * _Nonnull object) {
//    if (NULL == object) {
//        printf("COMemoryManagerDealloc error, object isNull, manager: %p", manager);
//        abort();
//    }
//    CPFree(object);
//}
//
//
//void CODefaultDeinit(CObjectManager_t const * _Nonnull manager, CObject_o * _Nonnull obj) {
//    assert(obj);
//    
//    COConstantType_t const * type = COConstantTypeGetWithIdentifier(obj->typeId.detailInfo);
//    if (type) {
//        if (type->deinit) {
//            type->deinit(manager, obj);
//        }
//    } else {
//        abort();
//    }
//    
//}
//void CCCustomDeinitMemoryObjectDeinitDispatch(CObjectManager_t const * _Nonnull manager, void * _Nonnull obj) {
//    assert(obj);
//    
//    CCCustomDeinitMemoryObject_o * object = (CCCustomDeinitMemoryObject_o *)obj;
//    void (* _Nullable deinit)(struct _CObjectManager const * _Nonnull manager, struct __CCCustomDeinitMemoryObject * _Nonnull obj) = object->deinit;
//    if (deinit) {
//        deinit(manager, object);
//    }
//}
//
//
//
//static uint32_t const COConstantTypeBufferSize = 9;
//static COConstantType_t const COConstantTypeBuffer[COConstantTypeBufferSize] = {
//    {CCNullTypeIdentifier, "CCNull", NULL},
//    {CCStringTypeIdentifier, "CCString", NULL},
//    {CCDataTypeIdentifier, "CCData", NULL},
//    {CCSIntegerTypeIdentifier, "CCSInteger", NULL},
//    {CCUIntegerTypeIdentifier, "CCUInteger", NULL},
//    {CCFloat32TypeIdentifier, "CCFloat32", NULL},
//    {CCFloat64TypeIdentifier, "CCFloat64", NULL},
//    {CPMemoryObjectTypeIdentifier, "CPMemoryObject", NULL},
//    {CCCustomDeinitMemoryObjectTypeIdentifier, "CCCustomDeinitMemoryObject", CCCustomDeinitMemoryObjectDeinitDispatch},
//};
//
//
//
//COConstantType_t const * _Nullable COConstantTypeGetWithIdentifier(uint32_t identifier) {
//    if (identifier >= COConstantTypeBufferSize) {
//        return NULL;
//    }
//    return &(COConstantTypeBuffer[identifier]);
//}



