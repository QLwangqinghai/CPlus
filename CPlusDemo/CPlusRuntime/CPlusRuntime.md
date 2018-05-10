# CPlusRuntime

## 内存布局

| 字段名 |  可选  |    字节长度    | 备注 | 
| :---: | :--: | :--------------: | :-----------: |
| customInfo | Y | 变长，取决于Type的 customInfoSize * CPContentAligentBlock|  |
| contentSize | Y | 0 或 CPContentAligentBlock|  |
| type | N | sizeof(void \*） | 必须非空 | 
| active | N | CPContentAligentBlock |  |


## 生命周期

### autoDealloc

Alloc
release prepareDealloc
callback->didPrepareDealloc
deallocing
callback->willDealloc
callback->deinit





### not autoDealloc


Alloc
release
callback->didResignActive









