#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "../packet/Packet_m.h"
#include "../packet/Setup_m.h"
#include "../packet/BufferLength_m.h"

using namespace std;
using namespace omnetpp;

class SharedBufferToR: public cSimpleModule {
    private:
        int myRack;
        int numOfNode;
        int numOfRack;
        int numOfMEMS;
        int numOfCoreSwitch;
        double T_report;
        cMessage *buflenEvent;

        vector < Setup * > endTransmissionEvent;
        vector < int > bufferToOpticalPort;
    public:
        vector< cQueue > queue;
    protected:
        virtual void initialize();
        virtual int routeAlgorithm(Packet *pk);
        virtual void handleMessage(cMessage *msg);
};

Define_Module(SharedBufferToR);

void SharedBufferToR::initialize() {

    myRack = par("RackNo");
    numOfNode = par("numOfNode");
    numOfRack = par("numOfRack");
    numOfMEMS = par("numOfMEMS");
    T_report = par("T_report");
    numOfCoreSwitch = par("numOfCoreSwitch");

    bufferToOpticalPort.assign(numOfNode+numOfRack, -1);

    for (int i=0; i<numOfNode+numOfRack; i++){
        char qname[10];
        sprintf(qname,"queue%d", i);
        queue.push_back(cQueue(qname));
    }
    for (int i=0; i<numOfNode+numOfMEMS; i++){
        char pname[10];
        sprintf(pname,"outport%d", i);
        Setup *tmp = new Setup(pname);
        tmp->setBufferId(i);
        tmp->setOutput(i);
        endTransmissionEvent.push_back(tmp);    //��vector���͵�endTransmissionEvent��������ֱ�ʾToR��Ӧ�˿������ŵ��Ƿ���У�-1����·�Ͽ�/�����ã�
    }

    buflenEvent = new cMessage("Buffer Length");
    scheduleAt(0.0, buflenEvent);
}

int SharedBufferToR::routeAlgorithm(Packet *pk){
    //��ȡ��ToR�Ա�����Ŀ�ķ������˿ں�/����id(ͬ���ܷ�����ͨ��ʱ)����ToR�Թ⽻�����˿ں�/����id(��ͬ���ܷ�����ͨ��ʱ)
    int bufferId;
    if (pk->getDestRack()==myRack){     //�����ͬһ�����ڵķ�����ͨ�ţ���Դ���ܺ�Ŀ�Ļ�����ͬʱ��
        bufferId = pk->getDestAddr();   //bufferId����ǣ�ToR��Ŀ�ķ������Ķ˿ں�
    }
    else{                                           //����ǲ�ͬ�����ڵķ�����ͨ�ţ���Դ���ܺ�Ŀ�Ļ��ܲ�ͬʱ��
        bufferId = pk->getDestRack() + numOfNode;   //bufferId����ǣ�ToR�Թ⽻�����Ķ˿ں�
    }
    return bufferId;
}

void SharedBufferToR::handleMessage(cMessage *msg) {

    //����(ת�����Ŷ�)����host/server��MEMS��message
    if (msg->arrivedOn("port$i")) { //�����message����������⽻����
        Packet *pk = check_and_cast<Packet *>(msg);
        int bufferId = routeAlgorithm(pk);
        int outport;
        //���message���Ա����ܷ�������ȷ��ToR��Ŀ�ķ�����������˿ں�
        if (bufferId < numOfNode){
            outport = bufferId;     //outport����ǣ�ToR�Ա�����Ŀ�ķ������Ķ˿ں�
        }
        //���message���Թ⽻������ȷ��ToR�Թ⽻����������˿ں�
        else{
            outport = bufferToOpticalPort[bufferId];    //outport����ǣ�Ŀ�Ļ��ܱ�ţ��⽻�����ı�ţ���������������������������
        }
        //�����ӦToR����˿ڵ��ŵ���æ(��message���ڴ���)�����յ���message���뻺�����
        if (outport == -1 || endTransmissionEvent[outport]->isScheduled())
            queue[bufferId].insert(pk); //���յ���message���뻺�����
        //�����ӦToR����˿ڵ��ŵ�����(��message���ڴ���)
        else {
            //���ݿ�����·����Ϣ��message����ȥ
            if(!gate("port$o", outport)->getTransmissionChannel()->isBusy())    //����Ӧ�˿ڵ��ŵ���æʱ��ִ�����²�����
                send(pk, "port$o", outport);    //��messageת����������Ŀ��������ͬ���ܼ������ͨ�ţ���⽻��������ͬ���ܼ������ͨ�ţ�
            simtime_t endTransmission = gate("port$o", outport)->getTransmissionChannel()->getTransmissionFinishTime(); //endTransmission��ToR��Ӧ�˿��������ŵ�����message������ʱ��
            //��һ����ʱ�����ŵ��տ�ʼ����ʱ���͸�����Ϣ
            if(!endTransmissionEvent[outport]->isScheduled())
                scheduleAt(endTransmission, endTransmissionEvent[outport]);
        }
    }

    //��������MEMS_Controller��message
    //���룺����ת����ת��message
        else if (msg->arrivedOn("MEMS_Controller$i")) {
            Setup *tmp = check_and_cast<Setup *>(msg);
            int outport = tmp->getInput() + numOfNode;      //�⽻�������
            int bufferId = tmp->getBufferId() + numOfNode;  //Ŀ��ToR���
            //�������·��ͨ��
            if (tmp->getFlag()){
                bufferToOpticalPort[bufferId] = outport;
                endTransmissionEvent[outport]->setBufferId(bufferId);
                if(!endTransmissionEvent[outport]->isScheduled())
                    scheduleAt(simTime(), endTransmissionEvent[outport]);
    //            EV<<"Get MEMS Controller Message: Setup Light Path to Rack "<<tmp->getOutput()<<endl;
            }
            //�������·��ͨ���⽻���������ڼ䣩
            else{
                bufferToOpticalPort[bufferId]=-1;
                cancelEvent(endTransmissionEvent[outport]);
    //            EV<<"Get MEMS Controller Message: Disconnect Light Path to Rack "<<tmp->getOutput()<<endl;
            }
            delete tmp;
        }

    //����ToR������Ϣ����buflenEvent��
    else if (msg->isSelfMessage() && msg != buflenEvent) {  //���message�Ƿ�buflenEvent������Ϣִ�����²�����
        //cout << "send packet start" << endl;
        Setup *tmp = check_and_cast<Setup *>(msg);
        int bufferId = tmp->getBufferId();
        int outport = tmp->getOutput();
        if (!queue[bufferId].isEmpty()) {
            Packet *pk = (Packet *) queue[bufferId].pop();
            if(!gate("port$o", outport)->getTransmissionChannel()->isBusy())
                send(pk, "port$o", outport);
            EV << "send packet" << pk->getName() << "through" << outport <<endl;
            simtime_t endTransmission = gate("port$o", outport)->getTransmissionChannel()->getTransmissionFinishTime();
            if(!endTransmissionEvent[outport]->isScheduled())
                scheduleAt(endTransmission, endTransmissionEvent[outport]);
        }
    }

    //����ToR������ϢbuflenEvent
    else if (msg == buflenEvent){   //���message��msg buflenEvent
        BufferLength *pk = new BufferLength;
        pk->setBuf_lenArraySize(numOfRack);
        for (int i=0;i<numOfRack;i++)
            pk->setBuf_len(i, queue[numOfNode + i].getLength());
        send(pk, "MEMS_Controller$o");
        scheduleAt(simTime()+T_report, buflenEvent);
    }
}

