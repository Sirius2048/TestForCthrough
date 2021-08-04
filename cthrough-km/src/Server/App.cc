//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2008 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#ifdef _MSe_VER
#pragma warning(disable:4786)
#endif

#include <vector>
#include <omnetpp.h>
#include "../packet/Packet_m.h"
#include <string.h>

using namespace omnetpp;

/**
 * Generates traffic for the network.
 */
class App: public cSimpleModule {
private:
    // configuration
    int myAddress;
    int myRack;
    int numOfNode;
    int numOfRack;
    int vlan;

    std::vector<int> destAddresses;
    std::vector< std::vector<simtime_t> > FCT;  //FCT是一个二维vector，格式为FCT[机架号][该机架内的主机号]，其中记录的数据是simtime_t型
    simtime_t tmp_FCT;
    cPar *sendIATime;
   // cPar *flowIATime;
    cPar *packetLengthBytes;

    cMessage *sendPacket;
    long pkCounter;
    long flowCounter;
    long counter;

    int destAddress_flow;
    int destRack_flow;
    simtime_t Server_Rate;

    // signals
//    simsignal_t endToEndDelaySignal;
//    simsignal_t hopCountSignal;
//    simsignal_t sourceAddressSignal;

    long numSent;
    long numReceived;
    long numReceived_m;
    long numReceived_c;

    cDoubleHistogram latencyStats;
    cOutVector latencyVector;   //Responsible for recording vector simulation results (an output vector)
    cOutVector packetVector;

    cDoubleHistogram FCTStats;
    cOutVector FCTVector;

    cDoubleHistogram m_latencyStats;
    cOutVector m_latencyVector;
    cOutVector m_packetVector;
    cDoubleHistogram m_FCTStats;
    cOutVector m_FCTVector;

    cDoubleHistogram e_latencyStats;
    cOutVector e_latencyVector;
    cOutVector e_packetVector;
    cDoubleHistogram e_FCTStats;
    cOutVector e_FCTVector;

public:
    App();
    virtual ~App();

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void generateFlow();    
    virtual void uniform_traffic(); 
    virtual void local_traffic();
    virtual void shuffle_traffic();
    virtual void E_flow_20_percent();
    virtual void E_flow_10_percent();
    virtual void packet_level();
};

Define_Module(App); //将.cc文件（c++文件中的Module类）与.ned文件（ned文件中的module）联系起来

App::App() {
  //  gentrateFlow = NULL;
    sendPacket = NULL;
}

App::~App() {   //"~"+类名：析构函数（执行与构造函数相反的操作————释放对象使用的资源，并销毁非static成员）
    //cancelAndDelete(generateFlow);
    cancelAndDelete(sendPacket);
}

void App::initialize() {
    myAddress = par("address"); //par()：从同名的.ned文件中取出address参数的值，存入.cc文件的myAddress中
    myRack = par("rack_address");
    numOfNode = par("numOfNode");
    numOfRack = par("numOfRack");
    packetLengthBytes = &par("packetLength");
    EV << "packetlenghth is " << packetLengthBytes->longValue() <<endl;
    sendIATime = &par("sendIaTime");  // volatile parameter（每次操作时重新读取该变量）
    Server_Rate = par("Server_Rate");
   // flowIATime = &par("flowIaTime");  // volatile parameter

    FCT.resize(numOfRack);  //.resize：重新调整FCT这一%vector的尺寸，将其尺寸设置为numOfRack大小
    for (int Rack_ID=0; Rack_ID<numOfRack; Rack_ID++){  //遍历所有的rack
        FCT[Rack_ID].assign(numOfNode, -1); //.assign()：给FCT这一%vector分配值。括号中的第一个数：将要分配的元素个数(同时将%vector的尺寸变为该值)。括号中的第二个数：需要分配的元素值(将%vector中存的数据变为该值)
    }
    pkCounter = 0;
    flowCounter = 0;
    counter = 0;
    vlan = 0;

    destAddress_flow = -1;
    destRack_flow = -1;

    numSent = 0;
    numReceived = 0;
    numReceived_m = 0;
    numReceived_c = 0;
    WATCH(numSent);
    WATCH(numReceived);

    latencyStats.setName("Latency");
    latencyStats.setRangeAutoUpper(0);
    latencyVector.setName("Latency");
    packetVector.setName("packet");

    m_FCTStats.setName("m_FCT");
    m_FCTStats.setRangeAutoUpper(0);
    m_FCTVector.setName("m_FCT");

    e_FCTStats.setName("e_FCT");
    e_FCTStats.setRangeAutoUpper(0);
    e_FCTVector.setName("e_FCT");

    FCTStats.setName("FCT");
    FCTStats.setRangeAutoUpper(0);
    FCTVector.setName("FCT");


    m_latencyStats.setName("m_Latency");
    m_latencyStats.setRangeAutoUpper(0);
    m_latencyVector.setName("m_Latency");
    m_packetVector.setName("m_packet");

    e_latencyStats.setName("e_Latency");
    e_latencyStats.setRangeAutoUpper(0);
    e_latencyVector.setName("e_Latency");
    e_packetVector.setName("e_packet");

    sendPacket = new cMessage("sendPacket");

//    scheduleAt(sendIATime->doubleValue(), generateFLow);
    generateFlow();
    scheduleAt(sendIATime->doubleValue(), sendPacket);  //将自消息(self-message)当作计时器使用，时间到后，若未收到发回的消息(说明该包丢失)，则重发该包

//    endToEndDelaySignal = registerSignal("endToEndDelay");
//    hopCountSignal = registerSignal("hopCount");
//    sourceAddressSignal = registerSignal("sourceAddress");
}

void App::uniform_traffic(){
    //等概随机通信（通信的目的机架号、目的机架内的主机号均随机生成）
    destRack_flow = intuniform(0, numOfRack-1); //intuniform(a,b):按照均匀分布（等概率）取[a,b]间的一个随机整数
    destAddress_flow = intuniform(0, numOfNode-1);
    while (myRack==destRack_flow)// && myAddress==destAddress_flow) //当随机出的目的机架号与发包机架号相同，重新随机目的机架号
    {
        destAddress_flow = intuniform(0, numOfNode-1);
        destRack_flow = intuniform(0, numOfRack-1);
    }
}

void App::local_traffic(){
    int distribution_number_address = intuniform(1,10);
    if(distribution_number_address <= 7 )   //以70%的概率机架内通信
    {
        //在当前机架内部进行通信
        destRack_flow = myRack;                         //目的机架号 = 发包机架号
        destAddress_flow = intuniform(0, numOfNode-1);  //目的服务器在机架内的编号随机生成
    }
    else                                     //以30%的概率机架间通信
    {
        //在机架间进行通信
        destRack_flow = intuniform(0, numOfRack-1);     //目的机架号随机生成
        destAddress_flow = intuniform(0, numOfNode-1);  //目的服务器在机架内的编号随机生成
    }

    while (myAddress==destAddress_flow && myRack==destRack_flow)
    {
        destAddress_flow = intuniform(0, numOfNode-1);
    }
}

void App::shuffle_traffic(){
    int stride = 3;
    int distribution_number_flow = intuniform(1,10);
    if(distribution_number_flow <= 8 )  //以80%的概率跨3个机架通信
    {
        destRack_flow = (myRack + stride) % numOfRack;  //跨stride个机架通信
        destAddress_flow = intuniform(0, numOfNode-1);  //目的服务器在机架内的编号随机生成
    }
    else                                //以20%的概率随机机架通信
    {
        destRack_flow = intuniform(0, numOfRack-1);
        destAddress_flow = intuniform(0, numOfNode-1);
    }
    while (myAddress==destAddress_flow) //&& myRack==destRack_flow)
    {
        destAddress_flow = intuniform(0, numOfNode-1);
    }
}

void App::E_flow_20_percent(){  //生成的大小流之比为  小流:大流=80%:20%
    int flow_size;
    int distribution_number = intuniform(1,100);//
    int packetsize = 1;



    //DCTCP traffic trace
    if(distribution_number >= 20 ){
        flow_size = intuniform(2,4);
        vlan = 1;
    }
    else{
        flow_size = intuniform(800,1000);
        vlan = 2;
    }
    counter = flow_size/packetsize;
    numSent+=flow_size;
}

void App::E_flow_10_percent(){  //生成的大小流之比为  小流:大流=90%:10%
    int flow_size;
    int distribution_number = intuniform(1,100);//
    int packetsize = 1;     //单个数据包的大小（单位：kb）

    //DCTCP traffic trace
    if(distribution_number >= 10 ){     //以90%概率做以下事
        flow_size = intuniform(2,4);    //生成大小为2kb、3kb、4kb的流
        vlan = 1;
    }
    else{                                   //以10%概率做以下事
        flow_size = intuniform(2900,3100);  //生成大小为2900kb~3100kb的流
        vlan = 2;
    }
    counter = flow_size/packetsize;         //生成流中的数据包个数
    numSent+=flow_size;                     //累计发送的全部流(象流+鼠流)的总大小
}

void App::packet_level(){
    counter = 1;
    numSent++;
}

void App::generateFlow(){
   uniform_traffic();       //确定数据包目的地址（随机生成目的机架号、目的机架的主机号）
//    local_traffic();
//    shuffle_traffic();
//
//    E_flow_5_percent();
    E_flow_10_percent();    //确定发送的流的大小
    //packet_level();
}
void App::handleMessage(cMessage *msg) {

    // Sending packet（发包）
    //如果在定时器超时时还未收到其他包（收到的包是作为定时器用的自消息），则重新发包
    if (msg == sendPacket) {

//        EV << endl;
//        EV <<"Rack:"<<myRack<<",Node:"<<myAddress<< "generating flow " << pkname << endl;
//        EV << endl;
//        EV <<"Rack:"<<myRack<<",Node:"<<myAddress<<" generating time is : "<< simTime() <<endl;
//        EV << endl;

        scheduleAt(simTime()+sendIATime->doubleValue()*counter, sendPacket);    //simTime()：获取当前仿真时刻
                                                                                //这句代码的意思是：每隔simTime()+sendIATime发送一次数据包
        for (int i=counter;i>0;i--){
            char pkname[40];
            sprintf(pkname, "pk-%d,%d-to-%d,%d-vlan%d-flow%ld-#%ld", myRack, myAddress, destRack_flow, destAddress_flow, vlan, flowCounter, i);

            //设置packet包的具体内容
            Packet *pk = new Packet(pkname);    //定义一个Packet型指针
            pk->setByteLength(packetLengthBytes->longValue());
            pk->setSrcAddr(myAddress);  //把本文件(.cc文件)中的myAddress值赋给Packet.msg文件中的srcAddr（Packet.msg文件中定义了Packet类）
            pk->setSrcRack(myRack);
            pk->setDestAddr(destAddress_flow);
            pk->setDestRack(destRack_flow);
            pk->setVlan(vlan);
            pk->setFlowId(flowCounter);
            pk->setPkId(i);
            send(pk, "out");
        }
        generateFlow();
        flowCounter++;
    }

    // Handle incoming packet （收包）
    else {

        Packet *pk = check_and_cast<Packet *>(msg);     //check_and_cast<Packet *>(msg)：检查msg是否是Packet类型，若不是（或msg是nullptr），则显示一个错误信息

        if (FCT[pk->getSrcRack()][pk->getSrcAddr()]==-1){                   //当该机架中该主机的FCT数据未记录（FCT数据为初始化时的-1）时
            FCT[pk->getSrcRack()][pk->getSrcAddr()]=pk->getTimestamp();     //将此时的时间戳（发送时刻）记录下来作为FCT数据
        }

        else if(pk->getPkId()==1){
            tmp_FCT = pk->getArrivalTime()-FCT[pk->getSrcRack()][pk->getSrcAddr()];     //tmp_FCT = 收到包的时刻 - FCT        ？？？？？？？？？？？为什么tmp_FCT可以作为FCT记录下来？？？？？？？？？？？？？？？？？？？
            if (pk->getVlan()==1){  //光网络
                m_FCTVector.record(tmp_FCT);    //Convenience method, delegates to record(double).
                m_FCTStats.collect(tmp_FCT);    //Convenience method, delegates to collect(double).
            }
            else{                   //电网络
                e_FCTVector.record(tmp_FCT);
                e_FCTStats.collect(tmp_FCT);
            }
            FCTVector.record(tmp_FCT);
            FCTStats.collect(tmp_FCT);
            FCT[pk->getSrcRack()][pk->getSrcAddr()]=-1;
        }

        if (pk->getVlan()==1){  //handle packet in VLAN1（光网络）
            simtime_t m_latency = pk->getArrivalTime() - pk->getTimestamp();
            numReceived_m++;
            m_latencyVector.record(m_latency);  //
            m_latencyStats.collect(m_latency);  //
            m_packetVector.record(1);           //
        }
        else{                   //handle packet in VLAN2（电网络）
            simtime_t e_latency = pk->getArrivalTime() - pk->getTimestamp();
            numReceived_c++;
            e_latencyVector.record(e_latency);
            e_latencyStats.collect(e_latency);
            e_packetVector.record(1);
        }

        EV << endl;
        EV <<"Rack:"<<myRack<<",Node:"<<myAddress<<"received packet " << pk->getName() << endl;
        EV << endl;
        EV <<"Rack:"<<myRack<<",Node:"<<myAddress<<"end time"<< simTime() << endl;

        EV << endl;
//                " after "<< pk->getHopCount() << " hops" << endl;
//        emit(endToEndDelaySignal, pk->getArrivalTime() - pk->getCreationTime());
//        emit(hopCountSignal, pk->getHopCount());
//        emit(sourceAddressSignal, pk->getSrcAddr());

        // update statistics.
        simtime_t latency = pk->getArrivalTime() - pk->getTimestamp();  //getTimestamp()：返回消息的时间戳(message的发送时刻)
                                                                        //getArrivalTime()：返回收到消息的时刻(message的到达时刻)
                                                                        //latency：传播时延（在路径上传输时花费的时间）
        EV << "latency is " <<latency <<endl;
        numReceived++;
        latencyVector.record(latency);
        latencyStats.collect(latency);
        packetVector.record(1);
        delete pk;
    }
}

void App::finish(){
    // This function is called by OMNeT++ at the end of the simulation.
//    EV << "Sent:     " << numSent << endl;
//    EV << "Received: " << numReceived << endl;
//    EV << "Latency, min:    " << latencyStats.getMin() << endl;
//    EV << "Latency, max:    " << latencyStats.getMax() << endl;
//    EV << "Latency, mean:   " << latencyStats.getMean() << endl;
//    EV << "Latency, stddev: " << latencyStats.getStddev() << endl;

    double averagethroughout = numReceived/simTime();
    recordScalar("#average throughout",averagethroughout);

    recordScalar("#sent", numSent);
    recordScalar("#received", numReceived);
    recordScalar("#m_received", numReceived_m);
    recordScalar("#e_received", numReceived_c);

    latencyStats.recordAs("Latency");
    m_latencyStats.recordAs("m_Latency");
    e_latencyStats.recordAs("e_Latency");
    FCTStats.recordAs("FCT");
    m_FCTStats.recordAs("m_FCT");
    e_FCTStats.recordAs("e_FCT");
}


