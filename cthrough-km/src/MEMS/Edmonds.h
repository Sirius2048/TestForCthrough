/*
 * muilthungarian.cc
 *
 *  Created on: 2016��4��30��
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
    vector<int> linkx;  //���ս������Xiƥ���Y�㣨���գ�X���ڵ�����ƥ�䣩
    vector<int> linky;  //���ս������Yiƥ���X�㣨���գ�Y���ڵ�����ƥ�䣩
    vector<bool> cover; //��Ӧ�ڵ��Ƿ���������·��(��1��0)
    vector<bool> judge;
    vector<bool> ly;
    vector<vector<double> > w;      //������г���
    vector<vector<int> > temp;      //tmp�����һά��ʾ��ͬMEMS��������ţ��ڶ�ά��ʾ��MEMS��������linkx
    vector<vector<int> > temp_f;    //��tmp������ת�ò���
    vector<vector<bool> > mapping;  //��ӦԴ-Ŀ�Ľڵ���Ƿ���ͨ·(��1��0)
protected:
    virtual bool Find(int x);
};

//��vector�ͱ���mapping�еĶ�Ӧֵ��Ϊ0(��Ӧ��·send�˻������<4000kb)��1(��Ӧ��·send�˻������>4000kb)
void Edmonds::setBufLen(vector< vector<double> > buf_len) {
    w = buf_len; //Ȩֵ����buf_len
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

//��ʼ��
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

//����send�ڵ�����(�µ�)����·����1��0�������⣬������Ӧ����·(Դ-Ŀ�Ľڵ��Ӧ��ϵ)����linkx��linky��
bool Edmonds::Find(int i) {
    for (int j = 0; j < N; ++j) {
        if (mapping[i][j] && !cover[j]) {
            cover[j] = true;    //�� Yj ��������·��
            if (linky[j] == -1 || Find(linky[j])) {
                linkx[i] = j;
                linky[j] = i;
                return true;
            }
        }
    }
    return false;
}

//���ض�Ӧ����·(Դ-Ŀ�Ľڵ��Ӧ��ϵ)linkx�����⣬������Ӧ����·(Դ-Ŀ�Ľڵ��Ӧ��ϵ)����linkx��linky��
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

//�õ�����MEMS�����ƥ��linkx
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
        //ϣ���ڴ�ѭ���и���w

        for (int j = 0; j < N; j++) {
            if(mapping[j][linkx[j]])
                w[i][j] = w[i][j] - oDataRate * (T_day - T_night);
        }

        //*************************************************************//
        for (int j = 0; j < N; j++) {
            if (linkx[j] > -1) {    //linkx��Ӧ��ֵ
                if (mapping[j][linkx[j]] == true)
                    result += w[j][linkx[j]];
                mapping[j][linkx[j]] = false;
            }
        }
        temp[i]=linkx;  //linkx��ʾlinkx[0][0]�ĵ�ַ
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

