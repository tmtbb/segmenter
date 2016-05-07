//  Copyright (c) 2016-2016 The segment Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry
#include "segmenter_init.h"
#include "core/common.h"
#include "core/plugins.h"
#include "segmenter_logic.h"


struct segmenterplugin{
    char* id;
    char* name;
    char* version;
    char* provider;
};


static void *OnSegmenterStart() {
    signal(SIGPIPE, SIG_IGN);
    struct segmenterplugin* segmenter = (struct segmenterplugin*)calloc(1,
            sizeof(struct segmenterplugin));
    segmenter->id = "segmenter";
    segmenter->name = "segmenter";
    segmenter->version = "1.0.0";
    segmenter->provider = "kerry";
    if (!segmenter_logic::Segmenterlogic::GetInstance())
        assert(0);
    return segmenter;
}

static handler_t OnSegmenterShutdown(struct server* srv, void* pd) {
    segmenter_logic::Segmenterlogic::FreeInstance();

    return HANDLER_GO_ON;
}

static handler_t OnSegmenterConnect(struct server *srv, int fd, void *data,
        int len) {
    segmenter_logic::Segmenterlogic::GetInstance()->OnSegmenterConnect(srv, fd);
    return HANDLER_GO_ON;
}

static handler_t OnSegmenterMessage(struct server *srv, int fd, void *data,
        int len) {
    segmenter_logic::Segmenterlogic::GetInstance()->OnSegmenterMessage(srv,
            fd, data, len);
    return HANDLER_GO_ON;
}

static handler_t OnSegmenterClose(struct server *srv, int fd) {
    segmenter_logic::Segmenterlogic::GetInstance()->OnSegmenterClose(srv, fd);
    return HANDLER_GO_ON;
}

static handler_t OnUnknow(struct server *srv, int fd, void *data,
        int len) {
    return HANDLER_GO_ON;
}

static handler_t OnBroadcastConnect(struct server* srv, int fd,
        void *data, int len) {
    segmenter_logic::Segmenterlogic::GetInstance()->OnBroadcastConnect(
            srv, fd, data, len);
    return HANDLER_GO_ON;
}

static handler_t OnBroadcastClose(struct server* srv, int fd) {
    segmenter_logic::Segmenterlogic::GetInstance()->OnBroadcastClose(srv, fd);
    return HANDLER_GO_ON;
}

static handler_t OnBroadcastMessage(struct server* srv, int fd, void *data,
        int len) {
    segmenter_logic::Segmenterlogic::GetInstance()->OnBroadcastMessage(srv,
            fd, data, len);
    return HANDLER_GO_ON;
}

static handler_t OnIniTimer(struct server* srv) {
    segmenter_logic::Segmenterlogic::GetInstance()->OnIniTimer(srv);
    return HANDLER_GO_ON;
}

static handler_t OnTimeOut(struct server* srv, char* id, int opcode, int time) {
    segmenter_logic::Segmenterlogic::GetInstance()->OnTimeout(srv,
            id, opcode, time);
    return HANDLER_GO_ON;
}


int segmenter_plugin_init(struct plugin *pl) {
    pl->init = OnSegmenterStart;
    pl->clean_up = OnSegmenterShutdown;
    pl->connection = OnSegmenterConnect;
    pl->connection_close = OnSegmenterClose;
    pl->connection_close_srv = OnBroadcastClose;
    pl->connection_srv = OnBroadcastConnect;
    pl->handler_init_time = OnIniTimer;
    pl->handler_read = OnSegmenterMessage;
    pl->handler_read_srv = OnBroadcastMessage;
    pl->handler_read_other = OnUnknow;
    pl->time_msg = OnTimeOut;
    pl->data = NULL;
    return 0;
}

