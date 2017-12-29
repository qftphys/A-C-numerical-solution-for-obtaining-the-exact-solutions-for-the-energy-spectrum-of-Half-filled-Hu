#include <iostream>
#include <Eigen/Dense>
#include <fstream>
#include <cmath>
#include <clocale>
#include <chrono>
#include <complex>
#include <ctype.h>
#include "edlib.h"
#include "common_globals.h"

using namespace std;
using namespace std::chrono;
using namespace Eigen;

typedef complex <double> cd;
typedef std::vector<pair<int,double>> eivec;

const char  *ptr = NULL;
const wchar_t up[] = L"\u2191";
const wchar_t down[] = L"\u2193";
void vis(int u, int d)
{
  setlocale(LC_ALL, "");
  if(u==1 && d==1)     std::wcout << up << down;
  else if(u==1&& d==0) std::wcout << up;
  else if(u==0&& d==1) std::wcout << down;
  else                 std::wcout <<"O";
}

void vis_basis(int x, char newline)
{
  VectorXi v = inttobin(x);
  freopen(ptr, "w", stdout);
  for(int i=0; i<size; i++)
  {
    vis(v(i),v(i+size)); wcout << " ";
  }
  if(newline=='y') wcout << endl;
  else wcout << " ";
  freopen(ptr, "w", stdout);
}

class basis {
  int x; double spin;
public:
  basis(){x=spin=0;}
  basis(int b, double s){x=b; spin=s;}
  int get_x(){return x;}
  void get_arr(char c) {vis_basis(x,c);}
  float get_spin(){return spin;}
  void attach_spin(int s){spin = s;}
  void output(void){cout << inttobin(x).transpose() << "\t \t" << spin << endl;}
};

double filter(double x) {if(abs(x)<1e-4) return 0.0; else return x;}
void filter(std::vector<double>& v) {for(int i=0; i<v.size(); i++)  v[i]=filter(v[i]); }
void filter(VectorXd& v) {for(int i=0; i<v.size(); i++)  v(i)=filter(v(i));}
bool compare( pair<double,eivec> p1,  pair<double,eivec> p2) {return p1.first < p2.first;}
bool compare_eivec( pair <int, double> p1, pair <int, double> p2) {return abs(p1.second) > abs(p2.second);}

int annhilate(VectorXi v, int index, int sigma)
{
  if(sigma==-1) index+=v.size()/2;
  if(v(index)==1)
  {
    v(index)=0;
    return bintoint(v);
  }
  else
    return 0;
}

int annhilate(int x, int index, int sigma)
{
  VectorXi v=inttobin(x);
  if(sigma==-1) index+=v.size()/2;
  if(v(index)==1)
  {
    v(index)=0;
    return bintoint(v);
  }
  else
    return 0;
}


int create(VectorXi v, int index, int sigma)
{
  if(sigma==-1) index+=v.size()/2;
  if(v(index)==0) { v(index)=1; return bintoint(v);}
  else return 0;
}

int create(int x, int index, int sigma)
{
  VectorXi v= inttobin(x);
  if(sigma==-1) index+=v.size()/2;
  if(v(index)==0){ v(index)=1; return bintoint(v);}
  else return 0;
}

void vector_out(std::vector<basis> v) {for(auto it=v.begin(); it!=v.end(); it++) (*it).output();}

void select_spin(std::vector<basis> master, std::vector<basis>& v, double spin)
{ for(auto it=master.begin(); it!=master.end(); it++) if((*it).get_spin()==spin)  v.push_back(*it);}


void check_consistency(double t, double U)
{
  if(size!=2) return;
  cout << "-------------------------------------\n";
  cout << "Theoretical result for eigenvalues: \n";
  cout << U/2-sqrt(U*U/4+4*t*t) << endl << 0 << " (Triplet) " << endl << U << endl << U/2+sqrt(U*U/4+4*t*t)<< endl;
}

void check_tb_validity(void)
{
  cout << "The eigenvalues obtained by theoretical TB model: \n";
  for(int i=0; i<size; i++) cout << -2*t*cos(2*M_PI*i/double(size)) << ", ";
  cout << endl;
}

int main(int argc, char* argv[])
{
  cout << "Enter lattice size and U: ";
  cin >> size >> U;
  assert(size%2==0);

  long int i_min,i_max; i_min=i_max=0;

  for(int i=0; i<size; i++) i_min += pow(2,i);
  for(int i=size; i<2*size; i++) i_max += pow(2,i);

  vector<basis> half_filling;

  for(int i=i_min; i<=i_max; i++)
  {
    if(inttobin(i).sum()==size)
    {
      double spin=seminvert(inttobin(i)).sum();
      half_filling.push_back(basis(i,spin));
    }
  }


  int spin; int spin_limit = int(0.5*size);
  cout << "Enter total spin (m_s): "; cin >> spin;
  if(abs(spin)>spin_limit) {cout << "Maximum m_s= " << spin_limit << ". Exiting.\n"; exit(1);}

  std::vector<basis> v_spin;
  select_spin(half_filling, v_spin, spin);
  cout << "Spin: " << spin << " sector\n----------------\nsize=" << v_spin.size() << endl;

  milliseconds begin_ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
  MatrixXd Ht(v_spin.size(),v_spin.size());
  for(int a=0; a<Ht.rows(); a++)
    {
      for(int b=0; b<Ht.rows(); b++)
      {
        Ht(a,b)=0;
        for(int sigma=-1; sigma<=1; sigma+=2)
        {
          for(int i=0; i<size-1; i++)
          {
            int temp=annhilate(v_spin.at(b).get_x(),i+1,sigma);
            (v_spin.at(a).get_x()==create(temp,i,sigma))? Ht(a,b)+= -t: Ht(a,b)+=0;
          }

          for(int i=0; i<size-1; i++)
          {
            int temp=annhilate(v_spin.at(b).get_x(),i,sigma);
            (v_spin.at(a).get_x()==create(temp,i+1,sigma))? Ht(a,b)+= -t: Ht(a,b)+=0;
          }
        }
        cout << a << " " << b << "\r";
      }
    }
  milliseconds end_ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
  show_time(begin_ms,end_ms,"Ht construction");

  MatrixXd HU= MatrixXd::Zero(v_spin.size(),v_spin.size());
  for(int a=0; a<HU.rows(); a++)
  {
    VectorXi basis = inttobin(v_spin.at(a).get_x());
    for(int i=0; i<size; i++) HU(a,a) += basis(i)*basis(i+size);
    HU(a,a) *= U;
  }

  MatrixXd H=Ht+HU; VectorXd ith_spin_eivals; MatrixXd ith_eigenvectors;

  begin_ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
  diagonalize(H, ith_spin_eivals, ith_eigenvectors);
  end_ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
  show_time(begin_ms,end_ms,"Diagonalization");

  filter(ith_spin_eivals);

  vector < pair<double,eivec> > eigenspectrum;
  eivec ith_eigenvectors_listed;

  for(int i=0; i<ith_spin_eivals.size(); i++)
  {
    for(int j=0; j<ith_spin_eivals.size(); j++) ith_eigenvectors_listed.push_back(make_pair(v_spin[j].get_x(),ith_eigenvectors(j,i)));
    eigenspectrum.push_back(make_pair(ith_spin_eivals(i),ith_eigenvectors_listed));
    ith_eigenvectors_listed.clear();
  }

  sort(eigenspectrum.begin(),eigenspectrum.end(),compare);

  ofstream fout; string filename;
  filename = "data/debug_eivals_size"+to_string(size)+"_U"+to_string(int(U))/*+"_spin"+to_string(spin)*/+".txt"; fout.open(filename);
  for(auto it=eigenspectrum.begin(); it!=eigenspectrum.end(); it++) fout << (*it).first << endl;

  for(; ;)
  {
    cout << endl << "Enter an eigenvalue: ";
    int count=0; double d; cin >> d;
    for(auto it=eigenspectrum.begin(); it!=eigenspectrum.end(); it++)
    {
      if(abs((*it).first-d) < 0.01)
      {
        count++; cout << endl << "Eigenvector: " << count  << "\n=============================\n";
        sort((*it).second.begin(),(*it).second.end(),compare_eivec);
        for(int j=0; j<(*it).second.size(); j++)
         {
           // cout << ((*it).second)[j].first << " ";
           vis_basis(((*it).second)[j].first,'n');
           cout << filter(pow((((*it).second)[j].second),2)) << endl;
         }

      }
    }
    if(count==0) cout << "Not an eigenvalue!\n";
  }
  v_spin.clear(); eigenspectrum.clear();
  return 0;
}
