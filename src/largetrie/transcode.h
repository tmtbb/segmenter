#ifndef __TRANS_CODE_H_
#define __TRANS_CODE_H_

#include "baseTpe.h"

enum qsrch_code_t {
	qsrch_code_gbk = 0,
	qsrch_code_utf8,
	qsrch_code_gb2312,
	qsrch_code_iso88591,
	qsrch_code_ucs2,
	qsrch_code_max,
};

int convToucs2(const char *__psrc, int __nsrc, uint16_t *__pdst, int __ndst, qsrch_code_t __code = qsrch_code_gbk );
int convFromucs2(const uint16_t *__psrc, int __nsrc, char *__pdst, int __ndst, qsrch_code_t __code = qsrch_code_gbk );
qsrch_code_t codeFind( const char *__scode );


#endif
