class ISLIP:public Algorithm{
    public:
        virtual void init(int _numOfRack, int _numOfMEMS);
        virtual vector<int> calculate();
        virtual vector< vector<int> > multicalculate();
        virtual void setBufLen(vector< vector<double> > buf_len);
    private:
        int N;
        vector< vector<double> > w;
        vector< vector<int> > request, accept;
        vector<int> clockwise_x, clockwise_y;
        vector<bool> outport_lock;
    protected:
        vector<int> linky;
        int repeat;
        int numOfMEMS;
};

//ISLIP::ISLIP(int n){
//    for (int i=0;i<N;i++){
//        for (int j=0;j<N;j++)
//            cout<<setw(6)<<w[i][j]<<' ';
//        cout<<endl;
//    }
//}

void ISLIP::setBufLen(vector< vector<double> > buf_len){
    w = buf_len;
}

void ISLIP::init(int _numOfRack, int _numOfMEMS)
{
    repeat = 3;
    numOfMEMS = _numOfMEMS;
    N = _numOfRack;
    request.resize(N);
    accept.resize(N);
    clockwise_x.assign(N, 0);
    clockwise_y.assign(N, 0);
}
vector<int> ISLIP::calculate(){//extend the previous section,in this section,mainly tell us for the received message
    linky.assign(N, -1);
    outport_lock.assign(N, false);
//    EV<<"ISLIP Initialize Finished!"<<endl;
    for (int counter=0; counter<repeat; counter++){
        //Initialize
//        cout<<"ISLIP round "<<counter<<" :"<<endl;
        for (int i=0;i<N;i++){
            request[i].clear();
            accept[i].clear();
        }
        //Request
        for (int inport=0;inport<N;inport++){
            for (int outport=0;outport<N;outport++){
                if (w[inport][outport]>0){
                    request[outport].push_back(inport);
                }
            }
        }
        //Grant
        for (int outport=0;outport<N;outport++){
            if (request[outport].size()==0 || outport_lock[outport])
                continue;
            bool flag = false;
            for (vector<int>::iterator it = request[outport].begin(); it!=request[outport].end(); it++){
                if (*it >= clockwise_y[outport]){
                    clockwise_y[outport] = *it+1;
                    flag = true;
                    break;
                }
            }
            if (!flag){
                clockwise_y[outport] = 0;
                for (vector<int>::iterator it = request[outport].begin(); it!=request[outport].end(); it++){
                    if (*it >= clockwise_y[outport]){
                        clockwise_y[outport] = *it+1;
                        break;
                    }
                }
            }
//            cout<<clockwise_y[outport]-1<<endl;
            accept[clockwise_y[outport]-1].push_back(outport);
        }
        //EV<<"Grant Finished!"<<endl;
        //Accept
        for (int i=0;i<N;i++){
            if (accept[i].size()==0)
                continue;
//            cout<<"\tAccept "<<i<<" : ";
//            for (vector<int>::iterator it = accept[i].begin(); it!=accept[i].end(); it++)
//                cout<<*it<<' ';
//            cout<<endl;
            bool flag = false;
            for (vector<int>::iterator it = accept[i].begin(); it!=accept[i].end(); it++){
                if (*it >= clockwise_x[i]){
                    clockwise_x[i] = *it+1;
                    flag = true;
                    break;
                }
            }
            if (!flag){
                clockwise_x[i] = 0;
                for (vector<int>::iterator it = accept[i].begin(); it!=accept[i].end(); it++){
                    if (*it >= clockwise_x[i]){
                        clockwise_x[i] = *it+1;
                        break;
                    }
                }
            }
//            cout<<clockwise_x[i]-1<<endl;
            int inport = i;
            int outport = clockwise_x[i]-1;
            w[inport][outport]=0;
            outport_lock[outport] = true;
            linky[inport]=outport;
        }
        //EV<<"Accept Finished!"<<endl;
    }
    return linky;
}

vector< vector<int> > ISLIP::multicalculate(){
    vector< vector<int> > m;
    m.resize(N);
    for (int i=0; i<N; i++)
        m[i].clear();
    for (int counter=0; counter<numOfMEMS; counter++){
        calculate();
        for (int i=0; i<N; i++)
            m[i].push_back(linky[i]);
    }
    return m;
}



