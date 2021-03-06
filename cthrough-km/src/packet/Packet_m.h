//
// Generated file, do not edit! Created by nedtool 5.6 from packet/Packet.msg.
//

#ifndef __PACKET_M_H
#define __PACKET_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



/**
 * Class generated from <tt>packet/Packet.msg:15</tt> by nedtool.
 * <pre>
 * //
 * // Represents a packet in the network.
 * //
 * packet Packet
 * {
 *     int srcAddr;	//源服务器在机架中的编号
 *     int srcRack;	//源服务器所在机架的编号（机架号）
 *     int destAddr;	//目的服务器在机架中的编号
 *     int destRack;	//目的服务器所在机架的编号（机架号）
 *     int hopCount;	//已转发的跳数
 *     int vlan;
 *     int flowId;		//流的id
 *     int pkId;		//数据包在流中的id(编号)
 * }
 * </pre>
 */
class Packet : public ::omnetpp::cPacket
{
  protected:
    int srcAddr;
    int srcRack;
    int destAddr;
    int destRack;
    int hopCount;
    int vlan;
    int flowId;
    int pkId;

  private:
    void copy(const Packet& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const Packet&);

  public:
    Packet(const char *name=nullptr, short kind=0);
    Packet(const Packet& other);
    virtual ~Packet();
    Packet& operator=(const Packet& other);
    virtual Packet *dup() const override {return new Packet(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual int getSrcAddr() const;
    virtual void setSrcAddr(int srcAddr);
    virtual int getSrcRack() const;
    virtual void setSrcRack(int srcRack);
    virtual int getDestAddr() const;
    virtual void setDestAddr(int destAddr);
    virtual int getDestRack() const;
    virtual void setDestRack(int destRack);
    virtual int getHopCount() const;
    virtual void setHopCount(int hopCount);
    virtual int getVlan() const;
    virtual void setVlan(int vlan);
    virtual int getFlowId() const;
    virtual void setFlowId(int flowId);
    virtual int getPkId() const;
    virtual void setPkId(int pkId);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const Packet& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, Packet& obj) {obj.parsimUnpack(b);}


#endif // ifndef __PACKET_M_H

