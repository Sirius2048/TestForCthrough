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
        endTransmissionEvent.push_back(tmp);    //在vector类型的endTransmissionEvent，存的数字表示ToR对应端口所连信道是否空闲（-1：链路断开/不可用）
    }

    buflenEvent = new cMessage("Buffer Length");
    scheduleAt(0.0, buflenEvent);
}

int SharedBufferToR::routeAlgorithm(Packet *pk){
    //获取：ToR对本机架目的服务器端口号/缓存id(同机架服务器通信时)，或，ToR对光交换机端口号/缓存id(不同机架服务器通信时)
    int bufferId;
    if (pk->getDestRack()==myRack){     //如果是同一机架内的服务器通信（当源机架和目的机架相同时）
        bufferId = pk->getDestAddr();   //bufferId存的是：ToR对目的服务器的端口号
    }
    else{                                           //如果是不同机架内的服务器通信（当源机架和目的机架不同时）
        bufferId = pk->getDestRack() + numOfNode;   //bufferId存的是：ToR对光交换机的端口号
    }
    return bufferId;
}

void SharedBufferToR::handleMessage(cMessage *msg) {

    //处理(转发、排队)来自host/server、MEMS的message
    if (msg->arrivedOn("port$i")) { //如果有message来自主机或光交换机
        Packet *pk = check_and_cast<Packet *>(msg);
        int bufferId = routeAlgorithm(pk);
        int outport;
        //如果message来自本机架服务器：确定ToR对目的服务器的输出端口号
        if (bufferId < numOfNode){
            outport = bufferId;     //outport存的是：ToR对本机架目的服务器的端口号
        }
        //如果message来自光交换机：确定ToR对光交换机的输出端口号
        else{
            outport = bufferToOpticalPort[bufferId];    //outport存的是：目的机架编号？光交换机的编号？？？？？？？？？？？？？？
        }
        //如果对应ToR输出端口的信道繁忙(有message正在传输)：将收到的message存入缓存队列
        if (outport == -1 || endTransmissionEvent[outport]->isScheduled())
            queue[bufferId].insert(pk); //将收到的message存入缓存队列
        //如果对应ToR输出端口的信道空闲(无message正在传输)
        else {
            //根据控制器路由信息将message发出去
            if(!gate("port$o", outport)->getTransmissionChannel()->isBusy())    //当对应端口的信道不忙时，执行以下操作：
                send(pk, "port$o", outport);    //将message转发给本机架目的主机（同机架间服务器通信）或光交换机（不同机架间服务器通信）
            simtime_t endTransmission = gate("port$o", outport)->getTransmissionChannel()->getTransmissionFinishTime(); //endTransmission：ToR对应端口所连的信道发送message结束的时间
            //开一个定时器，信道刚开始空闲时发送该自信息
            if(!endTransmissionEvent[outport]->isScheduled())
                scheduleAt(endTransmission, endTransmissionEvent[outport]);
        }
    }

    //处理来自MEMS_Controller的message
    //猜想：根据转发表转发message
        else if (msg->arrivedOn("MEMS_Controller$i")) {
            Setup *tmp = check_and_cast<Setup *>(msg);
            int outport = tmp->getInput() + numOfNode;      //光交换机编号
            int bufferId = tmp->getBufferId() + numOfNode;  //目标ToR编号
            //如果光链路是通的
            if (tmp->getFlag()){
                bufferToOpticalPort[bufferId] = outport;
                endTransmissionEvent[outport]->setBufferId(bufferId);
                if(!endTransmissionEvent[outport]->isScheduled())
                    scheduleAt(simTime(), endTransmissionEvent[outport]);
    //            EV<<"Get MEMS Controller Message: Setup Light Path to Rack "<<tmp->getOutput()<<endl;
            }
            //如果光链路不通（光交换机配置期间）
            else{
                bufferToOpticalPort[bufferId]=-1;
                cancelEvent(endTransmissionEvent[outport]);
    //            EV<<"Get MEMS Controller Message: Disconnect Light Path to Rack "<<tmp->getOutput()<<endl;
            }
            delete tmp;
        }

    //处理ToR的自消息（非buflenEvent）
    else if (msg->isSelfMessage() && msg != buflenEvent) {  //如果message是非buflenEvent的自消息执行以下操作：
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

    //处理ToR的自消息buflenEvent
    else if (msg == buflenEvent){   //如果message是msg buflenEvent
        BufferLength *pk = new BufferLength;
        pk->setBuf_lenArraySize(numOfRack);
        for (int i=0;i<numOfRack;i++)
            pk->setBuf_len(i, queue[numOfNode + i].getLength());
        send(pk, "MEMS_Controller$o");
        scheduleAt(simTime()+T_report, buflenEvent);
    }
}

