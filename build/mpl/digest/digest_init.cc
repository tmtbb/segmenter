//  Copyright (c) 2016-2016 The segment Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry
#include "digest_init.h"
#include "core/common.h"
#include "core/plugins.h"
#include "digest_logic.h"


struct digestplugin{
    char* id;
    char* name;
    char* version;
    char* provider;
};


static void *OnDigestStart() {
    signal(SIGPIPE, SIG_IGN);
    struct digestplugin* digest = (struct digestplugin*)calloc(1,
            sizeof(struct digestplugin));
    digest->id = "digest";
    digest->name = "digest";
    digest->version = "1.0.0";
    digest->provider = "kerry";
    if (!digest_logic::DigestLogic::GetInstance())
        assert(0);
    return digest;
}

static handler_t OnDigestShutdown(struct server* srv, void* pd) {
	digest_logic::DigestLogic::FreeInstance();

    return HANDLER_GO_ON;
}

static handler_t OnDigestConnect(struct server *srv, int fd, void *data,
        int len) {
	digest_logic::DigestLogic::GetInstance()->OnDigestConnect(srv, fd);
    return HANDLER_GO_ON;
}

static handler_t OnDigestMessage(struct server *srv, int fd, void *data,
        int len) {
	digest_logic::DigestLogic::GetInstance()->OnDigestMessage(srv,
            fd, data, len);
    return HANDLER_GO_ON;
}

static handler_t OnDigestClose(struct server *srv, int fd) {
	digest_logic::DigestLogic::GetInstance()->OnDigestClose(srv, fd);
    return HANDLER_GO_ON;
}

static handler_t OnUnknow(struct server *srv, int fd, void *data,
        int len) {
    return HANDLER_GO_ON;
}

static handler_t OnBroadcastConnect(struct server* srv, int fd,
        void *data, int len) {
	digest_logic::DigestLogic::GetInstance()->OnBroadcastConnect(
            srv, fd, data, len);
    return HANDLER_GO_ON;
}

static handler_t OnBroadcastClose(struct server* srv, int fd) {
	digest_logic::DigestLogic::GetInstance()->OnBroadcastClose(srv, fd);
    return HANDLER_GO_ON;
}

static handler_t OnBroadcastMessage(struct server* srv, int fd, void *data,
        int len) {
	digest_logic::DigestLogic::GetInstance()->OnBroadcastMessage(srv,
            fd, data, len);
    return HANDLER_GO_ON;
}

static handler_t OnIniTimer(struct server* srv) {
	digest_logic::DigestLogic::GetInstance()->OnIniTimer(srv);
    return HANDLER_GO_ON;
}

static handler_t OnTimeOut(struct server* srv, char* id, int opcode, int time) {
	digest_logic::DigestLogic::GetInstance()->OnTimeout(srv,
            id, opcode, time);
    return HANDLER_GO_ON;
}


int digest_plugin_init(struct plugin *pl) {
    pl->init = OnDigestStart;
    pl->clean_up = OnDigestShutdown;
    pl->connection = OnDigestConnect;
    pl->connection_close = OnDigestClose;
    pl->connection_close_srv = OnBroadcastClose;
    pl->connection_srv = OnBroadcastConnect;
    pl->handler_init_time = OnIniTimer;
    pl->handler_read = OnDigestMessage;
    pl->handler_read_srv = OnBroadcastMessage;
    pl->handler_read_other = OnUnknow;
    pl->time_msg = OnTimeOut;
    pl->data = NULL;
    return 0;
}

