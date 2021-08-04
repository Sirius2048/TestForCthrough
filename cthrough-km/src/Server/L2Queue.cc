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
    endTransmissionEvent = new cMessage("endTxEvent");  //�½�һ��cMessage�࣬������ΪendTransmissionEvent������ĵ�һ������(MessageName)Ϊ"endTxEvent"�����ڷ���ʱ��ͼ����ʾ���ڶ�������(MessageKind)Ϊ��

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
    msg->setTimestamp();    //��ʱ�����¼֡��ʼ����ʱ��
    long numBytes = check_and_cast<cPacket *>(msg)->getByteLength();    //numBytes = �����ݰ�(���������)�Ĵ�С(��λ��byte)��1byte=8bits��
    send(msg, "line$o");    //����¼ʱ�����msg��line�ŵ�out�ڷ���

    emit(txBytesSignal, (long) numBytes);

    // Schedule an event for the time when last bit will leave the gate.
    simtime_t endTransmission =
            gate("line$o")->getTransmissionChannel()->getTransmissionFinishTime(); //simtime_t�ͱ���endTransmission�洢�ŵ�(channel)�������ݵ�ʱ�̣�����ʱ�̣�
    //�൱�ڣ�
    //  cChannel *txChannel = gate("line$o")->getTransmissionChannel();
    //  simtime_t endTransmission = txChannel->getTransmissionFinishTime();
    scheduleAt(endTransmission, endTransmissionEvent);  //��ʱ�������յ���msgΪendTransmissionEvent����˵����ʱ�ŵ������ˣ���û������message���ڴ���
}

void L2Queue::handleMessage(cMessage *msg) {

    // Transmission finished, we can start next one.
    //�ӻ�������н������ܷ����������ݰ�����ȥ���ȷ���ToR������ToRת����
    if (msg == endTransmissionEvent) {  //��ʱ�������յ���msgΪendTransmissionEvent����˵����ʱ�ŵ������ˣ���û������message���ڴ���
//        EV << "Transmission finished.\n";
        if (queue.isEmpty()) {
            emit(busySignal, false);    //emit()�����������ź�
        }
        //ȡ�������Ͷ��������絽���message
        else {
            msg = (cMessage *) queue.pop(); //�̶��÷�  �˾���Ϊ��cQueue���ʵ��queue(����msg��ɵĶ���)��ջ��Ԫ��ȡ����������msg
                                            //(cMessage *) queue��Ϊ��cQueue���͵�queueǿ��ת��ΪcMessage���ͣ�ǿ��ת����
            emit(queueingTimeSignal, simTime() - msg->getTimestamp());
            emit(qlenSignal, queue.getLength());
            startTransmitting(msg);
        }
    }

    // pass up
    //���յ������ݰ����͸������ܶ�Ӧ������
    else if (msg->arrivedOn("line$i")) {
        emit(rxBytesSignal,
                (long) check_and_cast<cPacket *>(msg)->getByteLength());
        EV<<"APP L2Queue Received " << msg<<endl;
        EV<<"APP L2Queue Received Time is : "<< simTime() << endl;
        send(msg, "out");
    }

    // message arrived on gate "in"
    //�յ�app���������ݰ�ʱ�������ܷ��������������ݰ�����
    else
    {
        // We are currently busy, so just queue up the packet.
        if (endTransmissionEvent->isScheduled()) {
            //����������ɾ����message
            if (frameCapacity && queue.getLength() >= frameCapacity) {
                EV << "APP L2Queue Received " << msg
                          << " but transmitter busy and queue full: discarding\n";
                emit(dropSignal,
                        (long) check_and_cast<cPacket *>(msg)->getByteLength());
                delete msg;
            }
            //����δ��������message���뻺�����
            else {
                EV << "L2Queue Received " << msg
                          << " but transmitter busy: queueing up\n";
                msg->setTimestamp();
                queue.insert(msg);
                emit(qlenSignal, queue.getLength());
            }
        }

        // We are idle(����), so we can start transmitting right away.
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

