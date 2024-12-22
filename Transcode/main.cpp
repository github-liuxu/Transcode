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
    work->Convert("/Users/liudongxu/金晨卜卦.mp4", "/Users/liudongxu/金晨卜卦1.mp4", 0, 100);
    while (true) {
        sleep(1);
    }
    return 0;
}
