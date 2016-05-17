//  Copyright (c) 2016-2016 The segmenter Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry
#include "ptl_packet.h"
#include <list>
#include <string>
#include "basic/unzip.h"
#include "basic/zip.h"
#include "protocol/data_packet.h"
#include "logic/logic_comm.h"
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

      case ARTICLE_DIGEST_UNIT : {
    	 struct ArticleDigestUnit* vArticleDigestUnit =
    			 (struct ArticleDigestUnit*)packet_head;
    	 BUILDHEAD(ARTICLEDIGESTUNIT_SIZE);
    	 out.Write64(vArticleDigestUnit->article_identifies);
    	 out.WriteData(vArticleDigestUnit->article_unit.c_str(),
    			 vArticleDigestUnit->article_unit.length());
    	 body_stream = const_cast<char*>(out.GetData());
   	     break;
      }

      case ARTICLE_RESULT_DIGEST : {
    	 struct ArticleResultDigest* vArticleResultDigest =
    			 (struct ArticleResultDigest*)packet_head;
    	 BUILDHEAD (ARICLERESULTDIGEST_SIZE);
    	 out.Write64(vArticleResultDigest->article_identifies);
    	 out.WriteData(vArticleResultDigest->digest.c_str(),
    			 vArticleResultDigest->digest.length());
    	 body_stream = const_cast<char*>(out.GetData());
    	 break;
      }
      case ARTICLE_DIGEST_END : {
    	  struct ArticleDigestEnd* vArticleDigestEnd =
    			  (struct ArticleDigestEnd*)packet_head;
    	  BUILDHEAD(ARTICLEDIGESTEND_SIZE);
    	  out.Write64(vArticleDigestEnd->article_identifies);
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

       case HEART_PACKET:
       case WORD_RESULT_END :{
         BUILDHEAD(0);
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
    	  vSegmenterWord->content.assign(str,str_len);
    	  if (str) {
    		  delete [] str;
    		  str = NULL;
    	  }
    	  break;
      }

      case ARTICLE_DIGEST_UNIT : {
    	  struct ArticleDigestUnit* vArticleDigestUnit =
    			  new struct ArticleDigestUnit;
    	  *packet_head = (struct PacketHead*)vArticleDigestUnit;
    	  FILLHEAD();
    	  vArticleDigestUnit->article_identifies = in.Read32();
    	  int32 str_len = vArticleDigestUnit->data_length - sizeof(int32);
    	  char* str = new char[str_len];
    	  memcpy(str, in.ReadData(str_len,temp),str_len);
    	  vArticleDigestUnit->article_unit.assign(str,str_len);
    	  if (str) {
    		  delete [] str;
    		  str = NULL;
    	  }
    	  break;
      }

      case ARTICLE_RESULT_DIGEST: {
    	  struct ArticleResultDigest* vArticleResultDigest =
    			  new struct ArticleResultDigest;
    	  *packet_head = (struct PacketHead*)vArticleResultDigest;
    	  FILLHEAD();
    	  vArticleResultDigest->article_identifies = in.Read32();
    	  int32 str_len = vArticleResultDigest->data_length - sizeof(int32);
    	  char* str = new char[str_len];
    	  memcpy(str, in.ReadData(str_len,temp),str_len);
    	  vArticleResultDigest->digest.assign(str,str_len);
    	  if (str) {
    		  delete [] str;
    		  str = NULL;
    	  }
    	  break;
      }

      case ARTICLE_DIGEST_END : {
    	  struct ArticleDigestEnd* vAricleDigestEnd =
    			  new struct ArticleDigestEnd;
    	  *packet_head = (struct PacketHead*)vAricleDigestEnd;
    	  FILLHEAD();
    	  vAricleDigestEnd->article_identifies = in.Read32();
    	  break;
      }

      case WORD_RESULT : {
    	  struct WordResult* vWordResult =
    			  new struct WordResult;
    	  *packet_head = (struct PacketHead*)vWordResult;

    	  FILLHEAD();
    	  int32 num = data_length / (WORDUNIT_SIZE);
    	  int32 len = 0;
    	  int32 i = 0;
    	  for (; i < num; i++) {
    		  struct WordUnit* unit = new struct WordUnit;
    		  int32 temp = 0;
              memcpy(unit->word, in.ReadData(WORD_SIZE - 1, temp),
            		  WORD_SIZE - 1);
              int32 word_len = (temp - 1) < (WORD_SIZE - 1) ?
                                (temp - 1) : (WORD_SIZE - 1);
              unit->word[word_len] = '\0';
              unit->utype = in.Read32();
              vWordResult->list.push_back(unit);
    	  }
    	  break;
      }

      case HEART_PACKET :
      case WORD_RESULT_END :{
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
#if 0
	int16 packet_length = packet_head->packet_length;
	int16 is_zip_encrypt = packet_head->is_zip_encrypt;
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
      case HEART_PACKET:
      case WORD_RESULT_END: {
      	PRINT_TITLE("struct HeartPacket Dump Begin");
      	DUMPHEAD();
      	PRINT_END("struct HeartPacket Dump End");
    	break;
      }
      case USER_REGISTER:
      case USER_LOGIN : {
    	struct UserRegister* vUserRegister =
    			(struct UserRegister*)packet_head;
    	PRINT_TITLE("struct UserRegister Dump Begin");
    	DUMPHEAD();
    	PRINT_STRING(vUserRegister->username);
    	PRINT_STRING(vUserRegister->password);
    	PRINT_END("struct UserRegister Dump End");
    	break;
      }
      case WORD_SEGMENTER: {
    	struct SegmenterWord* vSegmenterWord =
    			(struct SegmenterWord*)packet_head;
    	PRINT_TITLE("struct SegmenterWord Dump Begin");
    	DUMPHEAD();
    	PRINT_INT64(vSegmenterWord->uid);
    	PRINT_STRING(vSegmenterWord->token);
    	PRINT_STRING(vSegmenterWord->content.c_str());
    	PRINT_END("struct SegmenterWord Dump End");
    	break;
      }

      case ARTICLE_DIGEST_UNIT : {
    	struct ArticleDigestUnit* vArticleDigestUnit =
    			(struct ArticleDigestUnit*)packet_head;
    	PRINT_TITLE("struct ArticleDigestUnit Dump Begin");
    	DUMPHEAD ();
    	PRINT_INT32(vArticleDigestUnit->article_identifies);
    	PRINT_STRING(vArticleDigestUnit->article_unit.c_str());
    	PRINT_END("struct ArticleDigestUnit Dump End")
    	break;
      }

      case ARTICLE_RESULT_DIGEST : {
    	  struct ArticleResultDigest* vArticleResultDigest =
    			  (struct ArticleResultDigest*)packet_head;
    	  PRINT_TITLE
      }

      case ARTICLE_DIGEST_END : {
    	struct ArticleDigestEnd* vArticleDigestEnd =
    			(struct ArticleDigestEnd*)packet_head;
    	PRINT_TITLE("struct ArticleDigestEnd Dump Begin")
    	PRINT_INT32(vArticleDigestEnd->article_identifies);
    	PRINT_END("struct ArticleDigestEnd Dump End")
    	break;
      }

      case WORD_RESULT: {
    	struct WordResult* vWordResult =
    			(struct WordResult*)packet_head;
    	PRINT_TITLE("struct AssignmentMultiTask Dump Begin");
    	DUMPHEAD();
    	std::list<struct WordUnit*>::iterator it =
    			vWordResult->list.begin();
    	for (; it != vWordResult->list.end(); it++) {
    		PRINT_STRING((*it)->word);
    		PRINT_INT((*it)->utype);
    	}

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
#if 1
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
    LOG_DEBUG2("===start====\nopcode[%d]:\n%s\n====end====\n",
    		head->operate_code, sret.c_str());
#endif
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
