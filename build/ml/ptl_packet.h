//  Copyright (c) 2015-2015 The KID Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry


#ifndef SEGMENTER_ML_PTL_PACKET_H_
#define SEGMENTER_ML_PTL_PACKET_H_

#include <stddef.h>
#include "protocol/data_packet.h"
#include "basic/basictypes.h"
#include "ptl_comm_head.h"


#define PRINT_TITLE(v)   \
    j += snprintf(buf + j, DUMPPACKBUF - j, "\n-------- %s --------\n", v)

#define PRINT_END(v)     \
    j += snprintf(buf + j, DUMPPACKBUF - j, "-------- %s --------\n", v)

#define PRINT_INT(v)    \
    j += snprintf(buf + j, DUMPPACKBUF - j, "\t%s = %d\n", #v, \
            static_cast<int>(v))

#define PRINT_INT64(v)    \
    j += snprintf(buf + j, DUMPPACKBUF - j, "\t%s = %lld\n", #v, (int64_t)v)

#define PRINT_STRING(v) \
    j += snprintf(buf + j, DUMPPACKBUF - j, "\t%s = %s\n", #v, (const char *)v)


// 包含压缩或加密 故需要先把处理段数据处理
#define BUILDHEAD(DATA_LENGTH)\
    data_length = DATA_LENGTH;\
    packet_length = data_length + PACKET_HEAD_LENGTH;\
    packet::DataOutPacket out(false, packet_length\
            - sizeof(int16) - sizeof(int8));\
    out.Write8(type);\
    out.Write16(signature);\
    out.Write16(operate_code);\
    out.Write16(data_length);\
    out.Write32(timestamp);\
    out.Write64(session_id);\
    out.Write32(reserved);\

#define BUILDPACKHEAD()\
    packet::DataInPacket in(reinterpret_cast<char*>(body_stream), \
            body_length);\
    int8 type = in.Read8();\
    int16 signature = in.Read16();\
    int16 operate_code = in.Read16();\
    int16 data_length = in.Read16();\
    int32 timestamp = in.Read32();\
    int64 session_id = in.Read64();\
    int32 reserved = in.Read32();\

#define FILLHEAD()\
    (*packet_head)->packet_length = packet_length;\
    (*packet_head)->is_zip_encrypt = is_zip_encrypt;\
    (*packet_head)->type = type;\
    (*packet_head)->signature = signature;\
    (*packet_head)->operate_code = operate_code;\
    (*packet_head)->data_length = data_length;\
    (*packet_head)->timestamp = timestamp;\
    (*packet_head)->session_id = session_id;\
    (*packet_head)->reserved = reserved;\



#define DUMPHEAD()\
    PRINT_INT(packet_length);\
    PRINT_INT(is_zip_encrypt);\
    PRINT_INT(type);\
    PRINT_INT(signature);\
    PRINT_INT(operate_code);\
    PRINT_INT(data_length);\
    PRINT_INT(timestamp);\
    PRINT_INT64(session_id);\
    PRINT_INT(reserved);\






#define _MAKE_HEAD(head, _packet_length, _operate_code, _data_length, _type, \
        _is_zip_encrypt, _session_id, _reserved)\
    do {\
        head.packet_length = _packet_length;\
        head.is_zip_encrypt = _is_zip_encrypt;\
        head.data_length = _data_length;\
        head.type = _type;\
        head.signature = 0;\
        head.operate_code = _operate_code;\
        head.timestamp = time(NULL);\
        head.session_id = _session_id;\
        head.reserved = _reserved;\
    }while(0)\

#define MAKE_FAIL_HEAD(head, _operate_code, _session_id, \
        _reserved)\
    _MAKE_HEAD(head, PACKET_HEAD_LENGTH, _operate_code, 0, 0, 0, _session_id, \
        _reserved)

#define MAKE_HEAD(head, _operate_code, _type, _is_zip_encrypt, \
        _session_id, _reserved)\
    _MAKE_HEAD(head, 0, _operate_code, 0, _type, _is_zip_encrypt, _session_id, \
        _reserved)


namespace ptl {

class PacketProsess{
 public:
    static bool PacketStream(const struct PacketHead* packet_head,
        void** packet_stream, int32* packet_stream_length);

    static bool UnpackStream(const void* packet_stream, int32 len,
            struct PacketHead** packet_head);

    static void HexEncode(const void* bytes, size_t size);

    static void DumpPacket(const struct PacketHead* packet_head);

    static void DeletePacket(const void* packet_stream, int32 len,
    		struct PacketHead* packet_head);

    static void ClearWordList(struct PacketHead* packet_head);

 private:
    static uint64 CompressionStream(const uint8* unzip_data, uint64 unzip_len,
            uint8** zip_data);  //  压缩

    static uint64 DecompressionStream(const uint8* zip_data, uint64 zip_len,
            uint8** unzip_data);  //  解压缩
};

}
#endif
