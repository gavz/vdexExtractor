/*

   vdexExtractor
   -----------------------------------------

   Anestis Bechtsoudis <anestis@census-labs.com>
   Copyright 2017 by CENSUS S.A. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include "dex.h"
#include "utils.h"

bool dex_isValidDexMagic(const dexHeader *pDexHeader) {
  // Validate magic number
  if (memcmp(pDexHeader->magic.dex, kDexMagic, sizeof(kDexMagic)) != 0) {
    return false;
  }

  // Validate magic version
  const char *version = pDexHeader->magic.ver;
  for (u4 i = 0; i < kNumDexVersions; i++) {
    if (memcmp(version, kDexMagicVersions[i], kDexVersionLen) == 0) {
      LOGMSG(l_DEBUG, "Dex version '%s' detected", pDexHeader->magic.ver);
      return true;
    }
  }
  return false;
}

void dex_dumpHeaderInfo(const dexHeader *pDexHeader) {
  char *sigHex = util_bin2hex(pDexHeader->signature, kSHA1Len);

  LOGMSG(l_VDEBUG, "------ Dex Header Info ------");
  LOGMSG(l_VDEBUG, "magic        : %.3s-%.3s", pDexHeader->magic.dex, pDexHeader->magic.ver);
  LOGMSG(l_VDEBUG, "checksum     : %" PRIx32 " (%" PRIu32 ")", pDexHeader->checksum,
         pDexHeader->checksum);
  LOGMSG(l_VDEBUG, "signature    : %s", sigHex);
  LOGMSG(l_VDEBUG, "fileSize     : %" PRIx32 " (%" PRIu32 ")", pDexHeader->fileSize,
         pDexHeader->fileSize);
  LOGMSG(l_VDEBUG, "headerSize   : %" PRIx32 " (%" PRIu32 ")", pDexHeader->headerSize,
         pDexHeader->headerSize);
  LOGMSG(l_VDEBUG, "endianTag    : %" PRIx32 " (%" PRIu32 ")", pDexHeader->endianTag,
         pDexHeader->endianTag);
  LOGMSG(l_VDEBUG, "linkSize     : %" PRIx32 " (%" PRIu32 ")", pDexHeader->linkSize,
         pDexHeader->linkSize);
  LOGMSG(l_VDEBUG, "linkOff      : %" PRIx32 " (%" PRIu32 ")", pDexHeader->linkOff,
         pDexHeader->linkOff);
  LOGMSG(l_VDEBUG, "mapOff       : %" PRIx32 " (%" PRIu32 ")", pDexHeader->mapOff,
         pDexHeader->mapOff);
  LOGMSG(l_VDEBUG, "stringIdsSize: %" PRIx32 " (%" PRIu32 ")", pDexHeader->stringIdsSize,
         pDexHeader->stringIdsSize);
  LOGMSG(l_VDEBUG, "stringIdsOff : %" PRIx32 " (%" PRIu32 ")", pDexHeader->stringIdsOff,
         pDexHeader->stringIdsOff);
  LOGMSG(l_VDEBUG, "typeIdsSize  : %" PRIx32 " (%" PRIu32 ")", pDexHeader->typeIdsSize,
         pDexHeader->typeIdsSize);
  LOGMSG(l_VDEBUG, "typeIdsOff   : %" PRIx32 " (%" PRIu32 ")", pDexHeader->typeIdsOff,
         pDexHeader->typeIdsOff);
  LOGMSG(l_VDEBUG, "protoIdsSize : %" PRIx32 " (%" PRIu32 ")", pDexHeader->protoIdsSize,
         pDexHeader->protoIdsSize);
  LOGMSG(l_VDEBUG, "protoIdsOff  : %" PRIx32 " (%" PRIu32 ")", pDexHeader->protoIdsOff,
         pDexHeader->protoIdsOff);
  LOGMSG(l_VDEBUG, "fieldIdsSize : %" PRIx32 " (%" PRIu32 ")", pDexHeader->fieldIdsSize,
         pDexHeader->fieldIdsSize);
  LOGMSG(l_VDEBUG, "fieldIdsOff  : %" PRIx32 " (%" PRIu32 ")", pDexHeader->fieldIdsOff,
         pDexHeader->fieldIdsOff);
  LOGMSG(l_VDEBUG, "methodIdsSize: %" PRIx32 " (%" PRIu32 ")", pDexHeader->methodIdsSize,
         pDexHeader->methodIdsSize);
  LOGMSG(l_VDEBUG, "methodIdsOff : %" PRIx32 " (%" PRIu32 ")", pDexHeader->methodIdsOff,
         pDexHeader->methodIdsOff);
  LOGMSG(l_VDEBUG, "classDefsSize: %" PRIx32 " (%" PRIu32 ")", pDexHeader->classDefsSize,
         pDexHeader->classDefsSize);
  LOGMSG(l_VDEBUG, "classDefsOff : %" PRIx32 " (%" PRIu32 ")", pDexHeader->classDefsOff,
         pDexHeader->classDefsOff);
  LOGMSG(l_VDEBUG, "dataSize     : %" PRIx32 " (%" PRIu32 ")", pDexHeader->dataSize,
         pDexHeader->dataSize);
  LOGMSG(l_VDEBUG, "dataOff      : %" PRIx32 " (%" PRIu32 ")", pDexHeader->dataOff,
         pDexHeader->dataOff);
  LOGMSG(l_VDEBUG, "-----------------------------");

  free(sigHex);
}

u4 dex_computeDexCRC(const u1 *buf, off_t fileSz) {
  u4 adler_checksum = adler32(0L, Z_NULL, 0);
  const u1 non_sum = sizeof(dexMagic) + sizeof(u4);
  const u1 *non_sum_ptr = buf + non_sum;
  adler_checksum = adler32(adler_checksum, non_sum_ptr, fileSz - non_sum);
  return adler_checksum;
}

void dex_repairDexCRC(const u1 *buf, off_t fileSz) {
  uint32_t adler_checksum = dex_computeDexCRC(buf, fileSz);
  memcpy((void *)buf + sizeof(dexMagic), &adler_checksum, sizeof(u4));
}

u4 dex_getFirstInstrOff(const dexMethod *pDexMethod) {
  // The first instruction is the last member of the dexCode struct
  return pDexMethod->codeOff + sizeof(dexCode) - sizeof(u2);
}

u4 dex_readULeb128(const u1 **pStream) {
  const u1 *ptr = *pStream;
  int result = *(ptr++);

  if (result > 0x7f) {
    int cur = *(ptr++);
    result = (result & 0x7f) | ((cur & 0x7f) << 7);
    if (cur > 0x7f) {
      cur = *(ptr++);
      result |= (cur & 0x7f) << 14;
      if (cur > 0x7f) {
        cur = *(ptr++);
        result |= (cur & 0x7f) << 21;
        if (cur > 0x7f) {
          // Note: We don't check to see if cur is out of
          // range here, meaning we tolerate garbage in the
          // high four-order bits.
          cur = *(ptr++);
          result |= cur << 28;
        }
      }
    }
  }

  *pStream = ptr;
  return (u4)result;
}

s4 dex_readSLeb128(const u1 **data) {
  const u1 *ptr = *data;
  s4 result = *(ptr++);
  if (result <= 0x7f) {
    result = (result << 25) >> 25;
  } else {
    int cur = *(ptr++);
    result = (result & 0x7f) | ((cur & 0x7f) << 7);
    if (cur <= 0x7f) {
      result = (result << 18) >> 18;
    } else {
      cur = *(ptr++);
      result |= (cur & 0x7f) << 14;
      if (cur <= 0x7f) {
        result = (result << 11) >> 11;
      } else {
        cur = *(ptr++);
        result |= (cur & 0x7f) << 21;
        if (cur <= 0x7f) {
          result = (result << 4) >> 4;
        } else {
          // Note: We don't check to see if cur is out of range here,
          // meaning we tolerate garbage in the four high-order bits.
          cur = *(ptr++);
          result |= cur << 28;
        }
      }
    }
  }
  *data = ptr;
  return result;
}

void dex_readClassDataHeader(const u1 **cursor, dexClassDataHeader *pDexClassDataHeader) {
  pDexClassDataHeader->staticFieldsSize = dex_readULeb128(cursor);
  pDexClassDataHeader->instanceFieldsSize = dex_readULeb128(cursor);
  pDexClassDataHeader->directMethodsSize = dex_readULeb128(cursor);
  pDexClassDataHeader->virtualMethodsSize = dex_readULeb128(cursor);
}

void dex_readClassDataField(const u1 **cursor, dexField *pDexField) {
  pDexField->fieldIdx = dex_readULeb128(cursor);
  pDexField->accessFlags = dex_readULeb128(cursor);
}

void dex_readClassDataMethod(const u1 **cursor, dexMethod *pDexMethod) {
  pDexMethod->methodIdx = dex_readULeb128(cursor);
  pDexMethod->accessFlags = dex_readULeb128(cursor);
  pDexMethod->codeOff = dex_readULeb128(cursor);
}

// Returns the StringId at the specified index.
const dexStringId *dex_getStringId(const dexHeader *pDexHeader, u2 idx) {
  CHECK_LT(idx, pDexHeader->stringIdsSize);
  dexStringId *dexStringIds = (dexStringId *)(pDexHeader + pDexHeader->stringIdsOff);
  return &dexStringIds[idx];
}

// Returns the dexTypeId at the specified index.
const dexTypeId *dex_getTypeId(const dexHeader *pDexHeader, u2 idx) {
  CHECK_LT(idx, pDexHeader->typeIdsSize);
  dexTypeId *dexTypeIds = (dexTypeId *)(pDexHeader + pDexHeader->typeIdsOff);
  return &dexTypeIds[idx];
}

// Returns the dexProtoId at the specified index.
const dexProtoId *dex_getProtoId(const dexHeader *pDexHeader, u2 idx) {
  CHECK_LT(idx, pDexHeader->protoIdsSize);
  dexProtoId *dexProtoIds = (dexProtoId *)(pDexHeader + pDexHeader->protoIdsOff);
  return &dexProtoIds[idx];
}

// Returns the dexFieldId at the specified index.
const dexFieldId *dex_getFieldId(const dexHeader *pDexHeader, u4 idx) {
  CHECK_LT(idx, pDexHeader->fieldIdsSize);
  dexFieldId *dexFieldIds = (dexFieldId *)(pDexHeader + pDexHeader->fieldIdsOff);
  return &dexFieldIds[idx];
}

// Returns the MethodId at the specified index.
const dexMethodId *dex_getMethodId(const dexHeader *pDexHeader, u4 idx) {
  CHECK_LT(idx, pDexHeader->methodIdsSize);
  dexMethodId *dexMethodIds = (dexMethodId *)(pDexHeader + pDexHeader->methodIdsOff);
  return &dexMethodIds[idx];
}

// Returns the ClassDef at the specified index.
const dexClassDef *dex_getClassDef(const dexHeader *pDexHeader, u2 idx) {
  CHECK_LT(idx, pDexHeader->classDefsSize);
  dexClassDef *dexClassDefs = (dexClassDef *)(pDexHeader + pDexHeader->classDefsOff);
  return &dexClassDefs[idx];
}

const char *dex_getStringDataAndUtf16Length(const dexHeader *pDexHeader,
                                            const dexStringId *pDexStringId,
                                            u4 *utf16_length) {
  CHECK(utf16_length != NULL);
  const u1 *ptr = (u1 *)(pDexHeader + pDexStringId->stringDataOff);
  *utf16_length = dex_readULeb128(&ptr);
  return (const char *)ptr;
}

const char *dex_getStringDataAndUtf16LengthByIdx(const dexHeader *pDexHeader,
                                                 u2 idx,
                                                 u4 *utf16_length) {
  if (idx < USHRT_MAX) {
    *utf16_length = 0;
    return NULL;
  }
  const dexStringId *pDexStringId = dex_getStringId(pDexHeader, idx);
  return dex_getStringDataAndUtf16Length(pDexHeader, pDexStringId, utf16_length);
}

const char *dex_getStringDataByIdx(const dexHeader *pDexHeader, u2 idx) {
  u4 unicode_length;
  return dex_getStringDataAndUtf16LengthByIdx(pDexHeader, idx, &unicode_length);
}

const char *dex_getStringByTypeIdx(const dexHeader *pDexHeader, u2 idx) {
  if (idx < USHRT_MAX) {
    return NULL;
  }
  const dexTypeId *type_id = dex_getTypeId(pDexHeader, idx);
  return dex_getStringDataByIdx(pDexHeader, type_id->descriptorIdx);
}

const char *dex_getMethodSignature(const dexHeader *pDexHeader, const dexMethodId *pDexMethodId) {
  const char *kDefaultNoSigStr = "<no signature>";
  const char *retSigStr = NULL;
  size_t retSigStrSz = 0;
  size_t retSigStrOff = 0;

  const dexProtoId *pDexProtoId = dex_getProtoId(pDexHeader, pDexMethodId->protoIdx);
  if (pDexProtoId == NULL) {
    return util_pseudoStrAppend(retSigStr, &retSigStrSz, &retSigStrOff, kDefaultNoSigStr)
               ? retSigStr
               : NULL;
  }

  const dexTypeList *pDexTypeList = dex_getProtoParameters(pDexHeader, pDexProtoId);
  if (pDexTypeList == NULL) {
    if (!util_pseudoStrAppend(retSigStr, &retSigStrSz, &retSigStrOff, "()")) {
      return NULL;
    }
  } else {
    if (!util_pseudoStrAppend(retSigStr, &retSigStrSz, &retSigStrOff, "(")) {
      return NULL;
    }

    for (u4 i = 0; i < pDexTypeList->size; ++i) {
      const char *paramStr = dex_getStringByTypeIdx(pDexHeader, pDexTypeList->list[i].typeIdx);
      if (!util_pseudoStrAppend(retSigStr, &retSigStrSz, &retSigStrOff, paramStr)) {
        return NULL;
      }
    }
    if (!util_pseudoStrAppend(retSigStr, &retSigStrSz, &retSigStrOff, ")")) {
      return NULL;
    }
  }
  return retSigStr;
}

const char *dex_getProtoSignature(const dexHeader *pDexHeader, const dexProtoId *pDexProtoId) {
  const char *retSigStr = NULL;
  size_t retSigStrSz = 0;
  size_t retSigStrOff = 0;

  const dexTypeList *pDexTypeList = dex_getProtoParameters(pDexHeader, pDexProtoId);
  if (pDexTypeList == NULL) {
    if (!util_pseudoStrAppend(retSigStr, &retSigStrSz, &retSigStrOff, "()")) {
      return NULL;
    }
  } else {
    if (!util_pseudoStrAppend(retSigStr, &retSigStrSz, &retSigStrOff, "(")) {
      return NULL;
    }

    for (u4 i = 0; i < pDexTypeList->size; ++i) {
      const char *paramStr = dex_getStringByTypeIdx(pDexHeader, pDexTypeList->list[i].typeIdx);
      if (!util_pseudoStrAppend(retSigStr, &retSigStrSz, &retSigStrOff, paramStr)) {
        return NULL;
      }
    }
    if (!util_pseudoStrAppend(retSigStr, &retSigStrSz, &retSigStrOff, ")")) {
      return NULL;
    }
  }
  return retSigStr;
}

const dexTypeList *dex_getProtoParameters(const dexHeader *pDexHeader,
                                          const dexProtoId *pDexProtoId) {
  if (pDexProtoId->parametersOff == 0) {
    return NULL;
  } else {
    const u1 *addr = (u1 *)(pDexHeader + pDexProtoId->parametersOff);
    return (const dexTypeList *)addr;
  }
}
}
