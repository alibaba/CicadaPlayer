
#ifndef CicadaRenderCBWrapper_H
#define CicadaRenderCBWrapper_H

class IAFFrame;

class CicadaRenderCBWrapper {
public:
    static bool OnRenderFrame(void *userData, IAFFrame *frame);
};


#endif //CicadaRenderCBWrapper_H
