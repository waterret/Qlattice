// Based on gwu kentucky/utils_construction.h 
// Yi-Bo. Yang
// Jun. 2013
// Gen Wang, Jan. 2021

#ifndef _gammas_h_
#define _gammas_h_

#pragma once

#include <cmath>
#include <complex>
#include <qlat/qlat.h>

#include "general_funs.h"

#define Imag Complexq(0.0,1.0)

class ga_M
{
public:
  ////std::complex<double> g[4];
  ////int ind[4];
  
  qlat::vector<Complexq > g;
  qlat::vector<int > ind;

  ga_M(){g.resize(4);ind.resize(4);for(int i=0;i<4;i++){g[i]=0.0;ind[i]=0;}};
     
  void print()
  {
    for(int i=0;i<4;i++)
    {
      for(int j=0;j<4;j++)
        if(ind[i]==j)printf("(%6.2e,%6.2e)",g[i].real(),g[i].imag());
        else printf("(%6.2e,%6.2e)",0.0,0.0);
      printf("\n");
    }
  }
     
  qacc ga_M operator*(const ga_M &src)
  {
    ga_M res;
    for(int i=0;i<4;i++)
    { 
       res.g[i]=g[i]*src.g[ind[i]];
       res.ind[i]=ind[src.ind[i]];
    }
    return res;
  }

  void check_sum(unsigned long &resp,unsigned long&resm){
    ///unsigned long resp = 0;
    resp = 0;
    for(int i=0;i<4;i++){
      resp += ((int(g[i].real())+5)*100+(int(g[i].imag())+5)*10+ind[i])*long(std::pow(1000,i));
    }
    ///unsigned long resm = 0;
    resm = 0;
    for(int i=0;i<4;i++){
      resm += ((-1*int(g[i].real())+5)*100+(-1*int(g[i].imag())+5)*10+ind[i])*long(std::pow(1000,i));
    }
  }

  int ga_sign()
  {
    Ftype res=0.0;
    ga_M tmp; 
    for(int i=0;i<4;i++)
    {
      tmp.g[i]+=g[i];
      tmp.g[ind[i]]-=qlat::qconj(g[i]);
    }
    for(int i=0;i<4;i++)
       res+=abs(tmp.g[i]);
    return (res<1e-5)? 1:-1;
  }
};

template<typename GAM>
void set_GAM(GAM &GA)
{
  ///std::cout << teml << " ";
  for(int i=0;i<4;i++)
  {
    for(int j=0;j<4;j++)
    {
      GA.ga_5i[j].g[i]=GA.ga_i[4].g[i]*GA.ga_i[j].g[GA.ga_i[4].ind[i]];
      GA.ga_5i[j].ind[i]=GA.ga_i[j].ind[GA.ga_i[4].ind[i]];
    }
    for(int j=0;j<3;j++)
    {
      GA.ga_4i[j].g[i]=GA.ga_i[3].g[i]*GA.ga_i[j].g[GA.ga_i[3].ind[i]];
      GA.ga_4i[j].ind[i]=GA.ga_i[j].ind[GA.ga_i[3].ind[i]];
    }
  }
  for(int i=0;i<4;i++)
    for(int j=0;j<3;j++)
    {
      GA.sig_i[j].g[i]=GA.ga_i[4].g[i]*GA.ga_4i[j].g[GA.ga_i[4].ind[i]];
      GA.sig_i[j].ind[i]=GA.ga_4i[j].ind[GA.ga_i[4].ind[i]];
    }

  ga_M *ga0[6]={&GA.unit,&GA.ga_i[0],&GA.ga_i[1],&GA.ga_i[2],&GA.ga_i[3],&GA.ga_i[4]};
  for(int i=0;i<6;i++)
  for(int j=0;j<6;j++)
  for(int k=0;k<4;k++)
  {
    GA.ga[i][j].g[k]=ga0[i]->g[k]*ga0[j]->g[ga0[i]->ind[k]];
    GA.ga[i][j].ind[k]=ga0[j]->ind[ga0[i]->ind[k]];
  }
};


class ga_matrices_PS
{
public:
  ga_M unit;
  ga_M ga_i[5];
  ga_M ga_5i[4];
  ga_M sig_i[3];
  ga_M ga_4i[3];//equal to sig_5i
  ga_M ga[6][6];
  ga_matrices_PS()
  {
    Ftype a[4]={-1,1,1,-1};
    
    for(int i=0;i<4;i++)
    {
      unit.g[i]=1.0;unit.ind[i]=i;
      ga_i[0].g[i]=Imag*Ftype(2*(i/2)-1.0);ga_i[0].ind[i]=3-i;    //gamma1
      ga_i[1].g[i]=a[i];              ga_i[1].ind[i]=3-i;    //gamma2
      ga_i[2].g[i]=Imag*a[i];         ga_i[2].ind[i]=(i+2)%4;//gamma3
      ga_i[3].g[i]=1.0-2*(i/2);       ga_i[3].ind[i]=i;      //gamma4
      ga_i[4].g[i]=-1.0;              ga_i[4].ind[i]=(i+2)%4;//gamma5
    }
    set_GAM(*this);
  }

};


class ga_matrices_milc
{
public:
  ga_M unit;
  ga_M ga_i[5];
  ga_M ga_5i[4];
  ga_M sig_i[3];
  ga_M ga_4i[3];//equal to sig_5i
  ga_M ga[6][6];
  ga_matrices_milc()
  {
    Ftype a[4]={-1,1,1,-1};
    
    for(int i=0;i<4;i++)
    {
      unit.g[i]=1.0;unit.ind[i]=i;
      ga_i[0].g[i]=Imag*Ftype(2*(i/2)-1.0);ga_i[0].ind[i]=3-i;    //gamma1
      ga_i[1].g[i]=-1.0*a[i];         ga_i[1].ind[i]=3-i;    //gamma2
      ga_i[2].g[i]=Imag*(a[i]);       ga_i[2].ind[i]=(i+2)%4;//gamma3
      ga_i[3].g[i]=-1.0;              ga_i[3].ind[i]=(i+2)%4;//gamma4
      ga_i[4].g[i]=1.0-2*(i/2);       ga_i[4].ind[i]=i;      //gamma5
    }
    set_GAM(*this);
  }

};

class ga_matrices_cps
{
public:
  ga_M unit;
  ga_M ga_i[5];
  ga_M ga_5i[4];
  ga_M sig_i[3];
  ga_M ga_4i[3];//equal to sig_5i
  ga_M ga[6][6];
  ////qlat::vector<ga_M > gL;
  ga_matrices_cps()
  {
    Ftype a[4]={-1,1,1,-1};
    for(int i=0;i<4;i++)
    {
      unit.g[i]=1.0;unit.ind[i]=i;
      ga_i[0].g[i]=Imag*Ftype(1.0-2*(i/2));ga_i[0].ind[i]=3-i;    //gamma1
      ga_i[1].g[i]=a[i];              ga_i[1].ind[i]=3-i;    //gamma2
      ga_i[2].g[i]=Imag*Ftype(-1.0*a[i]);  ga_i[2].ind[i]=(i+2)%4;//gamma3
      ga_i[3].g[i]=1.0;               ga_i[3].ind[i]=(i+2)%4;//gamma4
      ga_i[4].g[i]=1.0-2*(i/2);       ga_i[4].ind[i]=i;      //gamma5
    }
    set_GAM(*this);

    ////gL.resize(16);
    ////{int o=0;
    ////for(int i=0;i<6;i++){gL[o] = ga[0][i];o+=1;}
    ////for(int i=2;i<6;i++){gL[o] = ga[1][i];o+=1;}
    ////for(int i=3;i<6;i++){gL[o] = ga[2][i];o+=1;}
    ////for(int i=4;i<6;i++){gL[o] = ga[3][i];o+=1;}
    ////for(int i=5;i<6;i++){gL[o] = ga[4][i];o+=1;}}

  }

};

//static ga_matrices_PS    ga_PS;
//static ga_matrices_milc  ga_milc;
//static ga_matrices_cps   ga_cps;

//__device__ __constant__  Complexq gamma_com_cps[4*16];
//__device__ __constant__  int      gamma_int_cps[4*16];
//for(int iv=0;iv<16;iv++){
//  cudaMemcpyToSymbol(gamma_com_cps, &ga_cps.gL[iv].g[0]  , 4*sizeof(Complexq),iv*4*sizeof(Complexq), cudaMemcpyHostToDevice);
//  cudaMemcpyToSymbol(gamma_int_cps, &ga_cps.gL[iv].ind[0], 4*sizeof(int), iv*4*sizeof(int),cudaMemcpyHostToDevice);
//}


//__constant__  Complexq gamma_com[4*16];
//__constant__  int      gamma_int[4*16];

template<typename Ty>
qacc Ty reduce_gamma(const Ty *src,const ga_M &ga){
  Ty res = 0.0;
  for(int i=0;i<4;i++){
    Ty tem(ga.g[i].real(),ga.g[i].imag()); 
    res += tem*src[i*4 + ga.ind[i]];
  }
  return res;
}

//template<typename Ty>
//__device__ inline Ty reduce_gamma(const Ty *src,const int gi){
//  Ty res = 0.0;
//  int offg = gi*4;
//  for(int i=0;i<4;i++){
//    res += gamma_com_cps[offg + i]*src[i*4+gamma_int_cps[offg + i]];
//  }
//  return res;
//}

#endif