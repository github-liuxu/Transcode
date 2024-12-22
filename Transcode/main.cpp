//
//  main.cpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/16.
//

#include <iostream>
#include "ConvertWork.hpp"
#include <unistd.h>
int main(int argc, const char * argv[]) {
    // insert code here...
    ConvertWork *work = new ConvertWork();
    work->Convert("/Users/liudongxu/Downloads/11111.MP4", "/Users/liudongxu/11111.mp4", 0, 10000000);
    work->Start();
    while (true) {
        sleep(1);
    }
    return 0;
}
