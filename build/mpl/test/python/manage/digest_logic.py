#!/usr/bin/python2.6  
# -*- coding: utf-8 -*-  
#encoding=utf-8

'''
Created on 2016年5月17日

@author: Administrator
'''

import netsvc.packet_processing as ppr
from netsvc.packet_processing import ArticleUnit
from netsvc.packet_processing import ArticleEnd
from netsvc.packet_processing import ResultDigest
from base.log import kidlog

class DigestLogic(object):
    '''
    classdocs
    '''
    def __init__(self):
        pass
    
    def on_textrank_digest(self,id,data):
        digset = ArticleUnit()
        digset.make_head(0,0,0,3001,0,0)
        digset.set_id(id)
        digset.set_article(data)
        return digset.packetstream()
    
    def on_digest_end(self,transport,id):
        end = ArticleEnd()
        end.make_head(0,0,0,3002,0,0)
        end.set_id(id)
        return end.packetstream()
        
    
    def on_set_digeset(self,transport,id,data):
        digest_unit = self.on_textrank_digest(id,data)
        #return digest_unit
        transport.write(digest_unit)
        
    def on_result_digest(self,data):
        result_digest = ResultDigest()
        result_digest.unpacketstream(data)
        kidlog.log().debug("id %d  digest %s",result_digest.id,result_digest.digest)
        
    def on_test_digeset(self,t):
        id = 10001
        content_first = "上交所在2012年鼓励上市公司每年的现金分红比例不少于30%，但随着中国经济的收缩和融资成本的上升，很多上市公司的现金流愈趋紧张，敢于大比例分红的企业屈指可数，不过，汽车板块无疑是个例外。截至5月12日，从各大车企发布的年度财务报告来看，虽然2015年我国汽车市场增幅比上年同期有所减缓，但九成上市车企净利润均实现不同程度的涨 幅，排名前三位的车企为上汽集团、长安汽车以及长城汽车，净利润额分别为296.51亿元、99.53亿元和80.4亿元。"
        #content_first = "1111"
        self.on_set_digeset(t,id,content_first)
        
        
        content_two = "据记者统计，22家上市车企中有16家公布了分红、送转股份预案，其中上汽集团、宇通客车和长安汽车最为慷慨，分红额分别为149.95亿元、33.21亿元和29.84亿元。"
        self.on_set_digeset(t,id,content_two)
        
        content_three = "另一方面，“铁公鸡”金杯汽车从1991年到现在，25年不分配、不转增;亚星客车从2003年至今，13年未分红送转。"
        self.on_set_digeset(t,id,content_three)
        
        content_four = "通过Wind数据统计，2015年，22家整车上市公司中，16家公司有转增股份或现金分红预案发布，占比72%。上述16家车企累计分红达258.08亿元，相比其611.12亿元的利润总额来看，汽车行业股利支付率达到42.23%。"
        self.on_set_digeset(t,id,content_four)
        content_five = "此前上汽集团公布的财报显示，2015年公司将向股东派发现金红利，每10股派13.60元(含税)，共计分红金额149.95亿元，公司2015全年净利润共计297.94亿元。这意味着，上汽全年净利润的50.3%都回馈给了股东。"
        self.on_set_digeset(t,id,content_five)

        content_six = "而作为新能源补贴最大受益者，宇通客车2015年财报显示，公司累计完成客车销售67018辆，实现营业收入312.1亿元，获得国家和地方政府的新能源汽车推广应用补贴高达68.6亿元，而这相当于净利润35.35亿元的近两倍。"
        self.on_set_digeset(t,id,content_six)
        content_seven = "年报发布的同时，宇通客车也公布其分红预案，公司拟每10股派发现金红利15元(含税)。按照目前宇通客车股本22.13亿股计算，每10股分红 15元，分红将耗资约33.20亿元，这一数字占到了其净利润的93.92%。按照其目前股价20.33元/股计算，其股息率高达7.4%。"
        self.on_set_digeset(t,id,content_seven)
        content_eight = "值得一提的是，上述16家提出分红送转预案的车企中，每股分红超过0.5元的有5家，占比31.3%，其余每股分红均在0.2元以下。有6家高于0.1元，占比37.5%;5家在0.1元以下，占比31.3%。"
        self.on_set_digeset(t,id,content_eight)
        content_nine = "有汽车行业分析师对记者表示，资金充裕、运营稳健的企业在日常经营、正常投资过程中不会过分依赖利润累积，对于当期收入会给予投资者更多分红回报，公司会因此被认为更具有价值投资潜力，容易吸引投资者长期持有。"
        self.on_set_digeset(t,id,content_nine)
        content_ten = "金杯汽车24年未分红 汽车行业股息率仅为1%"
        self.on_set_digeset(t,id,content_ten)
        
        
        t.write(self.on_digest_end(t,id))
        
        
digest_logic = DigestLogic()