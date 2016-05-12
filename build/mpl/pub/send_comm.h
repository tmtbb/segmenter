//  Copyright (c) 2015-2015 The KID Authors. All rights reserved.
//  Created on: 2016年5月3日 Author: kerry

#ifndef ML_SEND_COMM_H__
#define ML_SEND_COMM_H__

#include <sys/socket.h>
#include <string>
#include "basic/basictypes.h"
#include "basic/native_library.h"
#include "ptl_comm_head.h"
#include "logic/logic_comm.h"
#include "basic/basic_util.h"

namespace segmenter_logic {

class SendComm {
public:
	static int32 SendFull(int socket, const char* buffer, size_t bytes);

	static bool SendBytes(int socket, const void* bytes, int32 len,
	            const char* file, int32 line);

	static bool SendMessage(int socket, struct PacketHead* packet,
			const char* file, int32 line);

	static bool SendHeadMessage(int socket, int32 operate_code,
			int32 type, int32 is_zip_encrypt, int64 session,
			int32 reserved, const char* file, int32 line);
};

}

#define send_bytes (socket, bytes, len)\
	segmenter_logic::SendComm::SendBytes(socket, bytes, len, __FILE__, __LINE__)\


#define send_message(socket, packet) \
	segmenter_logic::SendComm::SendMessage(socket, packet, __FILE__, __LINE__)\


#define send_headmsg(socket, operate_code, type, is_zip_encrypt, \
        reserved, session)\
    segmenter_logic::SendComm::SendHeadMessage(socket, operate_code, type, \
            is_zip_encrypt, reserved, session, __FILE__, __LINE__)\

#define closelockconnect(socket) \
    shutdown(socket, SHUT_RDWR);

#endif
