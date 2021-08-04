/*
 * muilthungarian.cc
 *
 *  Created on: 2016年4月30日
 *      Author: Administrator
 */

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include"Algorithm.h"
#include "../packet/Setup_m.h"
#include "../packet/BufferLength_m.h"

using namespace omnetpp;

class Edmonds:public Algorithm{
public:
    Edmonds(){};
    virtual ~Edmonds(){};
    virtual void setBufLen(vector< vector<double> > buf_len);
    virtual void init(int _numOfRack, int _numOfMEMS);
    virtual vector<int> calculate();
//    virtual vector< vector<int> > multicalculate();
    virtual vector< vector<int> > multicalculate(double _T_day,double _T_night,double _oDataRate){return temp;};
    virtual double getResult();
    virtual int getNumber();
private:
    int N;
    int number;
    int numOfMEMS;
    double result;
    vector<int> linkx;  //最终结果中与Xi匹配的Y点（最终：X各节点的最大匹配）
    vector<int> linky;  //最终结果中与Yi匹配的X点（最终：Y各节点的最大匹配）
    vector<bool> cover; //对应节点是否属于增广路径(是1否0)
    vector<bool> judge;
    vector<bool> ly;
    vector<vector<double> > w;      //缓存队列长度
    vector<vector<int> > temp;      //tmp数组第一维表示不同MEMS交换机编号，第二维表示该MEMS交换机的linkx
    vector<vector<int> > temp_f;    //对tmp矩阵做转置操作
    vector<vector<bool> > mapping;  //对应源-目的节点间是否有通路(有1无0)
protected:
    virtual bool Find(int x);
};

//将vector型变量mapping中的对应值置为0(对应链路send端缓存队列<4000kb)或1(对应链路send端缓存队列>4000kb)
void Edmonds::setBufLen(vector< vector<double> > buf_len) {
    w = buf_len; //权值等于buf_len
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (w[i][j] < 4000)
                mapping[i][j] = false;
            else
                mapping[i][j] = true;
        }
    }
    result = 0;
}

//初始化
void Edmonds::init(int _numOfRack, int _numOfMEMS) {
    N = _numOfRack;
    numOfMEMS = _numOfMEMS;
    temp.resize(numOfMEMS);
    for (int i = 0; i < numOfMEMS; i++)
        temp[i].resize(N);
    temp_f.resize(N);
    for (int i = 0; i < N; i++) {
        temp_f[i].resize(numOfMEMS);
    }
    cover.resize(N);
    mapping.resize(N);
    for (int i = 0; i < N; i++)
        mapping[i].resize(N);
}

//返回send节点有无(新的)增广路（有1无0）。此外，还将对应的链路(源-目的节点对应关系)存入linkx和linky中
bool Edmonds::Find(int i) {
    for (int j = 0; j < N; ++j) {
        if (mapping[i][j] && !cover[j]) {
            cover[j] = true;    //将 Yj 加入增广路径
            if (linky[j] == -1 || Find(linky[j])) {
                linkx[i] = j;
                linky[j] = i;
                return true;
            }
        }
    }
    return false;
}

//返回对应的链路(源-目的节点对应关系)linkx。此外，还将对应的链路(源-目的节点对应关系)存入linkx和linky中
vector<int> Edmonds::calculate() {

    linkx.assign(N, -1);
    linky.assign(N, -1);
    number = 0;

    for (int i = 0; i < N; ++i) {
        cover.assign(N, 0);
        Find(i);
    }
    return linkx;
}

//得到各个MEMS的最佳匹配linkx
vector<vector<int> > Edmonds::multicalculate(double _T_day,double _T_night,double _oDataRate) {
    //************************* add by xxr ************************//
    double T_day = _T_day;
    double T_night = _T_night;
    double oDataRate = _oDataRate;
    //*************************************************************//
    for (int i = 0; i < numOfMEMS; i++) {
        setBufLen(w);   //add by xxr!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        calculate();

        //************************* add by xxr ************************//
        //希望在此循环中更新w

        for (int j = 0; j < N; j++) {
            if(mapping[j][linkx[j]])
                w[i][j] = w[i][j] - oDataRate * (T_day - T_night);
        }

        //*************************************************************//
        for (int j = 0; j < N; j++) {
            if (linkx[j] > -1) {    //linkx对应数值
                if (mapping[j][linkx[j]] == true)
                    result += w[j][linkx[j]];
                mapping[j][linkx[j]] = false;
            }
        }
        temp[i]=linkx;  //linkx表示linkx[0][0]的地址
    }
    for (int j = 0; j < N; j++) {
        judge.assign(N, 0);
        for (int i = 0; i < numOfMEMS; i++) {
            if (temp[i][j] > -1) {
                if (judge[temp[i][j]] == 1 || j == temp[i][j]) {
                    temp[i][j] = -1;
                    ++number;
                } else
                    judge[temp[i][j]] = 1;
            } else
                ++number;

        }
    }
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < numOfMEMS; i++) {
            temp_f[j][i] = temp[i][j];
        }
    }
    return temp_f;
}
double Edmonds::getResult() {
    return result;
}
int Edmonds::getNumber() {
    return number;
}

