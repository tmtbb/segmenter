#!/usr/bin/python2.6
# -*- coding: utf-8 -*-
#encoding=utf-8
from _ast import Pass

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



class ResultDigest(PacketHead):
    '''
    reuslt digest
    '''
    def __init__(self):
        PacketHead.__init__(self)
        self.id = 0
        self.digest = ""
        
    def unpacketstream(self,data):
        self.data_length = struct.unpack_from('=h',data,8)
        str_format = '=i%ds' % self.data_length
        self.id,self.digest = struct.unpack_from(str_format,data,16)
        print self.id
        
        
        
class ArticleEnd(PacketHead):
    '''
    article unit
    '''
    def __init__(self):
        PacketHead.__init__(self)
    
    def set_id(self,id):
        self.id = id
        
    def packet_body_length(self):
        return 4
    
    def bodystream(self):
        str_format = '=i'
        self.body = struct.pack(str_format,self.id)
    
    def packetstream(self):
        '''
        pakcet stream
        '''
        self.bodystream()
        self.set_packet_length(self.packet_head_length() + self.packet_body_length())
        self.set_data_length(self.packet_body_length())
        self.headstream()
        return self.head + self.body
            
        
class ArticleUnit(PacketHead):
    '''
    article unit
    '''
    def __init__(self):
        PacketHead.__init__(self)
        self.id = 0
        self.article = ""
    
    def set_id(self,id):
        self.id = id
        
    def set_article(self,article):
        self.article = article
        
    def packet_body_length(self):
        return 4 + len(str(self.article))
    
    def bodystream(self):
        str_format = '=i%ds' % (len(str(self.article)))
        self.body = struct.pack(str_format,self.id,self.article)
    
    def packetstream(self):
        '''
        pakcet stream
        '''
        self.bodystream()
        self.set_packet_length(self.packet_head_length() + self.packet_body_length())
        self.set_data_length(self.packet_body_length())
        self.headstream()
        return self.head + self.body
        
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
        
        return 40+len(str(self.content))
        
    def bodystream(self):
        str_formart = '=q32s%ds' %(len(str(self.content)))
        self.body = struct.pack(str_formart,
                                self.uid,
                                self.token,
                                self.content)
        
    def packetstream(self):
        '''
        pakcet stream
        '''
        self.bodystream()
        self.set_packet_length(self.packet_head_length() + self.packet_body_length())
        self.set_data_length(self.packet_body_length())
        self.headstream()
        return self.head + self.body
    