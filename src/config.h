#ifndef __QSS_SEGMENTER_CONFIG_HEADER__
#define __QSS_SEGMENTER_CONFIG_HEADER__
#include "INIFile.h"

namespace qss {
namespace segmenter {

class Config
{
    public:
        static Config *get_instance()
        {
            if (_instance == NULL) _instance = new Config;

            return _instance;
        };
        bool init(const char* cfgfile)
        {
            if (_ini.init(cfgfile) == 0)
				return true;
            return false;
        };
        const char* read(const char *sect, const char *key)
        {
            return _ini.read(sect, key);
        };
        void read(const char *sect, const char *key, char *buf, size_t len)
        {
            _ini.read(sect, key, buf, len);
        };
        int readint(const char *sect, const char *key, int defvalue)
        {
            return _ini.readint(sect, key, defvalue);
        };

    private:
        static Config  *_instance;
        Config()    {}
        ~Config()   {}
        INIFile _ini;
		// delete _instance while the program exit
		class GarbageClear {
		 public:
		  ~GarbageClear();
		};
		static GarbageClear garbage_clear;

};
}
}
#endif
