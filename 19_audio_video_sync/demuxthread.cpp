//
// Created by zql on 2026/3/25.
//

#include "demuxthread.h"

// void PacketQueue::push(AVPacket* pkt){
//     std::unique_lock<std::mutex> lock(mtx);
//     queue.push(pkt);
//     // cond.notify_one();
//     // finished = true;
//     cond.notify_all();
// }
// int PacketQueue::size() {
//     std::unique_lock<std::mutex> lock(mtx);
//     return queue.size();
// }
// AVPacket* PacketQueue::pop(){
//     std::unique_lock<std::mutex> lock(mtx);
//     while(queue.empty()){
//         cond.wait(lock);
//     }
//     AVPacket* pkt = queue.front();
//     queue.pop();
//     return pkt;
// }
//
// void PacketQueue::clear() {
//     std::unique_lock<std::mutex> lock(mtx);
//     while (!queue.empty()) {
//         AVPacket* pkt = queue.front();
//         av_packet_free(&pkt);
//         queue.pop();
//     }
// }