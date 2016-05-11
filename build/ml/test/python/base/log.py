#-*-coding:utf-8-*-
'''
Created on 2015年9月29日

@author: kerry
'''
import logging
import datetime


class Log(object):
    '''
    classdocs
    '''

    def __init__(self):
        '''
        Constructor
        '''
        filename = 'robots_'+datetime.datetime.now().strftime('%b_%d_%y-%H')+'.log'
        format_str = 'pid :%(process)d tid :%(thread)d  \
        %(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s'
        logging.basicConfig(level=logging.DEBUG,
                            format=format_str,
                            datefmt='%a, %d %b %Y %H:%M:%S',
                            filename=filename,
                            filemode='a')
        ##控制台输出
        console = logging.StreamHandler()
        console.setLevel(logging.INFO)
        console.setLevel(logging.DEBUG)
        formatter = logging.Formatter(format_str)
        console.setFormatter(formatter)
        # 将定义好的console日志handler添加到root logger
        logging.getLogger('').addHandler(console)

    def log(self):
        '''
        return logging
        '''
        return logging

kidlog = Log()
