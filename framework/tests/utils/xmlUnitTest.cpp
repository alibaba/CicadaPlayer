//
// Created by moqi on 2020/4/2.
//
#include "gtest/gtest.h"
#include <data_source/dataSourcePrototype.h>
#include <string>
#include <xml/DOMParser.h>
using namespace Cicada;
using namespace std;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    //  ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}

TEST(xml, DOMParser)
{
    DOMParser parser{};
    string url = "http://dash.edgesuite.net/dash264/TestCases/1/manifest.mpd";
    IDataSource *dataSource = dataSourcePrototype::create(url, nullptr);
    dataSource->Open(0);

    int size = dataSource->Seek(0, SEEK_SIZE);
    auto *buffer = static_cast<uint8_t *>(malloc(size));

    int len = 0;
    while (len < size) {
        int ret = dataSource->Read(buffer + len, size - len);
        if (ret > 0) {
            len += ret;
        } else
            break;
    }

    if (len > 0) {
        bool ret = parser.parse(reinterpret_cast<const char *>(buffer),size);
        if (ret){
            parser.print();
        }
    }
    free(buffer);
    delete dataSource;
}