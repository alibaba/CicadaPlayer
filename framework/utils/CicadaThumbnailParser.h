
#ifndef CicadaThumbnailParser_H__
#define CicadaThumbnailParser_H__

#include <string>
#include <list>

#include "CicadaType.h"

using namespace std;

class CICADA_CPLUS_EXTERN ThumbnailInfo
{
public:
	int64_t timestamp = 0;
	unsigned int duration = 0;
	unsigned int posX = 0;
	unsigned int posY = 0;
	unsigned int width = 0;
	unsigned int height = 0;
    string URI;
};

class CICADA_CPLUS_EXTERN CicadaThumbnailParser
{
public:
	CicadaThumbnailParser(void);
	virtual ~CicadaThumbnailParser(void);
    list<ThumbnailInfo> setInputData(const std::string& input);

private:
    bool getLine(const std::string &input, std::size_t &startPos, std::string &outLine);
	bool getText(const std::string &line, ThumbnailInfo &info);
    bool getTime(const std::string &line, int64_t &startTime, unsigned int &duration);
	bool covertTime(std::string &time, int64_t &hour, int64_t &min, int64_t &sec, int64_t &ms);
};


#endif // CicadaThumbnailParser_H__
