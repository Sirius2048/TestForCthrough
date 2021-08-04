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
    std::vector< std::vector<simtime_t> > FCT;  //FCT��һ����άvector����ʽΪFCT[���ܺ�][�û����ڵ�������]�����м�¼��������simtime_t��
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

Define_Module(App); //��.cc�ļ���c++�ļ��е�Module�ࣩ��.ned�ļ���ned�ļ��е�module����ϵ����

App::App() {
  //  gentrateFlow = NULL;
    sendPacket = NULL;
}

App::~App() {   //"~"+����������������ִ���빹�캯���෴�Ĳ������������ͷŶ���ʹ�õ���Դ�������ٷ�static��Ա��
    //cancelAndDelete(generateFlow);
    cancelAndDelete(sendPacket);
}

void App::initialize() {
    myAddress = par("address"); //par()����ͬ����.ned�ļ���ȡ��address������ֵ������.cc�ļ���myAddress��
    myRack = par("rack_address");
    numOfNode = par("numOfNode");
    numOfRack = par("numOfRack");
    packetLengthBytes = &par("packetLength");
    EV << "packetlenghth is " << packetLengthBytes->longValue() <<endl;
    sendIATime = &par("sendIaTime");  // volatile parameter��ÿ�β���ʱ���¶�ȡ�ñ�����
    Server_Rate = par("Server_Rate");
   // flowIATime = &par("flowIaTime");  // volatile parameter

    FCT.resize(numOfRack);  //.resize�����µ���FCT��һ%vector�ĳߴ磬����ߴ�����ΪnumOfRack��С
    for (int Rack_ID=0; Rack_ID<numOfRack; Rack_ID++){  //�������е�rack
        FCT[Rack_ID].assign(numOfNode, -1); //.assign()����FCT��һ%vector����ֵ�������еĵ�һ��������Ҫ�����Ԫ�ظ���(ͬʱ��%vector�ĳߴ��Ϊ��ֵ)�������еĵڶ���������Ҫ�����Ԫ��ֵ(��%vector�д�����ݱ�Ϊ��ֵ)
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
    scheduleAt(sendIATime->doubleValue(), sendPacket);  //������Ϣ(self-message)������ʱ��ʹ�ã�ʱ�䵽����δ�յ����ص���Ϣ(˵���ð���ʧ)�����ط��ð�

//    endToEndDelaySignal = registerSignal("endToEndDelay");
//    hopCountSignal = registerSignal("hopCount");
//    sourceAddressSignal = registerSignal("sourceAddress");
}

void App::uniform_traffic(){
    //�ȸ����ͨ�ţ�ͨ�ŵ�Ŀ�Ļ��ܺš�Ŀ�Ļ����ڵ������ž�������ɣ�
    destRack_flow = intuniform(0, numOfRack-1); //intuniform(a,b):���վ��ȷֲ����ȸ��ʣ�ȡ[a,b]���һ���������
    destAddress_flow = intuniform(0, numOfNode-1);
    while (myRack==destRack_flow)// && myAddress==destAddress_flow) //���������Ŀ�Ļ��ܺ��뷢�����ܺ���ͬ���������Ŀ�Ļ��ܺ�
    {
        destAddress_flow = intuniform(0, numOfNode-1);
        destRack_flow = intuniform(0, numOfRack-1);
    }
}

void App::local_traffic(){
    int distribution_number_address = intuniform(1,10);
    if(distribution_number_address <= 7 )   //��70%�ĸ��ʻ�����ͨ��
    {
        //�ڵ�ǰ�����ڲ�����ͨ��
        destRack_flow = myRack;                         //Ŀ�Ļ��ܺ� = �������ܺ�
        destAddress_flow = intuniform(0, numOfNode-1);  //Ŀ�ķ������ڻ����ڵı���������
    }
    else                                     //��30%�ĸ��ʻ��ܼ�ͨ��
    {
        //�ڻ��ܼ����ͨ��
        destRack_flow = intuniform(0, numOfRack-1);     //Ŀ�Ļ��ܺ��������
        destAddress_flow = intuniform(0, numOfNode-1);  //Ŀ�ķ������ڻ����ڵı���������
    }

    while (myAddress==destAddress_flow && myRack==destRack_flow)
    {
        destAddress_flow = intuniform(0, numOfNode-1);
    }
}

void App::shuffle_traffic(){
    int stride = 3;
    int distribution_number_flow = intuniform(1,10);
    if(distribution_number_flow <= 8 )  //��80%�ĸ��ʿ�3������ͨ��
    {
        destRack_flow = (myRack + stride) % numOfRack;  //��stride������ͨ��
        destAddress_flow = intuniform(0, numOfNode-1);  //Ŀ�ķ������ڻ����ڵı���������
    }
    else                                //��20%�ĸ����������ͨ��
    {
        destRack_flow = intuniform(0, numOfRack-1);
        destAddress_flow = intuniform(0, numOfNode-1);
    }
    while (myAddress==destAddress_flow) //&& myRack==destRack_flow)
    {
        destAddress_flow = intuniform(0, numOfNode-1);
    }
}

void App::E_flow_20_percent(){  //���ɵĴ�С��֮��Ϊ  С��:����=80%:20%
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

void App::E_flow_10_percent(){  //���ɵĴ�С��֮��Ϊ  С��:����=90%:10%
    int flow_size;
    int distribution_number = intuniform(1,100);//
    int packetsize = 1;     //�������ݰ��Ĵ�С����λ��kb��

    //DCTCP traffic trace
    if(distribution_number >= 10 ){     //��90%������������
        flow_size = intuniform(2,4);    //���ɴ�СΪ2kb��3kb��4kb����
        vlan = 1;
    }
    else{                                   //��10%������������
        flow_size = intuniform(2900,3100);  //���ɴ�СΪ2900kb~3100kb����
        vlan = 2;
    }
    counter = flow_size/packetsize;         //�������е����ݰ�����
    numSent+=flow_size;                     //�ۼƷ��͵�ȫ����(����+����)���ܴ�С
}

void App::packet_level(){
    counter = 1;
    numSent++;
}

void App::generateFlow(){
   uniform_traffic();       //ȷ�����ݰ�Ŀ�ĵ�ַ���������Ŀ�Ļ��ܺš�Ŀ�Ļ��ܵ������ţ�
//    local_traffic();
//    shuffle_traffic();
//
//    E_flow_5_percent();
    E_flow_10_percent();    //ȷ�����͵����Ĵ�С
    //packet_level();
}
void App::handleMessage(cMessage *msg) {

    // Sending packet��������
    //����ڶ�ʱ����ʱʱ��δ�յ����������յ��İ�����Ϊ��ʱ���õ�����Ϣ���������·���
    if (msg == sendPacket) {

//        EV << endl;
//        EV <<"Rack:"<<myRack<<",Node:"<<myAddress<< "generating flow " << pkname << endl;
//        EV << endl;
//        EV <<"Rack:"<<myRack<<",Node:"<<myAddress<<" generating time is : "<< simTime() <<endl;
//        EV << endl;

        scheduleAt(simTime()+sendIATime->doubleValue()*counter, sendPacket);    //simTime()����ȡ��ǰ����ʱ��
                                                                                //���������˼�ǣ�ÿ��simTime()+sendIATime����һ�����ݰ�
        for (int i=counter;i>0;i--){
            char pkname[40];
            sprintf(pkname, "pk-%d,%d-to-%d,%d-vlan%d-flow%ld-#%ld", myRack, myAddress, destRack_flow, destAddress_flow, vlan, flowCounter, i);

            //����packet���ľ�������
            Packet *pk = new Packet(pkname);    //����һ��Packet��ָ��
            pk->setByteLength(packetLengthBytes->longValue());
            pk->setSrcAddr(myAddress);  //�ѱ��ļ�(.cc�ļ�)�е�myAddressֵ����Packet.msg�ļ��е�srcAddr��Packet.msg�ļ��ж�����Packet�ࣩ
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

    // Handle incoming packet ���հ���
    else {

        Packet *pk = check_and_cast<Packet *>(msg);     //check_and_cast<Packet *>(msg)�����msg�Ƿ���Packet���ͣ������ǣ���msg��nullptr��������ʾһ��������Ϣ

        if (FCT[pk->getSrcRack()][pk->getSrcAddr()]==-1){                   //���û����и�������FCT����δ��¼��FCT����Ϊ��ʼ��ʱ��-1��ʱ
            FCT[pk->getSrcRack()][pk->getSrcAddr()]=pk->getTimestamp();     //����ʱ��ʱ���������ʱ�̣���¼������ΪFCT����
        }

        else if(pk->getPkId()==1){
            tmp_FCT = pk->getArrivalTime()-FCT[pk->getSrcRack()][pk->getSrcAddr()];     //tmp_FCT = �յ�����ʱ�� - FCT        ����������������������Ϊʲôtmp_FCT������ΪFCT��¼������������������������������������������
            if (pk->getVlan()==1){  //������
                m_FCTVector.record(tmp_FCT);    //Convenience method, delegates to record(double).
                m_FCTStats.collect(tmp_FCT);    //Convenience method, delegates to collect(double).
            }
            else{                   //������
                e_FCTVector.record(tmp_FCT);
                e_FCTStats.collect(tmp_FCT);
            }
            FCTVector.record(tmp_FCT);
            FCTStats.collect(tmp_FCT);
            FCT[pk->getSrcRack()][pk->getSrcAddr()]=-1;
        }

        if (pk->getVlan()==1){  //handle packet in VLAN1�������磩
            simtime_t m_latency = pk->getArrivalTime() - pk->getTimestamp();
            numReceived_m++;
            m_latencyVector.record(m_latency);  //
            m_latencyStats.collect(m_latency);  //
            m_packetVector.record(1);           //
        }
        else{                   //handle packet in VLAN2�������磩
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
        simtime_t latency = pk->getArrivalTime() - pk->getTimestamp();  //getTimestamp()��������Ϣ��ʱ���(message�ķ���ʱ��)
                                                                        //getArrivalTime()�������յ���Ϣ��ʱ��(message�ĵ���ʱ��)
                                                                        //latency������ʱ�ӣ���·���ϴ���ʱ���ѵ�ʱ�䣩
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


