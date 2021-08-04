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
    std::vector<int> outputGate;    //outputGate:һ��vector�������ݣ��洢MEMS����˿ںţ�outputGate��index��Ӧ����˿ڣ���������outputGate���Եõ�MEMS�Ķ˿����ӹ�ϵ��

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
};

Define_Module(MEMS);    //���ж����ģ�������ͨ����Define_Module()ע��


void MEMS::initialize()
{
    outputIfSignal = registerSignal("outputIf");
    outputGate.assign(this->gateSize("ToR"), -1);
    packetVector.setName("packet");
    latencyVector.setName("Inter-Rack Latency");
    latencyStats.setName("Inter-Rack Latency");
    latencyStats.setRangeAutoUpper(0);
}

void MEMS::handleMessage(cMessage *msg) //MEMS::handleMessage(cMessage *msg):��MEMSģ���յ���Ϣ/�����¼�����������handleMessage()�����������Ϣ/�¼������ؽ��
{
    //set up the routing table(corresponding relationship between MEMS input port to output port) according to the message from MEMS controller
    if (msg->arrivedOn("MEMS_Controller$i")) {  //�����Ϣ������MEMS_Controller$i�˿ڵĿ��ư�
        Setup *pk = check_and_cast<Setup *>(msg);
        outputGate[pk->getInput()]=pk->getOutput(); //Input��Output����Setup�͵Ŀ��ư��ڵ���Ϣ
        EV << "MEMS configure crossbar input " << pk->getInput() << " to output " << pk->getOutput() << endl;
        delete pk;  //�������յ�����Ϣ
    }

    //forward the packet(message) from ToR according to the routing table setup above
    else if (msg->arrivedOn("ToR$i")) { //�����Ϣ������ToR$i�˿ڵ���ͨ���ݰ�
        EV << "MEMS Starting transmission of " << msg << endl;

        Packet *pk = check_and_cast<Packet *>(msg); //check_and_cast��ǿ������ת��+�Զ�������msgǿ��ת����Packet���ͣ������ת���������ΪNULL���򱨴����
        int inGateIndex=pk->getArrivalGate()->getIndex();
        int outGateIndex = outputGate.at(inGateIndex);
        EV << "forwarding packet " << pk->getName() << " on gate index " << outGateIndex << endl;
        pk->setHopCount(pk->getHopCount()+1);
        emit(outputIfSignal, outGateIndex);

        packetVector.record(1);
        latencyVector.record(simTime()-pk->getTimestamp());
        latencyStats.collect(simTime()-pk->getTimestamp());

        send(pk, "ToR$o", outGateIndex);    //�����ݰ�ת����ToR
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

void MEMS::finish(){    //MEMS::finish():�ڷ�����������ʱ����¼ģ��MEMS��ͳ�����ݳ�Ա���ۻ�������
    latencyStats.recordAs("Inter-Rack Latency");
}
