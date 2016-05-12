# -.- coding:utf-8 -.-
'''
Created on 2015年10月9日

@author: chenyitao
'''
from netsvc.packet_processing import AnalyticalState

class StateManager(object):
    '''
    classdocs
    '''

    def __init__(self):
        '''
        Constructor
        '''
        pass

    def set_package_info(self, analysis_id, state):
        '''
        set package info
        '''
        state_info = AnalyticalState()
        state_info.make_head(0, 0, 0, 1032, 0, 0)
        state_info.set_analytical_id(analysis_id)
        state_info.set_state(state)
        return state_info.packestream()

state_manager = StateManager()
