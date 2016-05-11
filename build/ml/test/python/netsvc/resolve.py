#-.- coding:utf-8 -.-
'''
Created on 2015年9月29日

@author: kerry
'''
import struct

class NetData(object):
    '''
    classdocs
    '''

    def __init__(self):
        '''
        Constructor
        '''
        self.struct_format = "=h"
        self.prefix_length = struct.calcsize(self.struct_format)
        self._unprocessed = ""
        self.PACKET_MAX_LENGTH = 99999

    def net_wok(self, data):
        '''
        work
        长度取前2个字节
        '''
        alldata = self._unprocessed + data
        current_offset = 0
        fmt = self.struct_format
        self._unprocessed = alldata
        while len(alldata) >= (current_offset + self.prefix_length):
            message_start = current_offset + self.prefix_length
            length, = struct.unpack(fmt, alldata[current_offset:message_start])
            if length > self.PACKET_MAX_LENGTH:
                self._unprocessed = alldata
                self.lenthLimitExceeded(length)
                return
            message_end = current_offset + length
            if len(alldata) < message_end:
                packet = ""
                result = 0
                break
            packet = alldata[current_offset:message_end]
            current_offset = message_end
            result = 1
        self._unprocessed = alldata[current_offset:]
        return packet, result
