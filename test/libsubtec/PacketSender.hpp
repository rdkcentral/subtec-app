/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#pragma once

#include <memory>
#include <queue>
#include <thread>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include <atomic>

#ifdef SUBTEC_PACKET_DEBUG

#undef AAMPLOG_WARN
#undef AAMPLOG_INFO
#undef AAMPLOG_TRACE
#undef AAMPLOG_ERR
#undef AAMPLOG_DEBUG

#define AAMPLOG_WARN(...) printf
#define AAMPLOG_INFO(...) printf
#define AAMPLOG_TRACE(...) printf
#define AAMPLOG_ERR(...) printf
#define AAMPLOG_DEBUG(...) printf

#else
#include "priv_aamp.h"
#endif

#include "SubtecPacket.hpp"

#if defined (__APPLE__) || defined(UBUNTU)
const constexpr char *SOCKET_PATH = "/tmp/pes_data_main"; // simulator
#else
const constexpr char *SOCKET_PATH = "/run/subttx/pes_data_main"; // device
#endif

void runWorkerTask(void *ctx);

class PacketSender
{
public:    
    ~PacketSender();
    
    void Close();
    void Flush();
    bool Init();
    bool Init(const char *socket_path);
    void SendPacket(PacketPtr && packet);
    void senderTask();
    bool IsRunning();
    static PacketSender *Instance();
private:
    void closeSenderTask();
    void flushPacketQueue();
    void sendPacket(PacketPtr && pkt);
    bool initSenderTask();
    bool initSocket(const char *socket_path);

    std::thread mSendThread;
    int mSubtecSocketHandle;
    std::atomic_bool running;
    std::queue<PacketPtr> mPacketQueue;
    std::mutex mPktMutex;
    std::condition_variable mCv;
    std::mutex mStartMutex;
    int mSockBufSize;
    int mPktWriteFailCtr;
    std::string mSocketPath;
protected:
    PacketSender() : 
        mSendThread(), 
        mSubtecSocketHandle(-1), 
        running(false), 
        mPacketQueue(), 
        mPktMutex(), 
        mCv(),
        mStartMutex(),
        mSockBufSize(0),
        mPktWriteFailCtr(0),
        mSocketPath("")
        {}
};
