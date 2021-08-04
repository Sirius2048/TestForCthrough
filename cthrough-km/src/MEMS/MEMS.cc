//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "../packet/Packet_m.h"
#include "../packet/Setup_m.h"

using namespace omnetpp;

/**
 * Demonstrates static Crossbar, utilizing the cTopology class.
 */
class MEMS : public cSimpleModule
{
  private:

//    simsignal_t dropSignal;
    simsignal_t outputIfSignal;
    cOutVector packetVector, latencyVector;
    cDoubleHistogram latencyStats;
    std::vector<int> outputGate;    //outputGate:一个vector类型数据，存储MEMS输出端口号（outputGate的index对应输入端口）（即根据outputGate可以得到MEMS的端口连接关系）

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

Define_Module(MEMS);    //所有定义的模块类必须通过宏Define_Module()注册


void MEMS::initialize()
{
    outputIfSignal = registerSignal("outputIf");
    outputGate.assign(this->gateSize("ToR"), -1);
    packetVector.setName("packet");
    latencyVector.setName("Inter-Rack Latency");
    latencyStats.setName("Inter-Rack Latency");
    latencyStats.setRangeAutoUpper(0);
}

void MEMS::handleMessage(cMessage *msg) //MEMS::handleMessage(cMessage *msg):当MEMS模块收到消息/发生事件后，立即调用handleMessage()函数处理该消息/事件并返回结果
{
    //set up the routing table(corresponding relationship between MEMS input port to output port) according to the message from MEMS controller
    if (msg->arrivedOn("MEMS_Controller$i")) {  //如果消息是来自MEMS_Controller$i端口的控制包
        Setup *pk = check_and_cast<Setup *>(msg);
        outputGate[pk->getInput()]=pk->getOutput(); //Input和Output都是Setup型的控制包内的信息
        EV << "MEMS configure crossbar input " << pk->getInput() << " to output " << pk->getOutput() << endl;
        delete pk;  //丢弃接收到的消息
    }

    //forward the packet(message) from ToR according to the routing table setup above
    else if (msg->arrivedOn("ToR$i")) { //如果消息是来自ToR$i端口的普通数据包
        EV << "MEMS Starting transmission of " << msg << endl;

        Packet *pk = check_and_cast<Packet *>(msg); //check_and_cast：强制类型转换+自动报错（将msg强制转换成Packet类型）。如果转换后的类型为NULL，则报错；如果
        int inGateIndex=pk->getArrivalGate()->getIndex();
        int outGateIndex = outputGate.at(inGateIndex);
        EV << "forwarding packet " << pk->getName() << " on gate index " << outGateIndex << endl;
        pk->setHopCount(pk->getHopCount()+1);
        emit(outputIfSignal, outGateIndex);

        packetVector.record(1);
        latencyVector.record(simTime()-pk->getTimestamp());
        latencyStats.collect(simTime()-pk->getTimestamp());

        send(pk, "ToR$o", outGateIndex);    //将数据包转发给ToR
//        emit(txBytesSignal, (long) pk->getByteLength());
//        Schedule an event for the time when last bit will leave the gate.
//        simtime_t endTransmission = gate("out", outGateIndex)->getTransmissionChannel()->getTransmissionFinishTime();
//        Setup *tmp = new Setup();
//        tmp->setInput(inGateIndex);
//        tmp->setOutput(outGateIndex);
//        tmp->setFlag(true);
//        sendDelayed(tmp, endTransmission-simTime(), "configurator$o");
    }
}

void MEMS::finish(){    //MEMS::finish():在仿真正常结束时，记录模块MEMS类统计数据成员所累积的数据
    latencyStats.recordAs("Inter-Rack Latency");
}
