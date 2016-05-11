#!/usr/bin/python2.6
# -*- coding: utf-8 -*-
#encoding=utf-8

'''
Created on 2015年10月6日

@author: kerry
'''
from netsvc.packet_processing import AnalyticalReg

class LoginManager(object):
    '''
    classdocs
    '''

    def __init__(self):
        '''
        Constructor
        '''

    def manager_register(self, level, mac, password):
        '''
        manager reg
        '''
        login = AnalyticalReg()
        login.make_head(0, 0, 0, 1031, 0, 0)
        login.set_level(level)
        login.set_password(password)
        login.set_mac(mac)
        return login.packestream()

login_manager = LoginManager()
