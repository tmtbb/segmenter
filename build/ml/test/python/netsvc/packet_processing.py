#!/usr/bin/python2.6
# -*- coding: utf-8 -*-
#encoding=utf-8

'''
Created on 2015年9月29日

@author: kerry
'''
import struct
class PacketHead(object):
    '''
    package head
    '''
    def __init__(self):
        self.packet_length = 0
        self.is_zip_encrypt = 0
        self.head_type = 0
        self.signature = 0
        self.operate_code = 0
        self.data_length = 0
        self.timestamp = 0
        self.session_id = 0
        self.reserved = 0

    def set_packet_length(self, packet_length):
        '''
        set packet length
        '''
        self.packet_length = packet_length

    def set_is_zip_encrypt(self, is_zip_encrypt):
        '''
        set is zip
        '''
        self.is_zip_encrypt = is_zip_encrypt

    def set_type(self, head_type):
        '''
        set type
        '''
        self.head_type = head_type

    def set_signature(self, set_signature):
        '''
        set signature
        '''
        self.signature = set_signature

    def set_operate_code(self, operate_code):
        '''
        set operate code
        '''
        self.operate_code = operate_code

    def set_data_length(self, data_length):
        '''
        set data length
        '''
        self.data_length = data_length

    def set_timestamp(self, timestamp):
        '''
        set rimestamp
        '''
        self.timestamp = timestamp

    def set_session_id(self, session_id):
        '''
        set session id
        '''
        self.session_id = session_id

    def set_reserved(self, reserved):
        '''
        set reserced
        '''
        self.reserved = reserved

    def make_head(self,
                  is_zip_encrypt,
                  head_type,
                  signature,
                  operate_code,
                  session_id,
                  reserved):
        '''
        make head
        '''
        self.is_zip_encrypt = is_zip_encrypt
        self.head_type = head_type
        self.signature = signature
        self.operate_code = operate_code
        self.timestamp = 0
        self.session_id = session_id
        self.reserved = reserved

    def headstream(self):
        '''
        head stream
        '''
        self.head = struct.pack('=HBBHHHIQI',
                                self.packet_length,
                                self.is_zip_encrypt,
                                self.head_type,
                                self.signature,
                                self.operate_code,
                                self.data_length,
                                self.timestamp,
                                self.session_id,
                                int(self.reserved))

    def packet_head_length(self):
        '''
        return packet head length
        '''
        return 26

    def unpackhead(self, packet_stream):
        '''
        unpackhead
        '''
        self.packet_length,\
        self.is_zip_encrypt,\
        self.head_type,\
        self.signature,\
        self.operate_code,\
        self.data_length,\
        self.timestamp,\
        self.session_id,\
        self.reserved = struct.unpack_from('=hbbhhhiqi', packet_stream)


class SegmenterWord(PacketHead):
    '''
    segmenter word
    '''
    def __init__(self):
        PacketHead.__init__(self)
        self.uid = 0
        self.token = ""
        self.content = ""
        
    def set_uid(self,uid):
        self.uid = uid
        
    def set_token(self,token):
        self.token = token
        
    def set_content(self,content):
        self.content = content
        
    def packet_body_length(self):
        '''
        return packet len
        '''
        
        return 40+len(str(self.content))
        
    def bodystream(self):
        '''
        body stream
        '''
        str_formart = '=q32s%ds' %(len(str(self.content)))
        self.body = struct.pack(str_formart,
                                self.uid,
                                self.token,
                                self.content)
        
    def packestream(self):
        '''
        pakcet stream
        '''
        self.bodystream()
        self.set_packet_length(self.packet_head_length() + self.packet_body_length())
        self.set_data_length(self.packet_body_length())
        self.headstream()
        return self.head + self.body
    
class AnalyticalReg(PacketHead):
    '''
    analycal reg
    '''
    def __init__(self):
        PacketHead.__init__(self)
        self.level = 0
        self.password = ""
        self.mac = ""

    def set_level(self, level):
        '''
        set level
        '''
        self.level = level

    def set_password(self, password):
        '''
        set password
        '''
        self.password = password

    def set_mac(self, mac):
        '''
        set mac
        '''
        self.mac = mac

    def bodystream(self):
        '''
        body stream
        '''
        self.body = struct.pack('=h8s16s',
                                self.level,
                                self.password,
                                self.mac)

    def packet_body_length(self):
        '''
        packet body length
        '''
        return 2 + 8 + 16

    def packestream(self):
        '''
        pakcet stream
        '''
        self.bodystream()
        self.set_packet_length(self.packet_head_length() + self.packet_body_length())
        self.set_data_length(self.packet_body_length())
        self.headstream()
        return self.head + self.body

class ElementAnaly(object):
    '''
    element analy
    '''
    def __init__(self):
        self.analysis_id = 0
        self.task_id = 0
        self.attr_id = 0
        self.type = 0 # 1:habse 3:ftp
        self.depth = 0
        self.cur_depth = 0
        self.name = ""
        self.key = ""

    def set_depth(self, depth):
        self.depth = depth

    def set_cur_depth(self, cur_depth):
        self.cur_depth = cur_depth

    def set_name(self, name):
        '''
        set name
        '''
        self.name = name

    def set_key(self, key):
        '''
        set key
        '''
        self.key = key

    def set_analysis_id(self, analysis_id):
        '''
        set analysis id
        '''
        self.analysis_id = analysis_id

    def set_task_id(self, task_id):
        '''
        set task id
        '''
        self.task_id = task_id

    def set_attr_id(self, attr_id):
        '''
        set attr id
        '''
        self.attr_id = attr_id

    def set_type(self, _type):
        self.type = _type

    def get_depth(self):
        return self.depth

    def get_cur_depth(self):
        return self.cur_depth

    def get_name(self):
        '''
        get name
        '''
        return self.name

    def get_key(self):
        '''
        get key
        '''
        return self.key

    def get_analysis_id(self):
        '''
        get analysis id
        '''
        return self.analysis_id

    def get_task_id(self):
        '''
        get task id
        '''
        return self.task_id

    def get_attr_id(self):
        '''
        get attr id
        '''
        return self.attr_id

    def get_type(self):
        return self.type

    @classmethod
    def packet_len(cls):
        '''
        return packet len
        '''
        return struct.calcsize('=qqiBBB32s32s')

class AnalyticalInfo(PacketHead):
    '''
    analytical info
    '''
    def __init__(self):
        PacketHead.__init__(self)
        self.id = 0
        self.analytical = []

    def getanalylist(self):
        '''
        get analy list
        '''
        return self.analytical

    def unpackstream(self, data):
        '''
        unpack stream
        '''
        self.unpackhead(data)
        i = 0
        n = (self.data_length) / ElementAnaly.packet_len()
        while n > 0:
            element = ElementAnaly()
            analysis_id,\
            task_id,\
            attr_id,\
            storage_type,\
            depth,\
            cur_depth,\
            name,\
            key = struct.unpack_from('=qqiBBB32s32s',
                                     data,
                                     26 + i * ElementAnaly.packet_len())
            n = n -1
            i = i + 1
            element.set_analysis_id(analysis_id)
            element.set_task_id(task_id)
            element.set_attr_id(attr_id)
            element.set_type(storage_type)
            element.set_name(name.rstrip('\x00'))
            element.set_key(key.rstrip('\x00'))
            element.set_depth(depth)
            element.set_cur_depth(cur_depth)
            self.analytical.append(element)

class AnalyticalState(PacketHead):
    '''
    analytical state
    '''
    def __init__(self):
        PacketHead.__init__(self)
        self.analytical_id = 0
        self.state = 0

    def set_analytical_id(self, analytical_id):
        '''
        set analytical id
        '''
        self.analytical_id = analytical_id

    def set_state(self, state):
        '''
        set state
        '''
        self.state = state

    def get_analytical_id(self):
        '''
        get analytical id
        '''
        return self.analytical_id

    def get_state(self):
        '''
        get state
        '''
        return self.state

    def bodystream(self):
        '''
        body stream
        '''
        self.body = struct.pack('=qB', self.analytical_id, self.state)

    def packet_body_length(self):
        '''
        packet body length
        '''
        return 8 + 1

    def packestream(self):
        '''
        pack stream
        '''
        self.bodystream()
        self.set_packet_length(self.packet_head_length() + self.packet_body_length())
        self.set_data_length(self.packet_body_length())
        self.headstream()
        return self.head + self.body


class AnalyzedURLInfo(object):
    '''
    analytical state
    '''
    length = struct.calcsize('QQBBB256s')
    
    def __init__(self):
        self.task_id = 0
        self.attr_id = 0
        self.depth = 0
        self.cur_depth = 0
        self.method = 0
        self.url = ''

    def bodystream(self):
        '''
        body stream
        '''
        self.body = struct.pack('=QQBBB256s',
                                self.task_id,
                                self.attr_id,
                                self.depth,
                                self.cur_depth,
                                self.method,
                                str(self.url))

    def packet_body_length(self):
        '''
        packet body length
        '''
        return struct.calcsize('QQBBB256s')

    def packestream(self):
        '''
        pack stream
        '''
        self.bodystream()
        return self.body

class AnalyzedURLs(PacketHead):

    def __init__(self):
        PacketHead.__init__(self)
        self.make_head(0, 0, 0, 1033, 0, 0)
        self.manage_id = 0
        self.token = ''
        self.url_infos = []

    def packestream(self):
        '''
        unpack stream
        '''
        infos_data = ''
        for info in self.url_infos:
            infos_data += info.packestream()
        self.body = struct.pack('=I32s%ds' % len(infos_data),
                                self.manage_id,
                                self.token,
                                infos_data)
        body_len = len(self.body)
        self.set_packet_length(self.packet_head_length() + body_len)
        self.set_data_length(body_len)
        self.headstream()
        return self.head + self.body
