//  Copyright (c) 2015-2015 The KID Authors. All rights reserved.
//  Created on: 2016年5月3日 Author: kerry

#include "send_comm.h"
#include "ptl_packet.h"

namespace segmenter_logic {

int32 SendComm::SendFull(int socket, const char *buffer,
                        size_t nbytes) {
	//if(NULL == SendUtils::socket_lock_)
		//InitThreadrw(&SendUtils::socket_lock_);
	//base_logic::WLockGd lk(SendUtils::socket_lock_);
    ssize_t amt = 0;
    ssize_t total = 0;
    const char *buf = buffer;
    do {
        amt = nbytes;
        amt = send(socket, buf, amt, 0);
        if (-1 == amt) {
            if (11 == errno)
                continue;
            else {
                break;
            }
        }
        buf = buf + amt;
        nbytes -= amt;
        total += amt;
    } while (nbytes > 0);
    return static_cast<int32>(amt == -1 ? amt : total);
}


bool SendComm::SendBytes(int socket, const void* bytes,
        int32 len, const char* file, int32 line) {
    if (socket <= 0 || bytes == NULL || len <= 0)
        return false;

    int32 ret = SendFull(socket, reinterpret_cast<const char*>(bytes), len);
    if (ret != len) {
        LOG_ERROR("Send bytes failed");
        return false;
    }
    return true;
}

bool SendComm::SendMessage(int socket, struct PacketHead* packet,
        const char* file, int32 line) {
    bool r;
    void *packet_stream = NULL;
    int32_t packet_stream_length = 0;
    int ret = 0;
    bool r1 = false;

    struct PacketHead* packet_test = NULL;
    if (socket <= 0 || packet == NULL)
        return false;

    if (ptl::PacketProsess::PacketStream(packet, &packet_stream,
            &packet_stream_length) == false) {
        //LOG_ERROR2("Call PackStream failed in %s:%d", file, line);
        r = false;
        goto MEMFREE;
    }

    if (!ptl::PacketProsess::UnpackStream(packet_stream, packet_stream_length,
    		&packet_test)) {
        LOG_ERROR2("UnpackStream Error socket %d", socket);
        ptl::PacketProsess::HexEncode(packet_stream, packet_stream_length);
        return false;
    }

   ptl::PacketProsess::DumpPacket(
    		const_cast<const struct PacketHead*>(packet_test));
    ret = SendFull(socket, reinterpret_cast<char*>(packet_stream),
            packet_stream_length);
    ptl::PacketProsess::HexEncode(packet_stream, packet_stream_length);
    if (ret != packet_stream_length) {
        //LOG_ERROR2("Sent msg failed in %s:%d", file, line);
        r = false;
        goto MEMFREE;
    } else {
        r = true;
        goto MEMFREE;
    }

MEMFREE:
    char* stream = reinterpret_cast<char*>(packet_stream);
    if (stream) {
        free((void*)stream);
        stream = NULL;
    }
    return r;
}

bool SendComm::SendHeadMessage(int socket, int32 operate_code,
        int32 type, int32 is_zip_encrypt, int64 session,
        int32 reserved, const char* file, int32 line) {
    struct PacketHead packet;
    MAKE_HEAD(packet, operate_code, type, is_zip_encrypt, session, reserved);
    return SendMessage(socket, &packet, file, line);
}

}
