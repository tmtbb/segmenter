//  Copyright (c) 2016-2016 The segmenter Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry

#ifndef DIGEST_MPL_LOGIC_
#define DIGEST_MPL_LOGIC_

#include "ptl_comm_head.h"
#include "ptl_packet.h"
#include "Python.h"
#include "core/common.h"
#include "basic/basictypes.h"
#include "basic/scoped_ptr.h"


namespace digest_logic {


class DigestLogic {
 public:
	DigestLogic();
    virtual ~DigestLogic();

 private:
    static DigestLogic    *instance_;

 public:
    static DigestLogic *GetInstance();
    static void FreeInstance();

 public:
    /*
    bool InitPython();

    bool PrintDict(PyObject* obj);
    */

    bool OnDigestConnect(struct server *srv, const int socket);

    bool OnDigestMessage(struct server *srv, const int socket,
            const void *msg, const int len);

    bool OnDigestClose(struct server *srv, const int socket);

    bool OnBroadcastConnect(struct server *srv, const int socket,
            const void *data, const int len);

    bool OnBroadcastMessage(struct server *srv, const int socket,
            const void *msg, const int len);

    bool OnBroadcastClose(struct server *srv, const int socket);

    bool OnIniTimer(struct server *srv);

    bool OnTimeout(struct server *srv, char* id, int opcode, int time);


 private:
    bool Init();

    bool OnArticleDigestUnit(struct server* srv, int socket,
            struct PacketHead *packet, const void *msg = NULL,
            int32 len = 0);

    bool OnArticleDigestEnd(struct server* srv, int socket,
                struct PacketHead *packet, const void *msg = NULL,
                int32 len = 0);
};

}  // namespace digest_logic

#endif

