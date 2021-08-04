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

#include <omnetpp.h>
#include "../packet/Setup_m.h"
#include "../packet/BufferLength_m.h"
#include "KM.h"
#include "Hungarian.h"
#include "Edmonds.h"
#include "WangXi.h"
#include "WangXiOld.h"
#include "ISLIP.h"
//#define MEMS_ID 0

using namespace omnetpp;

typedef struct {
    int inport;
    int outport;
    int bufferId;

} Request;
using namespace std;

class MEMS_Controller : public cSimpleModule
{
  private:
    int numOfGate;
    int numOfMEMS;   
    int algorithm_ID; 
    int MEMS_ID;
    double oDataRate;   //add by xxr!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    vector< vector<Request> > request, accept;//
    cMessage *selfmsg;
    vector< vector<int> > links;
    vector< vector<double> > buf_len;
    vector<bool> lock;
    simtime_t T_day;
    simtime_t T_night;
    simtime_t T_report;
    Algorithm *algorithm;
    cOutVector failureRate;
    cDoubleHistogram buflenStats;
    double deadlineWeight;
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void allocate();
};

Define_Module(MEMS_Controller);


void MEMS_Controller::initialize()
{
    deadlineWeight = par("deadlineWeight");
    failureRate.setName("failureRate");
    buflenStats.setName("BufferLength");
    buflenStats.setRangeAutoUpper(0);
    numOfGate = this->gateSize("ToR");  //numOfGate:MEMS_Controller模块中名为ToR的端口的尺寸（与ToR相连的端口个数）
    numOfMEMS = par("numOfMEMS");       //numOfMEMS:MEMS_Controller模块中与MEMS相连的端口个数
    T_day = par("T_day");       //par(): .ned文件-->.cc文件
    T_night = par("T_night");
    T_report = par("T_report");
    algorithm_ID = par("algorithm_ID");
    //************************* add by xxr ************************//
    oDataRate = par("oDataRate");
    //*************************************************************//
    MEMS_ID = 0;
    buf_len.resize(numOfGate);
    links.resize(numOfMEMS);
    for (int i=0;i<numOfMEMS;i++){  //links初始化为一个二维向量，长度为numOfMEMS，宽度为numOfGate，每个参数均为-1
        links[i].assign(numOfGate, -1);
    }
    for (int i=0;i<numOfGate;i++){  //buf_len初始化为一个二维向量，长度和宽度均为numOfGate，每个参数均为-1
        buf_len[i].assign(numOfGate, 1);
    }
    request.resize(numOfGate);
    accept.resize(numOfGate);
    selfmsg = new cMessage();
    lock.assign(numOfGate, false);
    switch(algorithm_ID)
    {
        case 0:
        {
            algorithm = new KM;
            break;
        }
        case 1:
        {
            algorithm = new Edmonds;
            break;
        }
        case 2:
        {
            algorithm = new ISLIP;
            break;
        }
        case 3:
        {
            algorithm = new WangXi;
            algorithm->setDeadlineWeight(3);
            break;
        }
        default:
            break;
    }
    algorithm->init(numOfGate,numOfMEMS);
    if (numOfMEMS>0)
        scheduleAt(T_night*0.1, selfmsg);//？？？？？？？？？？？？？？？？？？？？？？？？？
}

void MEMS_Controller::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()){          //若message来自光控制器自身(定时器)
        //Self Message.
        allocate();
        scheduleAt(T_report+simTime(), selfmsg);    //时长为T_report的定时器
    }
    else if(msg->arrivedOn("ToR$i")){   //若message来自ToR，则记录所有buffer的尺寸大小
        BufferLength *tmp = check_and_cast<BufferLength *>(msg);
        int inport = tmp->getArrivalGate()->getIndex();
        //记录对应端口的缓存中的包大小
        for (int i=0;i<numOfGate;i++){  //遍历所有与ToR相连的gate
            buf_len[inport][i] = tmp->getBuf_len(i);
            if (inport!=i){
                buflenStats.collect(buf_len[inport][i]);
            }
        }
        delete tmp;
    }
}

void MEMS_Controller::allocate(){
//    cout<< algorithm->getNumber()<<endl;

//    //hungarian *algorithm = new hungarian(buf_len);

//Debug
//    EV<<"MEMS "<<MEMS_ID<<endl;
    EV<<"MEMS Traffic matrix:"<<endl;

    for (int i=0;i<numOfGate;i++){
        for (int j=0;j<numOfGate;j++)
            EV<<setw(10)<<setfill(' ')<<buf_len[i][j];
        EV<<endl;
    }

    if (T_day==T_report){
        algorithm->setBufLen(buf_len);
//**********************       原代码             ******************************//
//        vector< vector<int> >linky = algorithm->multicalculate();
//**********************************************************************//

        //************************* add by xxr ************************//
        vector< vector<int> >linky = algorithm->multicalculate((double)T_day,(double)T_night,(double)oDataRate);
        //*************************************************************//

        failureRate.record(1.0*algorithm->getNumber()/numOfGate/numOfGate);
        for(int MEMS_ID=0;MEMS_ID<numOfMEMS;MEMS_ID++){
    //        EV<<"Light path of MEMS "<<i<<endl;
            for(int srcToR=0;srcToR<numOfGate;srcToR++){
                if (linky[srcToR][MEMS_ID]==-1){
                    continue;
                }
                int bufferId = linky[srcToR][MEMS_ID];
                int dstToR  = linky[srcToR][MEMS_ID];
                Setup *setup = new Setup();
                setup->setInput(srcToR);
                setup->setOutput(dstToR);
                setup->setBufferId(bufferId);
                setup->setFlag(true);
                //Setup MEMS.
                EV<<srcToR<<" connect to "<<dstToR<<endl;
                send(setup->dup(), "MEMS$o", MEMS_ID);
                setup->setInput(MEMS_ID);
                //Setup light path.
                send(setup->dup(), "ToR$o", srcToR);
                setup->setFlag(false);
                //Tear down light path.
                sendDelayed(setup, T_day-T_night, "ToR$o", srcToR); //将名为setup的message，延迟T_day-T_night这么长时间后，从index=srcToR的端口ToR$o发出
            }
        }
    }

    else{
        for (int i=0;i<numOfMEMS;i++){
            if (i==MEMS_ID)
                continue;
            for (int j=0;j<numOfGate;j++){
                if (links[i][j]!=-1){
                    buf_len[j][links[i][j]]=0;
                }
            }
        }

        algorithm->setBufLen(buf_len);
        vector<int> linky = algorithm->calculate();//通过上面的KM、Edmonds等其中一个算法，计算源-目的ToR的连接关系（路由）

        for(int srcToR=0;srcToR<numOfGate;srcToR++){
            if (linky[srcToR]==-1)
                continue;
            int dstToR = linky[srcToR];     //通过上面的KM、Edmonds等其中一个算法，得到各个ToR对应的目的ToR
            int bufferId = dstToR;
            Setup *setup = new Setup();
            setup->setInput(srcToR);        //将Setup型变量setup的 Input置为srcToR
            setup->setOutput(dstToR);       //将Setup型变量setup的 Output置为dstToR
            setup->setBufferId(bufferId);   //将Setup型变量setup的 BufferId置为bufferId
            setup->setFlag(true);           //将Setup型变量setup的 Flag置为true

            //Setup MEMS.
            EV<<srcToR<<" connect to "<<dstToR<<endl;
            send(setup->dup(), "MEMS$o", MEMS_ID);              //复制一个setup，发送给第MEMS_ID个光交换机
                                                                //发给MEMS的setup参数：Input=srcToR，Output=dstToR，BufferId=dstToR，Flag=true
            setup->setInput(MEMS_ID);

            //Setup light path.
            send(setup->dup(), "ToR$o", srcToR);                //复制一个setup，发送给第srcToR个ToR（建立光链路）
                                                                //发给ToR的setup参数：  Input=MEMS_ID，Output=dstToR，BufferId=dstToR，Flag=true
            setup->setFlag(false);

            //Tear down light path.
            sendDelayed(setup, T_day-T_night, "ToR$o", srcToR); //延迟T_day-T_night这么长时间后，将setup发送给ToR（拆除光链路/光链路不可用：模拟配置时间）
                                                                //发给ToR的setup参数：  Input=MEMS_ID，Output=dstToR，BufferId=dstToR，Flag=false
        }

        MEMS_ID++;
        MEMS_ID=MEMS_ID%numOfMEMS;
    }
}
