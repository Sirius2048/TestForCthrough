
#include"Algorithm.h"
struct matrixOld{
    double w;
    int index_x;
    int index_y;
};
bool   greatermarkOld(const   matrixOld&   s1,const   matrixOld&   s2)
{
  return   s1.w   >   s2.w;
}
using namespace std;

class WangXiOld:public Algorithm{
    public:
      WangXiOld(){};
      virtual ~WangXiOld(){};
      virtual void init(int _numOfRack, int _numOfMEMS);
      virtual void setBufLen(vector<vector<double> > buf_len);
      virtual vector< vector<int> > multicalculate();
      virtual vector<int> calculate();
      virtual double getResult();
      virtual int getNumber();
      double deadlineWeight = 0;
      virtual void setDeadlineWeight(double _deadlineWeight);
    private:
      int N;
      int numOfMEMS;
      int number;
      double result;
      vector<vector <double> > w1;
      vector< vector<matrixOld> >buffer;
      vector<vector<matrixOld> > mems;
      vector<vector<matrixOld> > order;
      vector<vector<matrixOld> > diagonal;
      vector<vector<int> > form;
      vector<bool> form_x;
      vector<bool> form_y;
      vector<bool> judge;
      vector<vector<bool> > mapping;
      vector<int> lx,ly;
      vector<int> ox,oy;
      vector< vector<int> > linkx;
      vector< vector<int> > linky;
      vector< vector<int> > temp;
      int count_x;
      int count_y;
};
void WangXiOld::init(int _numOfRack, int _numOfMEMS)
{
      N = _numOfRack;
      numOfMEMS = _numOfMEMS;
      linkx.resize(N);
      linky.resize(N);
      form.resize(N);
      form_x.resize(N);
      form_y.resize(N);
      buffer.resize(N);
      judge.resize(N);
      for (int i=0; i<N; i++)
      {
          buffer[i].resize(N);
      }
}

void WangXiOld::setBufLen(vector< vector<double> > buf_len){
    w1 = buf_len;
    for (int i=0; i<N; i++)
    {
        for (int j=0; j<N; j++)
        {
            buffer[i][j].w=w1[i][j];
            buffer[i][j].index_x=i;
            buffer[i][j].index_y=j;
        }
        buffer[i][i].w=-INF;
    }
}

void WangXiOld::setDeadlineWeight(double _deadlineWeight){
    deadlineWeight = _deadlineWeight;
}

vector <int> WangXiOld::calculate(){
    multicalculate();
    return linkx[0];
}

vector< vector <int> > WangXiOld::multicalculate()
{
    number=0;
    lx.assign(N,numOfMEMS);
    ly.assign(N,numOfMEMS);
    ox.assign(N,numOfMEMS);
    oy.assign(N,numOfMEMS);
    count_x=N;
    count_y=N;
    for (int i=0; i<N; i++)
    {
       form[i].assign(N,0);
       linkx[i].resize(numOfMEMS);
       linky[i].resize(numOfMEMS);
    }
    while(count_y)
    {
        mems.resize(count_x);                //creat mems
        for(int i=0;i<count_x;i++)
        {
            mems[i].resize(count_y);
        }
        for(int i=0,m=0;i<N;i++)
        {
            if(lx[i]>0)
            {
                for(int j=0,n=0;j<N;j++)
                {
                    if(ly[j]>0)
                    {
                        mems[m][n]=buffer[i][j];
                        if(form[i][j]==1)
                            mems[m][n].w=0;
                        ++n;
                    }
                }
                ++m;
            }
        }

        order.resize(count_x);              //in order
        for(int i=0;i<count_x;i++)
        {
            order[i].resize(count_y);
        }
        for (int i=0; i<count_x; i++)
        {
            vector<matrixOld> a = mems[i];
            sort(a.begin(),a.end(),greatermarkOld);
            order[i]=a;
        }
//=============================================select     create the map
        mapping.resize(N);
        for(int i=0;i<N;i++)
        {
            mapping[i].clear();
            mapping[i].resize(N);
        }

        for(int i=0;i<count_x;i++)
        {
            for(int j=0, m=lx[mems[i][0].index_x];j<m;j++)
            {
                --ly[order[i][j].index_y];
                --lx[order[i][j].index_x];
                mapping[order[i][j].index_x][order[i][j].index_y]=true;
                form[order[i][j].index_x][order[i][j].index_y]=true;
            }
        }
        for(int j=0; j<N;j++)     //delete the more  and  update the map
        {
            if(ly[j]<0)
            {
                vector<matrixOld> temp ;
                temp.resize(oy[j]+abs(ly[j]));
                for(int i=0,m=0;i<N;i++)
                {
                    if(mapping[i][j]==true)
                    {
                       temp[m]=buffer[i][j];
                       ++m;
                    }
                }
                sort(temp.begin(),temp.end(),greatermarkOld);
                for(int x=oy[j],m=oy[j]-ly[j] ;x<m;x++)
                {
                    mapping[temp[x].index_x][temp[x].index_y]=false;
                    form[temp[x].index_x][temp[x].index_y]=false;
                    ++lx[temp[x].index_x];
                    ++ly[j];
                }
            }
        }
        ox=lx;
        oy=ly;
        count_y=N;
        count_x=N;
        for(int i=0;i<N;i++)
        {
            if(ly[i]==0)
                --count_y;
            if(lx[i]==0)
                --count_x;
        }
    }
    diagonal.resize(2*N-3);
    for(int p=1;p<2*N-2;p++)
    {
        for(int i=0;i<=p;i++)
        {
            if(i<N&&p-i<N)
            {
                if(form[i][p-i]==1)
                {
                    diagonal[p-1].push_back(buffer[i][p-i]);
                }
            }
            else
                continue;
        }
    }
//=====================================================================================================================
    for(int Num=0;Num<numOfMEMS;Num++)
    {
        vector<matrixOld>::iterator it;
        int count_f=0,count_f_max=0;
        int m=0;
        for(int i=0;i<2*N-3;i++)
        {
            count_f=diagonal[i].size();
            if(count_f>count_f_max)
            {
                count_f_max=count_f;
                m=i;
            }
        }
        form_x.assign(N,0);
        form_y.assign(N,0);
        for(int i=m;i<2*N-3;i++)
        {
            it=diagonal[i].begin();
            while(it!=diagonal[i].end())
            {
                if(form_x[it->index_x]==0&&form_y[it->index_y]==0)
                {
                    form_x[it->index_x]=1;
                    form_y[it->index_y]=1;
                    form[it->index_x][it->index_y]=-1;
                    linkx[it->index_x][Num]=it->index_y;
                    linky[it->index_y][Num]=it->index_x;
                    it=diagonal[i].erase(it);
                }
                else
                    ++it;
            }
        }
        for(int i=0;i<m;i++)
        {
            it=diagonal[i].begin();
            while(it!=diagonal[i].end())
            {
                if(form_x[it->index_x]==0&&form_y[it->index_y]==0)
                {
                    form_x[it->index_x]=1;
                    form_y[it->index_y]=1;
                    form[it->index_x][it->index_y]=-1;
                    linkx[it->index_x][Num]=it->index_y;
                    linky[it->index_y][Num]=it->index_x;
                    it=diagonal[i].erase(it);
                }
                else
                    ++it;
            }
        }
        for(int j=0;j<N;j++)
        {
            if(form_y[j]==0)
            {
                for(int i=0;i<N;i++)
                {
                    if(form_x[i]==0&&form_y[j]==0)
                    {
                        if(form[i][j]==-1)
                        {
                            linkx[i][Num]=-1;
                            linky[j][Num]=-1;
                        }
                        else
                        {
                            form_x[i]=1;
                            form_y[j]=1;
                            form[i][j]=-1;
                            linkx[i][Num]=j;
                            linky[j][Num]=i;
                        }

                    }
                }
            }
        }


    }

    for(int i=0;i<N;i++)
    {
        judge.assign(N, false);
        for(int j=0;j<numOfMEMS;j++)
        {
            if(linkx[i][j]>-1)
            {
                if(judge[linkx[i][j]]==1||i==linkx[i][j])
               {
                    linkx[i][j]=-1;
                    ++number;
               }
                else
                judge[linkx[i][j]]=1;
            }
            else
                ++number;

        }
    }
    return linkx;
}
double WangXiOld::getResult()
{
    result = 0;
    for(int i=0;i<N;i++)
    {
        for(int j=0;j<numOfMEMS;j++)
        {
            if(linkx[i][j]>-1)
            {
                result+=w1[i][linkx[i][j]];
            }
        }
    }
    return result;
}
int WangXiOld::getNumber()
{
    return number;
}


