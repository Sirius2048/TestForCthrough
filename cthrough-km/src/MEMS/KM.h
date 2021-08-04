#ifndef KM_H_INCLUDED
#define KM_H_INCLUDED

#include"Algorithm.h"
using namespace std;

class KM:public Algorithm{
    public:
        KM(){};
        virtual ~KM(){};
        virtual void setBufLen(vector< vector<double> > buf_len);
        virtual void init(int _numOfRack, int _numOfMEMS);
        virtual vector<int> calculate();
        virtual vector< vector<int> > multicalculate();
        virtual double getResult();
        virtual int getNumber();
    private:
        int N;
        int numOfMEMS;
        int number;
        double result;
        vector< vector<double> > w;
        vector<double> lx,ly;
        vector<int> linky,linkx;
        vector<int> visx,visy;
        vector<double> slack;
        vector<int> judge;
        vector < vector<int> > temp;
        vector < vector<int> > temp_f;
        int nx,ny;
    protected:
        virtual bool find(int x);
};

void KM::setBufLen(vector< vector<double> > buf_len){
    w = buf_len;
    for(int i=0; i<N; i++ )
        w[i][i]=-INF;
//    for (int i=0;i<N;i++){
//        for (int j=0;j<N;j++)
//            cout<<setw(6)<<w[i][j];
//        cout<<endl;
//    }
}

void KM::init(int _numOfRack, int _numOfMEMS)
{
    N = _numOfRack;
    nx = N;
    ny = N;
    numOfMEMS = _numOfMEMS;

    temp.resize(numOfMEMS);
    for (int i = 0; i < numOfMEMS; i++)
        temp[i].assign(N, -1);

    temp_f.resize(N);
    for (int i = 0; i < N; i++)
        temp_f[i].assign(numOfMEMS, -1);
}

bool KM::find(int x)//ÐÙÑÀÀûËã·¨
{
    visx[x] = true;
    for(int y = 0; y < ny; y++)
    {
        if(visy[y])
            continue;
        double t = lx[x] + ly[y] - w[x][y];

        if(t==0)
        {
            visy[y] = true;
            if(linky[y]==-1 || find(linky[y]))
            {
                linky[y] = x;
                linkx[x] = y;
                return true;
            }

        }

        else if(slack[y] > t)
            slack[y] = t;

    }

    return false;

}

vector<int> KM::calculate()
{
    linkx.assign(N, -1);
    linky.assign(N, -1);
    ly.assign(N, 0);
    lx.assign(N, -INF);
    for(int i = 0; i < nx; i++)
    {
        for(int j = 0; j < ny; j++)
        {
            if(w[i][j] > lx[i])
                lx[i] = w[i][j];
        }
    }
    for(int x = 0; x < nx; x++)
    {
        slack.assign(ny, INF);
        while(1)
        {
            visx.assign(N, 0);
            visy.assign(N, 0);
//            memset(visx,0,sizeof(visx));
//            memset(visy,0,sizeof(visy));
            if(find(x))
                break;

            double d = INF;
            for(int i = 0; i < ny; i++)
            {
                if(!visy[i] && d > slack[i])
                    d = slack[i];
            }
            for(int i = 0; i < nx; i++)
            {
                if(visx[i])
                    lx[i] -= d;
            }
            for(int i = 0; i < ny; i++)
            {
                if(visy[i])
                    ly[i] += d;
                else
                    slack[i] -= d;
            }



        }

    }
//    cout<<result<<endl;
    return linkx;

}
//=================================================================================================================

vector< vector<int> > KM::multicalculate()
{
    for(int i=0;i<numOfMEMS;i++)
    {
		//	 vector < vector<int> > set;
		calculate();

		temp[i]=linkx;
		for(int ToR_ID=0;ToR_ID<N;ToR_ID++)
		{
			if(linkx[ToR_ID]>-1)
			{
			// cout<<j<<" connect to "<<k[j]<<endl;
			    w[ToR_ID][linkx[ToR_ID]]=-INF;
			}
		}
    }
//////////////////////////////////////////////////////////////////////////////////////

    for(int MEMS_ID=0; MEMS_ID<numOfMEMS; MEMS_ID++)
    {
        for(int ToR_ID = 0; ToR_ID < N; ToR_ID++)
        {
            temp_f[ToR_ID][MEMS_ID]=temp[MEMS_ID][ToR_ID];
            //cout<<temp[v][MEMS_ID]<<" ";
        }
    }

    number = 0;
    for(int ToR_ID=0;ToR_ID<N;ToR_ID++)
    {
        judge.assign(N, -1);
        for(int MEMS_ID=0;MEMS_ID<numOfMEMS;MEMS_ID++)
        {
            if(judge[temp_f[ToR_ID][MEMS_ID]]==-1)
            {
                judge[temp_f[ToR_ID][MEMS_ID]]=MEMS_ID;
            }
            else
            {
//                cout<<"Delete link from "<<ToR_ID<<" to "<<temp_f[ToR_I][MEMS_ID]<<" through MEMS "<<MEMS_ID<<endl;
                temp_f[ToR_ID][MEMS_ID]=-1;
                number++;
            }
        }
    }
    return temp_f;
}

double KM::getResult()
{
    double result = 0;
    for(int i = 0; i < ny; i++)
    {
        if(linky[i]>-1)
            result += w[linky[i]][i];
    //
    }
    return result;
}

int KM::getNumber() {
    return number;
}

#endif // KM_H_INCLUDED
