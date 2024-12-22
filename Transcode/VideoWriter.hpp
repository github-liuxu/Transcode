//
//  VideoWriter.hpp
//  Transcode
//
//  Created by 刘东旭 on 2024/11/24.
//

#ifndef VideoWriter_hpp
#define VideoWriter_hpp

#include <stdio.h>
#include "LXMessageHandler.hpp"

class ConvertWork;
class VideoWriter : public LXMessageHandler {
    
public:
    VideoWriter(ConvertWork *work);
    ~VideoWriter();
    void StartFileWriter(const char *filePath);
    void exec(LXMessage* message);
    const char *filePath = nullptr;
    ConvertWork *work = nullptr;
};

#endif /* VideoWriter_hpp */
