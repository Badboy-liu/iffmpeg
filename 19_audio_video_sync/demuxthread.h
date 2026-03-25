//
// Created by zql on 2026/3/25.
//

#ifndef IFFMPEG_DEMUXTHREAD_H
#define IFFMPEG_DEMUXTHREAD_H
#include <Qt6/QtCore/QThread>
#include <queue>

extern "C"{
#include <libavformat/avformat.h>
}

class PacketQueue {
public:
    void push(AVPacket* pkt){
        std::unique_lock<std::mutex> lock(mtx);
        queue.push(pkt);
        cond.notify_one();
    }

    AVPacket* pop(bool &ok){
        std::unique_lock<std::mutex> lock(mtx);

        while(queue.empty()){
            if (abort) {
                ok = false;
                return nullptr;
            }
            cond.wait(lock);
        }

        AVPacket* pkt = queue.front();
        queue.pop();
        ok = true;
        return pkt;
    }

    void clear() {
        std::unique_lock<std::mutex> lock(mtx);
        while (!queue.empty()) {
            AVPacket* pkt = queue.front();
            av_packet_free(&pkt);
            queue.pop();
        }
    }

    int size(){
        std::unique_lock<std::mutex> lock(mtx);
        return queue.size();
    }

    void stop(){
        std::unique_lock<std::mutex> lock(mtx);
        abort = true;
        cond.notify_all();
    }

private:
    std::queue<AVPacket*> queue;
    std::mutex mtx;
    std::condition_variable cond;
    bool abort = false;
};



class DemuxThread : public QThread {
public:

    DemuxThread(AVFormatContext* fmtCtx,int audioStreamIndex,int videoStreamIndex)
    :audioStreamIndex(audioStreamIndex),videoStreamIndex(videoStreamIndex)
    {
        audioQ = new PacketQueue();
        videoQ = new PacketQueue();
        this->fmtCtx = fmtCtx;
    }

    PacketQueue* audioQ;
    PacketQueue* videoQ;

    int audioStreamIndex = -1;
    int videoStreamIndex = -1;
    AVFormatContext* fmtCtx = NULL;

    void run() override {
        while (!isInterruptionRequested()) {
            // 🔥 ① 队列限速（就放这里）
            if (audioQ->size() > 50 ||
                videoQ->size() > 50)
            {
                QThread::msleep(10);
                continue;
            }

            AVPacket* pkt = av_packet_alloc();

            if (av_read_frame(fmtCtx, pkt) < 0) {
                av_packet_free(&pkt);

                audioQ->stop();
                videoQ->stop();
                break;
            }

            if (pkt->stream_index == videoStreamIndex) {
                videoQ->push(av_packet_clone(pkt));
            } else if (pkt->stream_index == audioStreamIndex) {
                audioQ->push(av_packet_clone(pkt));
            }
                av_packet_free(&pkt);

        }
    }
};



#endif //IFFMPEG_DEMUXTHREAD_H