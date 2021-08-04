//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

using namespace omnetpp;

/**
 * Point-to-point interface module. While one frame is transmitted,
 * additional frames get queued up; see NED file for more info.
 */
class L2Queue: public cSimpleModule {
private:
    long frameCapacity;

    cQueue queue;
    cMessage *endTransmissionEvent;

    simsignal_t qlenSignal;
    simsignal_t busySignal;
    simsignal_t queueingTimeSignal;
    simsignal_t dropSignal;
    simsignal_t txBytesSignal;
    simsignal_t rxBytesSignal;

public:
    L2Queue();
    virtual ~L2Queue();

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void startTransmitting(cMessage *msg);
    virtual void displayStatus(bool isBusy);
};

Define_Module(L2Queue);

L2Queue::L2Queue() {
    endTransmissionEvent = NULL;
}

L2Queue::~L2Queue() {
    cancelAndDelete(endTransmissionEvent);
}

void L2Queue::initialize() {
    queue.setName("queue");
    endTransmissionEvent = new cMessage("endTxEvent");  //新建一个cMessage类，并命名为endTransmissionEvent。该类的第一个参数(MessageName)为"endTxEvent"，用于仿真时在图中显示，第二个参数(MessageKind)为空

    frameCapacity = par("frameCapacity");

    qlenSignal = registerSignal("qlen");    //Returns the signal ID (handle) for the given signal name.
    busySignal = registerSignal("busy");
    queueingTimeSignal = registerSignal("queueingTime");
    dropSignal = registerSignal("drop");
    txBytesSignal = registerSignal("txBytes");
    rxBytesSignal = registerSignal("rxBytes");

    emit(qlenSignal, queue.getLength());
    emit(busySignal, false);
}

void L2Queue::startTransmitting(cMessage *msg) {
    EV << endl;
    EV << "Server Starting transmission of " << msg << endl;
    EV << endl;
    EV << "Serve Sending Time is : "<< simTime() << endl;
    EV << endl;
    msg->setTimestamp();    //此时间戳记录帧开始发的时刻
    long numBytes = check_and_cast<cPacket *>(msg)->getByteLength();    //numBytes = 该数据包(四舍五入后)的大小(单位：byte)（1byte=8bits）
    send(msg, "line$o");    //将记录时间戳的msg从line门的out口发出

    emit(txBytesSignal, (long) numBytes);

    // Schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmission =
            gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime(); //simtime_t型变量endTransmission存储信道(channel)传完数据的时刻（仿真时刻）
    //相当于：
    //  cChannel *txChannel = gate("line$o")->getTransmissionChannel();
    //  simtime_t endTransmission = txChannel->getTransmissionFinishTime();
    scheduleAt(endTransmission, endTransmissionEvent);  //定时器，若收到的msg为endTransmissionEvent，则说明此时信道空闲了，即没有其他message正在传输
}

void L2Queue::handleMessage(cMessage *msg) {

    // Transmission finished, we can start next one.
    //从缓存队列中将本机架服务器的数据包发出去（先发给ToR，再由ToR转发）
    if (msg == endTransmissionEvent) {  //定时器，若收到的msg为endTransmissionEvent，则说明此时信道空闲了，即没有其他message正在传输
//        EV << "Transmission finished.\n";
        if (queue.isEmpty()) {
            emit(busySignal, false);    //emit()函数：发射信号
        }
        //取出并发送队列中最早到达的message
        else {
            msg = (cMessage *) queue.pop(); //固定用法  此句意为将cQueue类的实例queue(即由msg组成的队列)的栈底元素取出，并赋给msg
                                            //(cMessage *) queue意为将cQueue类型的queue强制转换为cMessage类型（强制转换）
            emit(queueingTimeSignal, simTime() - msg->getTimestamp());
            emit(qlenSignal, queue.getLength());
            startTransmitting(msg);
        }
    }

    // pass up
    //将收到的数据包发送给本机架对应服务器
    else if (msg->arrivedOn("line$i")) {
        emit(rxBytesSignal,
                (long) check_and_cast<cPacket *>(msg)->getByteLength());
        EV<<"APP L2Queue Received " << msg<<endl;
        EV<<"APP L2Queue Received Time is : "<< simTime() << endl;
        send(msg, "out");
    }

    // message arrived on gate "in"
    //收到app发来的数据包时（本机架服务器发来的数据包）：
    else
    {
        // We are currently busy, so just queue up the packet.
        if (endTransmissionEvent->isScheduled()) {
            //缓存已满，删除此message
            if (frameCapacity && queue.getLength() >= frameCapacity) {
                EV << "APP L2Queue Received " << msg
                          << " but transmitter busy and queue full: discarding\n";
                emit(dropSignal,
                        (long) check_and_cast<cPacket *>(msg)->getByteLength());
                delete msg;
            }
            //缓存未满，将此message存入缓存队列
            else {
                EV << "L2Queue Received " << msg
                          << " but transmitter busy: queueing up\n";
                msg->setTimestamp();
                queue.insert(msg);
                emit(qlenSignal, queue.getLength());
            }
        }

        // We are idle(空闲), so we can start transmitting right away.
        else {
            EV<<endl;
            EV << "App L2Queue Received " << msg << endl;
            EV<<endl;
            EV << "App L2Queue Received time is : " << simTime() << endl;
            EV << endl;

            msg->setTimestamp();
            emit(queueingTimeSignal, SIMTIME_ZERO);
            startTransmitting(msg);
            emit(busySignal, true);
        }
    }
}

void L2Queue::displayStatus(bool isBusy) {
    getDisplayString().setTagArg("t", 0, isBusy ? "transmitting" : "idle");
    getDisplayString().setTagArg("i", 1,
            isBusy ? (queue.getLength() >= 3 ? "red" : "yellow") : "");
}

