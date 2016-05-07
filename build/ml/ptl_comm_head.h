//  Copyright (c) 2015-2015 The KID Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry


#ifndef SEGMENTER_ML_PTL_COMM_HEAD_H_
#define SEGMENTER_ML_PTL_COMM_HEAD_H_

#include <list>
#include <string>

#include "basic/basictypes.h"

#define USERNAME_SIZE   32 + 1
#define PASSWORD_SIZE   16 + 1
#define TOKEN_SIZE      32 + 1
#define WORD_SIZE       32 + 1
enum  PRS {
    NOZIP_AND_NOENCRYPT = 0,
    ZIP_AND_NOENCRYPT = 1,
    NOZIP_AND_ENCRYPT = 2,
    ZIP_AND_ENCRYPT = 3
};

enum operatorcode {
    HEART_PACKET = 0x64,

    USER_REGISTER = 0x3E9,
    USER_LOGIN =  0x3EA,

    WORD_SEGMENTER = 0x7D1,
    WORD_RESULT = 0x7D2

};

//  packet_length 长度为原始数据长度
struct PacketHead{
    int16 packet_length;
    int8  is_zip_encrypt;
    int8  type;
    int16 signature;
    int16 operate_code;
    int16 data_length;
    int32 timestamp;
    int64 session_id;
    int32 reserved;
};


#define PACKET_HEAD_LENGTH (sizeof(int16) * 4 + sizeof(int8) * 2 + \
    sizeof(int32) * 2 + sizeof(int64))

//USER_REGISTER
#define USERREGISTER_SIZE (USERNAME_SIZE - 1 + PASSWORD_SIZE - 1)
struct UserRegister : public PacketHead {
	char username[USERNAME_SIZE];
	char password[PASSWORD_SIZE];
};

//USER_LOGIN
#define USERLOGIN_SIZE (USERNAME_SIZE - 1 + PASSWORD_SIZE - 1)
struct UserLogin : public PacketHead {
	char username[USERNAME_SIZE];
	char password[PASSWORD_SIZE];
};

//WORD_SEGMENTER
#define WORDSEGMENTER_SIZE (sizeof(int64) + TOKEN_SIZE - 1 + vSegmenterWord->content.length())
struct SegmenterWord : public PacketHead {
	int64 uid;
	char token[TOKEN_SIZE];
	std::string content;
};

#define WORDUNIT_SIZE (sizeof(int32) + (WORD_SIZE - 1))
struct WordUnit {
    char     word[WORD_SIZE];
    int32    utype;
    //double   idf;
};

//WORD_RESULT
struct WordResult : public PacketHead {
    std::list <struct WordUnit*>   list;
    //double                         idf;
};







#endif
