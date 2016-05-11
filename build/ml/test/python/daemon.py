#!/usr/bin/python
#coding: utf-8
'''
Created on 2015年10月16日

@author: chenyitao
'''

import os
import time
import socket
from base.send_sms import SendSms

def check_aliveness(ip, port):
    sk = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sk.settimeout(1)
    try:
        sk.connect((ip,port))
        print 'server %s %d service is OK!' %(ip,port)
        return True
    except Exception:
        print 'server %s %d service is NOT OK!'  %(ip,port)
        return False
    finally:
        sk.close()
    return False

def main():
    '''
    daemon proc start
    '''
    while True:
        proc_info = os.popen('ps ax | grep '+'analysis_client.py'+'|grep -v grep',
                             'r').read()
        if len(proc_info) > 0:
            pid = proc_info.split(' ')[1]
            print pid
            print 'started'
        else:
            print 'restart'
            # 112.124.49.59
            if not check_aliveness('222.73.34.101', 16000):
                print 'server unusual'
                time.sleep(30)
                continue
            os.system('python ./test_client.py')
            sms = SendSms('15157109258,15158114927,18668169052',
                          '【KID解析程序】程序已停止，正在重启。')
#             sms.main()
        time.sleep(30)

if __name__ == '__main__':
    main()
