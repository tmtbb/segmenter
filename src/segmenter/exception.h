#ifndef __QSS_SEGMENTER_EXCEPTION_H__
#define __QSS_SEGMENTER_EXCEPTION_H__

#include <string>

namespace qss {
namespace segmenter {

  class ExceptionBase {
    public:
      ExceptionBase(const std::string msg):msg_(msg) {}
      /* override */ virtual ~ExceptionBase() {}
      std::string getMessage() const { return msg_; }
    protected:
      std::string msg_;
  };

  class ConfigException: public ExceptionBase {
    public:
      ConfigException(const std::string msg):ExceptionBase(msg) {}
  };
  class ConvertEncodingError: public ExceptionBase {
    public:
      ConvertEncodingError(const std::string msg):ExceptionBase(msg) {}
  };
  class RegularExpressionException: public ExceptionBase {
    public:
      RegularExpressionException(const std::string msg):ExceptionBase(msg) {}
  };

  class ChunkingError: public ExceptionBase {
    public:
      ChunkingError(const std::string msg):ExceptionBase(msg) {}
  };
  class WordsegmentError: public ExceptionBase {
    public:
      WordsegmentError(const std::string msg):ExceptionBase(msg) {}
  };
	
	class DictionaryError: public ExceptionBase {
    public:
      DictionaryError(const std::string msg):ExceptionBase(msg) {}
  };
	class ErrorCorrectionDicError: public ExceptionBase {
    public:
      ErrorCorrectionDicError(const std::string msg):ExceptionBase(msg) {}
  };
  class PosTaggerError: public ExceptionBase {
    public:
      PosTaggerError(const std::string msg):ExceptionBase(msg) {}
  };
}
}
#endif
