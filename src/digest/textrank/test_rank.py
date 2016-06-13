#-*- encoding:utf-8 -*-
from __future__ import print_function

import sys
import codecs
import textrank_manager
from textrank4zh import TextRank4Keyword, TextRank4Sentence

try:
    reload(sys)
    sys.setdefaultencoding('utf-8')
except:
    pass

if __name__ == '__main__':
    
    
    #text = codecs.open('./01.txt', 'r', 'utf-8').read()
    #print(textrank_manager.Analyzer(text))
    
    #text = codecs.open('./02.txt', 'r', 'utf-8').read()
    #print(textrank_manager.Analyzer(text))
    
    '''

    rank = textrank_manager.TextRank()
    text = codecs.open('./01.txt', 'r', 'utf-8').read()
    rank.Analyze(text)
    print(rank.GetSentences().get('sentence'))
    
    print('-------------------------------------------------------------------')
    text = codecs.open('./02.txt', 'r', 'utf-8').read()
    #rank.Analyze(text)
    #print(rank.GetSentences().get('sentence'))
    print(rank.Analyzer(text))
    '''
    
    text = codecs.open('./03.txt', 'r', 'utf-8').read()
    tr4s = TextRank4Sentence()
    print()
    print(len(text))
    tr4s.analyze(text=text, lower=True, source = 'all_filters')
    print('digest')
    for item in tr4s.get_key_sentences(num = 4):
        print(item.weight, item.sentence)
    
    
    


