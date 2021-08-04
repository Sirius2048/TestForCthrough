/*
 * Algorithm.h
 *
 *  Created on: 2016Äê6ÔÂ1ÈÕ
 *      Author: lanhao34
 */

#ifndef ALGORITHM_H_
#define ALGORITHM_H_
#define INF 999999

#include<iostream>
#include<vector>
#include<time.h>
#include<cstring>
#include<cstdlib>
#include<cstdio>
#include<climits>
#include<algorithm>
#include<iomanip>

using namespace std;

class Algorithm {
public:
    Algorithm(){};
    virtual ~Algorithm(){temp.resize(1);temp[0].assign(1, -1);};
    virtual void setBufLen(vector< vector<double> > buf_len){};
    virtual void init(int _numOfRack, int _numOfMEMS){};
    virtual vector<int> calculate(){return temp[0];};
//    virtual vector< vector<int> > multicalculate(){return temp;};
    virtual vector< vector<int> > multicalculate(double T_day,double T_night,double oDataRate){return temp;};
    virtual double getResult(){return -1;};
    virtual int getNumber(){return -1;};
    virtual void setDeadlineWeight(double _deadlineWeight){};
private:
    vector< vector<int> > temp;
};
#endif /* ALGORITHM_H_ */
