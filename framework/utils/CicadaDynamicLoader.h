//
//  CicadaDynamicLoader.h
//  utils
//
//  Created by huang_jiafa on 2019/4/17.
//  Copyright © 2019 alibaba. All rights reserved.
//

#ifndef CicadaDynamicLoader_H__
#define CicadaDynamicLoader_H__

#include <string>

class CicadaDynamicLoader
{
public:
    CicadaDynamicLoader(const std::string& libName);
	virtual ~CicadaDynamicLoader(void);
	virtual bool loadLib();
    virtual bool getFunctionAddress(const std::string& functionName, void **functionAddress);
	virtual bool freeLib(void);

    static void * loadALib(const std::string& libName);
    static bool freeALib(void *Handle);

    static void setWorkPath(const std::string& workPath);

    // for iOS plugin
    static void addFunctionToMap(const std::string& functionName, void *functionAddress);
public:
    static std::string mWorkPath;

    std::string mLibName;
    void * mDynamicHandle = nullptr;
};

#endif // CicadaDynamicLoader_H__
