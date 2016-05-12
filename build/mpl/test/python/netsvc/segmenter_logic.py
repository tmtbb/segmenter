#!/usr/bin/python2.6  
# -*- coding: utf-8 -*-  
#encoding=utf-8

'''
Created on 2016年5月11日

@author: Administrator
'''

import packet_processing as ppr 

class SegmenterLogic(object):
    '''
    classdocs
    '''
    def __init__(self):
        pass
    
    def UnpackHead(self,data):
        packet_head = ppr.PacketHead()
        packet_head.unpackhead(data)
        return packet_head
    
segmenter_logic = SegmenterLogic()