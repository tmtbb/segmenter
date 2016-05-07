//  Copyright (c) 2016-2016 The segmenter Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry
#include "ptl_packet.h"
#include <list>
#include <string>
#include "basic/unzip.h"
#include "basic/zip.h"
#include "protocol/data_packet.h"
#include "ptl_comm_head.h"

#define DUMPPACKBUF     4096 * 10

namespace ptl {
bool PacketProsess::PacketStream(const PacketHead* packet_head,
        void ** packet_stream, int32* packet_stream_length) {
	bool r = true;
    int16 packet_length = packet_head->packet_length;
    int8 is_zip_encrypt = packet_head->is_zip_encrypt;
    int8 type = packet_head->type;
    int16 signature = packet_head->signature;
    int16 operate_code = packet_head->operate_code;
    int16 data_length = packet_head->data_length;
    int32 timestamp = packet_head->timestamp;
    int64 session_id = packet_head->session_id;
    int32 reserved = packet_head->reserved;


    char* body_stream = NULL;
    char* data = NULL;

    switch (operate_code) {
      case HEART_PACKET:
    	break;
      case USER_REGISTER: {
    	struct UserRegister* vUserRegister =
    			(struct UserRegister*)packet_head;
    	BUILDHEAD(USERREGISTER_SIZE);
    	out.WriteData(vUserRegister->username, USERNAME_SIZE - 1);
    	out.WriteData(vUserRegister->password, PASSWORD_SIZE - 1);
    	body_stream = const_cast<char*>(out.GetData());
    	break;
      }

      case USER_LOGIN: {
    	struct UserLogin* vUserLogin =
    			(struct UserLogin*)packet_head;
    	BUILDHEAD(USERLOGIN_SIZE);
    	out.WriteData(vUserLogin->username, USERNAME_SIZE - 1);
    	out.WriteData(vUserLogin->password, PASSWORD_SIZE - 1);
    	body_stream = const_cast<char*>(out.GetData());
        break;
      }

      case WORD_SEGMENTER: {
    	struct SegmenterWord* vSegmenterWord =
    			(struct SegmenterWord*)packet_head;
    	BUILDHEAD(WORDSEGMENTER_SIZE);
    	out.Write64(vSegmenterWord->uid);
    	out.WriteData(vSegmenterWord->token, TOKEN_SIZE - 1);
    	out.WriteData(vSegmenterWord->content.c_str(),
    			vSegmenterWord->content.length());
        body_stream = const_cast<char*>(out.GetData());
        break;
      }
            
       case WORD_RESULT: {
         struct WordResult* vWordResult =
        		 (struct WordResult*)packet_head;
         int32 len = 0;
         data_length = vWordResult->list.size() * WORDUNIT_SIZE
        		 + sizeof(double);

         BUILDHEAD(data_length);
         std::list<struct WordUnit*>::iterator it =
        		 vWordResult->list.begin();

         for ( ; it != vWordResult->list.end(); it++) {
        	 out.WriteData((*it)->word, WORD_SIZE - 1);
        	 out.Write32((*it)->utype);
         }
         body_stream = const_cast<char*>(out.GetData());
         break;
       }

      default:
    	r = false;
    	break;
    }
    if (r) {
        char* packet = NULL;
        int32 packet_body_len = 0;
        if (is_zip_encrypt == ZIP_AND_NOENCRYPT) {
        	uint8* zip_data = NULL;
        	uint64 zip_len = 0;
        	const uint8* unzip =
        			const_cast<const uint8*>(reinterpret_cast<uint8*>(body_stream));
            zip_len = CompressionStream(unzip,
            		packet_length - sizeof(int16) - sizeof(int8),
            		&zip_data);
            free(body_stream);
            body_stream = NULL;
            packet  = reinterpret_cast<char*>(zip_data);
            packet_body_len = zip_len;
        } else {
        	packet = body_stream;
        	packet_body_len = packet_length - sizeof(int16) - sizeof(int8);
        }

        packet::DataOutPacket out_packet(false,
        		packet_body_len + sizeof(int16) + sizeof(int8));
        out_packet.Write16(packet_body_len + sizeof(int16) + sizeof(int8));
        out_packet.Write8(is_zip_encrypt);
        out_packet.WriteData(packet, packet_body_len);
        //free(packet);
        delete packet;
        packet = NULL;
        *packet_stream =  reinterpret_cast<void*>(
        		const_cast<char*>(out_packet.GetData()));
        *packet_stream_length =  packet_body_len + sizeof(int16) + sizeof(int8);
    }
    return r;
}

bool PacketProsess::UnpackStream(const void* packet_stream, int32 len,
            struct PacketHead** packet_head) {
	bool r = true;
	packet::DataInPacket in_packet(reinterpret_cast<const char*>(packet_stream),
			len);
	int16 packet_length = in_packet.Read16();
	int8 is_zip_encrypt = in_packet.Read8();
	char* body_stream = NULL;
	int32 body_length = 0;
	int32 temp = 0;

    // 是否解压解码
    if (is_zip_encrypt == ZIP_AND_NOENCRYPT) {
        char* temp_body_stream =
                reinterpret_cast<char*>(const_cast<void*>(packet_stream))
                + sizeof(int16) + sizeof(int8);
        const uint8* zipdata = reinterpret_cast<uint8*>(temp_body_stream);
        uint8* unzipdata = NULL;
        int32 temp_body_len = len - sizeof(int16) - sizeof(int8);
        body_length = DecompressionStream(zipdata,
                temp_body_len,
                &unzipdata);
        body_stream = reinterpret_cast<char*>(unzipdata);
    } else {
        body_stream =
                reinterpret_cast<char*>(const_cast<void*>(packet_stream)) +
                sizeof(int16) + sizeof(int8);
        body_length = len - sizeof(int16) - sizeof(int8);
    }

    BUILDPACKHEAD();
    switch (operate_code) {
      case USER_REGISTER : {
    	  struct UserRegister* vUserRegister = new struct UserRegister;
    	  *packet_head = (struct PacketHead*)vUserRegister;

    	  FILLHEAD();
    	  int temp = 0;
    	  memcpy(vUserRegister->username, in.ReadData(USERNAME_SIZE - 1, temp),
    			  USERNAME_SIZE - 1);
    	  int32 username_len = (temp - 1) < (USERNAME_SIZE - 1) ?
    			  (temp - 1) : (USERNAME_SIZE - 1);
    	  vUserRegister->username[username_len] = '\0';

    	  temp = 0;
    	  memcpy(vUserRegister->password, in.ReadData(PASSWORD_SIZE - 1, temp),
    			  PASSWORD_SIZE - 1);

    	  int32 password_len = (temp - 1) < (PASSWORD_SIZE - 1)?
    			  (temp - 1) : (PASSWORD_SIZE - 1);
    	  vUserRegister->password[password_len] = '\0';
    	  break;
      }

      case USER_LOGIN : {
    	  struct UserLogin* vUserLogin = new struct UserLogin;
    	  *packet_head = (struct PacketHead*)vUserLogin;

    	  FILLHEAD();
    	  int temp = 0;
    	  memcpy(vUserLogin->username, in.ReadData(USERNAME_SIZE - 1, temp),
    			  USERNAME_SIZE - 1);
    	  int32 username_len = (temp - 1) < (USERNAME_SIZE - 1) ?
    			  (temp - 1) : (USERNAME_SIZE - 1);
    	  vUserLogin->username[username_len] = '\0';

    	  temp = 0;
    	  memcpy(vUserLogin->password, in.ReadData(PASSWORD_SIZE - 1, temp),
    			  PASSWORD_SIZE - 1);

    	  int32 password_len = (temp - 1) < (PASSWORD_SIZE - 1)?
    			  (temp - 1) : (PASSWORD_SIZE - 1);
    	  vUserLogin->password[password_len] = '\0';
    	  break;
      }

      case WORD_SEGMENTER : {
    	  struct SegmenterWord* vSegmenterWord = new struct SegmenterWord;
    	  *packet_head = (struct PacketHead*)vSegmenterWord;

    	  FILLHEAD();

    	  vSegmenterWord->uid = in.Read64();
    	  int temp = 0;
    	  memcpy(vSegmenterWord->token, in.ReadData(TOKEN_SIZE - 1, temp),
    			  TOKEN_SIZE - 1);

    	  int32 token_len = (temp - 1) < (TOKEN_SIZE - 1)?
    			  (temp - 1) : (TOKEN_SIZE - 1);
    	  vSegmenterWord->token[token_len] = '\0';

    	  int32 str_len = vSegmenterWord->data_length - sizeof(int64)
    			  - (TOKEN_SIZE - 1);
    	  char* str = new char[str_len];
    	  memcpy(str,in.ReadData(str_len,temp),str_len);
    	  break;
      }

      case HEART_PACKET : {
    	  struct PacketHead* vHead =
                   new struct PacketHead;
    	  *packet_head = (struct PacketHead*)vHead;
    	  FILLHEAD();
    	  break;
      }
      default:
    	  r = false;
    	  break;
    }
    return r;
}


void PacketProsess::DumpPacket(const struct PacketHead* packet_head) {
#if 1
    int16 packet_length = packet_head->packet_length;
    int8 is_zip_encrypt = packet_head->is_zip_encrypt;
    int8 type = packet_head->type;
    int16 operate_code = packet_head->operate_code;
    int16 data_length = packet_head->data_length;
    int32 timestamp = packet_head->timestamp;
    int64 session_id = packet_head->session_id;
    int32 reserved = packet_head->reserved;
    int16 signature = packet_head->signature;

    char buf[DUMPPACKBUF];
    bool r = false;
    int32 j = 0;

    switch (operate_code) {
      case WORD_SEGMENTER : {
        struct SegmenterWord* vSegmenterWord =
                (struct SegmenterWord*)packet_head;
        PRINT_TITLE("struct SegmenterWord Dump Begin");
        DUMPHEAD();
        PRINT_INT(vSegmenterWord->uid);
        PRINT_STRING(vSegmenterWord->token);
        PRINT_STRING(vSegmenterWord->content.c_str());
        PRINT_END("struct SegmenterWord Dump End");
        break;
      }

      default:
        break;
    }
    if (buf[0] != '\0')
            LOG_DEBUG2("%s\n", buf);
#endif
}

void PacketProsess::HexEncode(const void *bytes, size_t size) {
/*//  #if defined HEXDUMP
    struct PacketHead* head = (struct PacketHead*)bytes;
    static const char kHexChars[] = "0123456789ABCDEF";
    std::string sret(size*3, '\0');
    for (size_t i = 0; i < size; ++i) {
        char b = reinterpret_cast<const char*>(bytes)[i];
        sret[(i*3)] = kHexChars[(b>>4) & 0xf];
        sret[(i*3)+1] = kHexChars[b&0xf];
        if ((((i*3)+2+1)%12) != 0)
            sret[(i * 3) + 2] = '\40';
        else
            sret[(i * 3) + 2] = '\n';
    }
    //LOG_DEBUG2("===start====\nopcode[%d]:\n%s\n====end====\n",
    //            head->operate_code, sret.c_str());
//  #endif
 * */
}

void PacketProsess::DeletePacket(const void* packet_stream, int32 len,
            struct PacketHead* packet_head) {
	switch (packet_head->operate_code) {
	  case WORD_SEGMENTER: {
		struct SegmenterWord* vSegmenterWord =
				(struct SegmenterWord*)packet_head;
		delete vSegmenterWord;
		break;
	  }
	default:
		break;
	}
}
void PacketProsess::ClearWordList(struct PacketHead* packet_head) {
    struct WordResult* word_result =
            (struct WordResult*)packet_head;
    while (word_result->list.size() > 0) {
        struct WordUnit* unit = word_result->list.front();
        word_result->list.pop_front();
        if (unit) {delete unit; unit = NULL;}
    }
}


}
