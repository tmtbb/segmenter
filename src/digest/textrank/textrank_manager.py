#-*- encoding:utf-8 -*-

from __future__ import print_function

import sys
import codecs
import logging
from textrank4zh import TextRank4Keyword, TextRank4Sentence


log_console = logging.StreamHandler(sys.stderr)
default_logger = logging.getLogger(__name__)
default_logger.setLevel(logging.DEBUG)
default_logger.addHandler(log_console)

class TextRank(object):
    '''
    classdocs
    '''
    
    def __init__(self):
        '''
        Constructor
        '''
        self.tr4s = TextRank4Sentence()
    
    def Analyze(self,text):
        self.tr4s.analyze(text=text, lower=True, source = 'all_filters')
    
    def GetSentences(self):
        result = self.tr4s.get_key_sentences(num = 4)
        return result[0]
    
    def Analyzer(self,text):
        self.tr4s.analyze(text=text, lower=True, source = 'all_filters')
        result = result = self.tr4s.get_key_sentences(num = 4)
        return result[0].get('sentence')

textrank = TextRank()

def Analyzer(text):
   default_logger.debug("=====================")
   default_logger.debug("%s" % text)
   default_logger.debug("len [%d]" % len(text))
   default_logger.debug("====================")
   digest = textrank.Analyzer(text)
   default_logger.debug("**********************")
   default_logger.debug("%s" % digest)
   default_logger.debug("**********************")
   return digest
