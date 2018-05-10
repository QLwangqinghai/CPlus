//
//  CObjectBase.c
//  O
//
//  Created by wangqinghai on 2018/3/16.
//  Copyright © 2018年 wangqinghai. All rights reserved.
//

#include "CPlusRuntimePrivate.h"



static CPTypeLayout_s const CPTypeStorage_CCNull = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 0,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CCNull_o),
            .name = "CCNull",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = NULL,
        },
        .callbacks = NULL,
        .context = NULL,
    },
};

CPType_s const * CPType_CCNull = &(CPTypeStorage_CCNull.type);

typedef struct {
    CPInfo_s info;
    CCNull_o content;
} CCNullStorage_t;

static CCNullStorage_t const CCNullStorageShare = {
    .info =  {
        .type =  &(CPTypeStorage_CCNull.type),
        .activeInfo = {
            .contentSize = 0,
            .deallocing = 0,
            .prepareDealloc = 0,
            .autoDealloc = 0,
            .refrenceCount = CPRefrenceStaticFlag,
        }
    },
    .content = {
        .__unused__ = 0,
    }
};
static CCNull_o const * _Nonnull const __CCShareNull = &(CCNullStorageShare.content);

CCNull_o const * _Nonnull CCNullShare(void) {
    return __CCShareNull;
}



static CPTypeLayout_s const CPTypeStorage_CCString = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 1,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CCString_o),
            .name = "CCString",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = NULL,
        },
        .callbacks = NULL,
        .context = NULL,
    },
};
CPType _Nonnull CPType_CCString = &(CPTypeStorage_CCString.type);


typedef struct {
    CPInfo_s info;
    CCString_o content;
    uint64_t footer;
} CCStringStorage_t;
static CCStringStorage_t const CCStringStorageShareNullString = {
    .info =  {
        .type =  &(CPTypeStorage_CCString.type),
        .activeInfo = {
            .contentSize = 0,
            .deallocing = 0,
            .prepareDealloc = 0,
            .autoDealloc = 0,
            .refrenceCount = CPRefrenceStaticFlag,
        }
    },
    .content = {
        .privateValue = 0,
        .length = 0,
        .cUtf8StringLength = 0,
    },
    .footer = 0,
};
static CCString_o const * _Nonnull const __CCShareNullString = &(CCStringStorageShareNullString.content);


static uint64_t const CCStringMaxStringContent = CPMaxContentSize - sizeof(CCString_o) - 1;
CCString_o const * _Nonnull CCStringInitBase(char const * _Nonnull cUtf8String, uint64_t cUtf8Length, uint32_t length, uint32_t hash, _Bool autoDealloc, _Bool isStatic) {
    CPType Self = CPType_CCString;
    CPAlloctor_s const * alloctor = (CPAlloctor_s const *)(Self->base.alloctor);
    if (NULL == alloctor) {
        alloctor = CPAlloctorGetDefault();
    }
    void * ptr = alloctor->allocInit(alloctor, Self, cUtf8Length + 1, autoDealloc, isStatic);
    CCString_o * str = (CCString_o *)ptr;
    *((uint64_t *)&(str->cUtf8StringLength)) = (uint64_t)cUtf8Length;
    *((uint32_t *)&(str->length)) = (uint32_t)length;
    str->privateValue = 0;
    memcpy((void *)&(str->cUtf8String[0]), cUtf8String, cUtf8Length);
    *((char *)&(str->cUtf8String[cUtf8Length])) = 0;
    return str;
}
CCString_o const * _Nonnull CCStringInit(char const * _Nullable cUtf8String) {
    if (NULL == cUtf8String) {
        if (NULL == __CCShareNullString) {
            abort();
        }
        return __CCShareNullString;
    }
    size_t length = strlen(cUtf8String);
    if (length > CCStringMaxStringContent) {
        abort();
    }
    if (length == 0) {
        if (NULL == __CCShareNullString) {
            abort();
        }
        return __CCShareNullString;
    }

    size_t validByteCount;
    size_t scalarCount = 0;
    if (scalarCount > UINT32_MAX) {
        abort();
    }
    if (!CStringCheckUtf8StringWithLength(cUtf8String, length, &scalarCount, &validByteCount)) {
        abort();
    }
    
    return CCStringInitBase(cUtf8String, validByteCount, (uint32_t)scalarCount, 0, 1, 0);
}
CCString_o const * _Nullable CCStringInitWithUtf8Byte(uint8_t const * _Nullable byte, uint32_t byteLength) {
    if (NULL == byte || byteLength == 0) {
        if (NULL == __CCShareNullString) {
            abort();
        }
        return __CCShareNullString;
    }
    
    size_t validByteCount;
    size_t scalarCount = 0;
    if (!CStringCheckUtf8Byte(byte, byteLength, &scalarCount, &validByteCount)) {
        return NULL;
    }
    if (validByteCount > CCStringMaxStringContent) {
        abort();
    }
    if (validByteCount == 0) {
        if (NULL == __CCShareNullString) {
            abort();
        }
        return __CCShareNullString;
    }
    return CCStringInitBase((char const *)byte, validByteCount, (uint32_t)scalarCount, 0, 1, 0);
}

uint32_t CCStringGetHashCode(CCString_o * _Nonnull self) {
    assert(self);
    
    
    return self->privateValue;
}
char const * _Nonnull CCStringGetCString(CCString_o const * _Nonnull self) {
    assert(self);
    
    
    return &(self->cUtf8String[0]);
}




static CPTypeLayout_s const CPTypeStorage_CCData = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 1,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CCData_o),
            .name = "CCData",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = NULL,
        },
        .callbacks = NULL,
        .context = NULL,
    }
};

CPType _Nonnull CPType_CCData = &(CPTypeStorage_CCData.type);

typedef struct {
    CPInfo_s info;
    CCData_o content;
    uint64_t footer;
} CCDataStorage_t;
static CCDataStorage_t const CCDataStorageShareNullData = {
    .info =  {
        .type =  &(CPTypeStorage_CCData.type),
        .activeInfo = {
            .contentSize = 0,
            .deallocing = 0,
            .prepareDealloc = 0,
            .autoDealloc = 0,
            .refrenceCount = CPRefrenceStaticFlag,
        }
    },
    .content = {
        .privateValue = 0,
        .byteLength = 0,
    },
    .footer = 0,
};

static CCData_o const * _Nonnull const __CCShareNullData = &(CCDataStorageShareNullData.content);

CCData_o * _Nonnull CCStringGetUtf8Data(CCString_o const * _Nonnull string) {
    assert(string);
    CCData_o * data = CCDataInit((uint8_t const *)&(string->cUtf8String[0]), string->cUtf8StringLength);
    CPAutorelease(data);
    return data;
}
CCData_o * _Nullable CCDataInit(uint8_t const * _Nullable byte, uint64_t byteLength) {
    CPType Self = CPType_CCData;

    if (byteLength == 0) {
        return __CCShareNullData;
    }
    if (byteLength > CPMaxContentSize - sizeof(CCData_o)) {
        abort();
    }
    CPAlloctor_s const * alloctor = (CPAlloctor_s const *)(Self->base.alloctor);
    if (NULL == alloctor) {
        alloctor = CPAlloctorGetDefault();
    }
    void * ptr = alloctor->allocInit(alloctor, Self, byteLength, 1, 0);
    CCData_o * data = (CCData_o *)ptr;
    *((uint64_t *)&(data->byteLength)) = (uint64_t)byteLength;
    data->privateValue = 0;
    memcpy((void *)&(data->byte[0]), byte, byteLength);
    return data;
}

static CPTypeLayout_s const CPTypeStorage_CCSInteger = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 0,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CCSInteger_o),
            .name = "CCSInteger",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = NULL,
        },
        .callbacks = NULL,
        .context = NULL,
    }
};
CPType _Nonnull CPType_CCSInteger = &(CPTypeStorage_CCSInteger.type);

static CPTypeLayout_s const CPTypeStorage_CCUInteger = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 0,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CCUInteger_o),
            .name = "CCUInteger",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = NULL,
        },
        .callbacks = NULL,
        .context = NULL,
    }
};
CPType _Nonnull CPType_CCUInteger = &(CPTypeStorage_CCUInteger.type);


static CPTypeLayout_s const CPTypeStorage_CCFloat32 = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 0,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CCFloat32_o),
            .name = "CCFloat32",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = NULL,
        },
        .callbacks = NULL,
        .context = NULL,
    }
};
CPType _Nonnull CPType_CCFloat32 = &(CPTypeStorage_CCFloat32.type);


static CPTypeLayout_s const CPTypeStorage_CCFloat64 = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 0,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CCFloat64_o),
            .name = "CCFloat64",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = NULL,
        },
        .callbacks = NULL,
        .context = NULL,
    }
};
CPType _Nonnull CPType_CCFloat64 = &(CPTypeStorage_CCFloat64.type);


CCSInteger_o * _Nonnull CCSIntegerInit(int64_t high, uint64_t low) {
    CPType Self = CPType_CCSInteger;
    CPAlloctor_s const * alloctor = (CPAlloctor_s const *)(Self->base.alloctor);
    if (NULL == alloctor) {
        alloctor = CPAlloctorGetDefault();
    }
    void * ptr = alloctor->allocInit(alloctor, Self, 0, 1, 0);
    CCSInteger_o * obj = (CCSInteger_o *)ptr;
    obj->high = high;
    obj->low = low;
    return obj;
}
CCSInteger_o * _Nonnull CCSIntegerInitWithSInt64(int64_t value) {
    int64_t high = 0;
    uint64_t low = 0;

    if (value < 0) {
        high = -1;
        low = value * -1;
    } else {
        high = 0;
        low = value;
    }
    return CCSIntegerInit(high, low);
}

CCSInteger_o * _Nonnull CCSIntegerInitWithUInt64(uint64_t value) {
    return CCSIntegerInit(0, value);
}

CCUInteger_o * _Nonnull CCUIntegerInit(uint64_t high, uint64_t low) {
    CPType Self = CPType_CCUInteger;
    CPAlloctor_s const * alloctor = (CPAlloctor_s const *)(Self->base.alloctor);
    if (NULL == alloctor) {
        alloctor = CPAlloctorGetDefault();
    }
    void * ptr = alloctor->allocInit(alloctor, Self, 0, 1, 0);
    
    CCUInteger_o * obj = (CCUInteger_o *)ptr;
    obj->high = high;
    obj->low = low;
    return obj;
}
CCUInteger_o * _Nonnull CCUIntegerInitUInt64(uint64_t value) {
    return CCUIntegerInit(0, value);
}
CCFloat32_o * _Nonnull CCFloat32Init(float32_t value) {
    CPType Self = CPType_CCFloat32;
    CPAlloctor_s const * alloctor = (CPAlloctor_s const *)(Self->base.alloctor);
    if (NULL == alloctor) {
        alloctor = CPAlloctorGetDefault();
    }
    void * ptr = alloctor->allocInit(alloctor, Self, 0, 1, 0);
    CCFloat32_o * obj = (CCFloat32_o *)ptr;
    obj->content = value;
    return obj;
}

CCFloat64_o * _Nonnull CCFloat64Init(float64_t value) {
    CPType Self = CPType_CCFloat64;
    CPAlloctor_s const * alloctor = (CPAlloctor_s const *)(Self->base.alloctor);
    if (NULL == alloctor) {
        alloctor = CPAlloctorGetDefault();
    }
    void * ptr = alloctor->allocInit(alloctor, Self, 0, 1, 0);
    CCFloat64_o * obj = (CCFloat64_o *)ptr;
    obj->content = value;
    return obj;
}


static CPTypeLayout_s const CPTypeStorage_CPMemory = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 0,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CPMemory_o),
            .name = "CPMemory",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = NULL,
        },
        .callbacks = NULL,
        .context = NULL,
    }
};
CPType _Nonnull CPType_CPMemory = &(CPTypeStorage_CPMemory.type);

CPMemory_o * _Nonnull CPMemoryInit(uint32_t byteLength) {
    if (byteLength > CPMaxContentSize - sizeof(CPMemory_o)) {
        abort();
    }
    
    CPType Self = CPType_CPMemory;
    CPAlloctor_s const * alloctor = (CPAlloctor_s const *)(Self->base.alloctor);
    if (NULL == alloctor) {
        alloctor = CPAlloctorGetDefault();
    }
    void * ptr = alloctor->allocInit(alloctor, Self, 0, 1, 0);
    
    CPMemory_o * obj = (CPMemory_o *)ptr;
    *((uint32_t *)&(obj->contentSize)) = (uint32_t)byteLength;
    return obj;
}

void CCCustomDeinitMemoryDeinit(void * _Nonnull object);

static CPTypeLayout_s const CPTypeStorage_CCCustomDeinitMemory = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 1,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CCCustomDeinitMemory_o),
            .name = "CCCustomDeinitMemory",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = CCCustomDeinitMemoryDeinit,
        },
        .callbacks = NULL,
        .context = NULL,
    }
};
CPType _Nonnull CPType_CCCustomDeinitMemory = &(CPTypeStorage_CCCustomDeinitMemory.type);


void CCCustomDeinitMemoryDeinit(void * _Nonnull object) {
    assert(object);
    CCCustomDeinitMemory_o * obj = (CCCustomDeinitMemory_o *)object;
    if (obj->deinit) {
        obj->deinit(obj);
    }
}

CCCustomDeinitMemory_o * _Nonnull CCCustomDeinitMemoryInit(uint32_t byteLength, void (* _Nullable deinit)(struct __CCCustomDeinitMemory * _Nonnull obj)) {
    if (byteLength > CPMaxContentSize - sizeof(CCCustomDeinitMemory_o)) {
        abort();
    }
    CPType Self = CPType_CCCustomDeinitMemory;
    CPAlloctor_s const * alloctor = (CPAlloctor_s const *)(Self->base.alloctor);
    if (NULL == alloctor) {
        alloctor = CPAlloctorGetDefault();
    }
    void * ptr = alloctor->allocInit(alloctor, Self, (size_t)byteLength, 1, 0);
    CCCustomDeinitMemory_o * obj = (CCCustomDeinitMemory_o *)ptr;
    obj->deinit = deinit;
    *((uint32_t *)&(obj->contentSize)) = (uint32_t)byteLength;
    return obj;
}









