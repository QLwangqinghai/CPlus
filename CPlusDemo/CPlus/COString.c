//
//  COString.c
//  CObject
//
//  Created by wangqinghai on 2018/3/20.
//

#include "CPlusRuntimePrivate.h"


_Bool CStringScalarInit(CStringScalar * _Nonnull scalar, uint32_t code) {
    uint32_t flag = 0x80000000;
    if ((code & flag) == flag) {
        return 0;
    } else {
        scalar->bigchar = code;
        return 1;
    }
}

//返回 CStringScalar 个数
_Bool CStringCheckUtf8Byte(const uint8_t * _Nonnull utf8StringByte, size_t length, size_t * _Nullable scalarCountRef, size_t * _Nullable validByteCountRef) {
    /*
     1字节 0xxxxxxx
     2字节 110xxxxx 10xxxxxx
     3字节 1110xxxx 10xxxxxx 10xxxxxx
     4字节 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     5字节 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
     6字节 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
     */
    
    size_t index = 0;
    size_t scalarCount = 0;
    uint8_t charByte = 0;
    int64_t lastValidByteIndex = -1;
    
    while (index < length) {
        charByte = *(utf8StringByte + index);
        
        if ((charByte & 0x80) == 0) {//单字节
            scalarCount += 1;
            index++;
            if (charByte != 0) {
                lastValidByteIndex = index;
            }
        } else {//多字节
            uint32_t uchar = 0;
            int scalarByteCount = 0;//字节个数
            while ((charByte & 0x80) != 0) {//获取字节个数
                charByte <<= 1;
                scalarByteCount++;
            }
            if ((scalarByteCount < 2) || (scalarByteCount > 6)) {//字节字数合法性校验
                if (scalarCountRef) {
                    *scalarCountRef = 0;
                }
                
                return 0;
            }
            scalarByteCount -= 1; //减去自身占的一个字节
            index++;
            
            if (length - index < scalarByteCount) {
                if (scalarCountRef) {
                    *scalarCountRef = 0;
                }
                if (validByteCountRef) {
                    *validByteCountRef = 0;
                }
                return 0;
            }
            if (scalarByteCount == 2) {
                uchar += (uint32_t)(charByte & 0x3F);
            } else if (scalarByteCount == 3) {
                uchar += (uint32_t)(charByte & 0x1F);
            } else if (scalarByteCount == 4) {
                uchar += (uint32_t)(charByte & 0xF);
            } else if (scalarByteCount == 5) {
                uchar += (uint32_t)(charByte & 0x7);
            } else if (scalarByteCount == 6) {
                uchar += (uint32_t)(charByte & 0x3);
            }
            
            int64_t tmpValidByteIndex = -1;
            while (scalarByteCount > 0) {
                charByte = *(utf8StringByte + index);
                if ((charByte & 0xC0) != 0x80) {
                    if (scalarCountRef) {
                        *scalarCountRef = 0;
                    }
                    if (validByteCountRef) {
                        *validByteCountRef = 0;
                    }
                    return 0;
                }
                scalarByteCount--;
                uchar = uchar << 6;
                uchar += (charByte & 0x3F);
                if (uchar != 0) {
                    tmpValidByteIndex = index;
                }
                index++;
            }
            if (tmpValidByteIndex >= 0) {
                lastValidByteIndex = tmpValidByteIndex;
            }
            scalarCount += 1;
        }
    }
    if (scalarCountRef) {
        *scalarCountRef = scalarCount;
    }
    if (validByteCountRef) {
        *validByteCountRef = lastValidByteIndex;
    }
    return 1;
}
_Bool CStringCheckUtf8String(const char * _Nonnull utf8String, size_t * _Nullable scalarCount, size_t * _Nullable validByteCountRef) {
    size_t length = strlen(utf8String);
    return CStringCheckUtf8StringWithLength(utf8String, length, scalarCount, validByteCountRef);
}
_Bool CStringCheckUtf8StringWithLength(const char * _Nonnull utf8String, size_t length, size_t * _Nullable scalarCount, size_t * _Nullable validByteCountRef) {
    return CStringCheckUtf8Byte((const uint8_t *)utf8String, length, scalarCount, validByteCountRef);
}

