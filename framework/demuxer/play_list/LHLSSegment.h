//
//  LHLSSegment.h
//  CicadaPlayerSDK
//
//  Created by weixin on 2020/9/21.
//

#ifndef LHLSSegment_h
#define LHLSSegment_h

#include "segment.h"
#include "SegmentPart.h"
#include <vector>
#include <mutex>

using namespace std;

namespace Cicada
{
    class LHLSSegment : public segment
    {
    public:
        LHLSSegment(uint64_t seq, string uri, uint64_t filesize);
        ~LHLSSegment();
        
        void updateParts(const vector<SegmentPart>& parts);
        void updateUriWithFileSize(string uri, uint64_t filesize);
        const vector<SegmentPart>& getSegmentParts();
        std::string getDownloadUrl() override;
        bool isDownloadComplete(bool& bHasUnusedParts);
        uint64_t getFileSize();
        void moveToNextPart();
        static uint64_t extractOffsetFromURI(string uri);
        static string appendRangeToURI(string uri, uint64_t from, uint64_t to);
        
    protected:
        uint64_t mFileSize;
        uint64_t mOffset = 0;
        vector<SegmentPart> mParts;
        int mPartsNextIndex = 0;
        recursive_mutex mMutex;
        string mDownloadUri = "";
    };
}

#endif /* LHLSSegment_h */
