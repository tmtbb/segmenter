#!/usr/bin/python2.6  
# -*- coding: utf-8 -*-  
#encoding=utf-8

'''
Created on 2015年9月29日

@author: kerry
'''

from multiprocessing import Pool
import platform
import sys

from netsvc.network import LRInitialScheduler

def start_process(user, password):
    initial_scheduler = LRInitialScheduler()
    initial_scheduler.connection("222.73.57.17", 16003, user, password)
    initial_scheduler.start_run()

def main():
    pool = Pool(processes=1)
    for i in range(1):
        ret = pool.apply_async(start_process, (account_list[i], '1234567'))
    pool.close()
    pool.join()
    if ret.successful():
        print 'create processes successful'

if __name__ == '__main__':
    sysstr = platform.system()
    print sysstr
    if(platform.system()=="Darwin" or platform.system()=="Linux"):
        reload(sys)
        sys.setdefaultencoding('utf-8')  # @UndefinedVariable
#     main()
    start_process('kerry','1234567') 
    
