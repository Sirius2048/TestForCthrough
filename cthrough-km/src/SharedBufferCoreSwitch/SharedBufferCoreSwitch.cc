#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "../packet/Packet_m.h"
#include "../packet/Setup_m.h"
#include "../packet/BufferLength_m.h"

using namespace std;
using namespace omnetpp;

class SharedBufferCoreSwitch: public cSimpleModule {
    private:
        int numOfRack;
        vector < Setup * > endTransmissionEvent;
    public:
        vector< cQueue > queue;
    protected:
        virtual void initialize();
        virtual int routeAlgorithm(Packet *pk);
        virtual void handleMessage(cMessage *msg);
};

Define_Module(SharedBufferCoreSwitch);

void SharedBufferCoreSwitch::initialize() {
    numOfRack = par("numOfRack");
    for (int i=0; i<numOfRack; i++){
        char qname[10];
        sprintf(qname,"queue%d", i);
        queue.push_back(cQueue(qname));
    }
    for (int i=0; i<numOfRack; i++){
        char pname[10];
        sprintf(pname,"outport%d", i);
        Setup *tmp = new Setup(pname);
        tmp->setBufferId(i);
        tmp->setOutput(i);
        endTransmissionEvent.push_back(tmp);
    }
}

int SharedBufferCoreSwitch::routeAlgorithm(Packet *pk){
    return pk->getDestRack();;
}

void SharedBufferCoreSwitch::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("port$i")) {
        Packet *pk = check_and_cast<Packet *>(msg);
        int bufferId = routeAlgorithm(pk);
        int outport = bufferId;
        if (outport == -1 || endTransmissionEvent[outport]->isScheduled())
            queue[bufferId].insert(pk);
        else {
            send(pk, "port$o", outport);
            simtime_t endTransmission = gate("port$o", outport)->getTransmissionChannel()->getTransmissionFinishTime();
            scheduleAt(endTransmission, endTransmissionEvent[outport]);
        }
    }
    else if (msg->isSelfMessage()) {
        Setup *tmp = check_and_cast<Setup *>(msg);
        int bufferId = tmp->getBufferId();
        int outport = tmp->getOutput();
        if (!queue[bufferId].isEmpty()) {
            Packet *pk = (Packet *) queue[bufferId].pop();
            send(pk, "port$o", outport);
            simtime_t endTransmission = gate("port$o", outport)->getTransmissionChannel()->getTransmissionFinishTime();
            scheduleAt(endTransmission, endTransmissionEvent[outport]);
        }
    }
}

