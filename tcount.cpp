#include "tcount.h"
#include "factorzs2.h"
#include "solvenormequation.h"
#include "matrix2x2.h"

#include "output.h"

#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

std::pair<int,long> min_w_t_count( const zwt& x, const zwt& y, long m, long k )
{
  zwt ycm = -y.conjugate();
  matrix2x2<mpz_class> mtr(x, ycm * zwt::omega(k),
                           y, x.conjugate() * zwt::omega(k), m );
  assert(mtr.is_unitary());

  int min_i = 0;
  long min_tc = mtr.t();

  for( int i = 1; i < 2; ++i )
  {
    mtr.d[1][0] = y * zwt::omega(i);
    mtr.d[0][1] = ycm * zwt::omega(8-i+k);
    int t = mtr.t();
    if( t < min_tc )
    {
      min_tc = t;
      min_i = i;
    }
  }
  return std::make_pair(min_i,min_tc);
}

min_unitaries min_t_count(const zwt &x, long m, int k)
{
  min_unitaries res;
  res.min_t_count = -1;
  res.x = x;
  res.m = m;
  res.k = k;

  std::vector<zwt> y[4];

  long n0 = 2*m - x.abs2().gde();
//  if( n0 < 4 )
//  {
//    cerr << x << "," << m << "," << k << endl;
//    throw std::logic_error(":unsupported regime");
//  }

  zs2type pow2 = zs2type(ztype(1) << m,0);
  zs2type rhs(pow2 - x.abs2());

  auto sln = solve_norm_equation(rhs);
  if( sln.exists )
  {
    auto all_slns = all_solutions(sln);
    for( const auto& a : all_slns )
    {
      auto tc = min_w_t_count(x,a,m,k);
      int d = tc.second - n0 + 2;
      assert( d == 0 || d == 1 );
      y[d].push_back(a * zwt::omega(tc.first));
    }

    for( int d = 0; d < 4 ;++d )
    {
      if( ! y[d].empty() )
      {
        res.min_t_count = n0 - 2 + d;
        swap( y[d], res.y );
        res.to_canonical_form();
        return res;
      }
    }

  }

  return res;
}


void min_unitaries::to_canonical_form()
{
  if( x < -x )
    x = -x;

  for( size_t i = 0; i < y.size(); ++i )
  {
    y[i] = y[i].i_canonical();
  }

  {
    vector<zwt> tmp;
    tmp.resize(y.size());
    sort(y.begin(),y.end());
    auto r = unique_copy(y.begin(),y.end(),tmp.begin());
    tmp.resize(distance(tmp.begin(),r));
    swap(tmp,y);
  }

  for( size_t i = 0; i < y.size(); ++i )
  {

    matrix2x2<mpz_class> mm1(x   ,-y[i].conjugate() * zwt::omega(k),
                      y[i], x.conjugate() * zwt::omega(k)    , m);

    matrix2x2<mpz_class> mm2(x                    ,-y[i].conjugate() * zwt::omega(k+7),
                       y[i] * zwt::omega(1) , x.conjugate() * zwt::omega(k)     , m);

    if( mm1.t() == mm2.t() )
    {
      y[i] = y[i].w_canonical();
    }
  }

  {
    vector<zwt> tmp;
    tmp.resize(y.size());
    sort(y.begin(),y.end());
    auto r = unique_copy(y.begin(),y.end(),tmp.begin());
    tmp.resize(distance(tmp.begin(),r));
    swap(tmp,y);
  }

}

bool min_unitaries::operator ==(const min_unitaries &rhs) const
{
  if( y.size() == 0 && rhs.y.size() == 0 ) //empty results are equal
    return true;

  if( rhs.k != k ) return false;
  if( rhs.m != m ) return false;
  if( rhs.min_t_count != min_t_count ) return false;
  if( rhs.x != x ) return false;
  if( rhs.y.size() != y.size() ) return false;

  for( size_t s = 0; s < y.size(); ++s )
  {
    if( y[s] != rhs.y[s] )
      return false;
  }

  return true;
}


ostream &operator <<(ostream &out, const min_unitaries &mu)
{
  out << "{" << mu.k << "," << mu.m <<
          "," << mu.x <<
          "," <<  mu.min_t_count;

  out << ",{";
  if(mu.y.size() > 0 )
    cout << mu.y[0];

  for( size_t i = 1; i < mu.y.size(); ++i )
  {
    out << "," << mu.y[i];
  }
  out << "}}";
  return out;
}
