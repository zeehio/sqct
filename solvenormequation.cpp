#include "solvenormequation.h"
#include "appr/normsolver.h"
#include <cassert>

using namespace std;

norm_equation_solution solve_norm_equation(const zs2type &rhs)
{
  norm_equation_solution res;
  res.exists = false;

  if( ! (rhs.non_negative() && rhs.g_conjugate().non_negative() ) )
    return res;

  auto Fz = factorize(rhs.norm());
  res.exists = is_solvable(Fz);

  const auto& ns = normSolver::instance();

  if( res.exists )
  {
    auto Fzs2 = factorize(rhs,Fz);
    res.exists = Fzs2.solvable;
    if( res.exists )
    {
      res.ramified.push_back(make_pair(zwt(1,1,0,0),Fzs2.ramified_prime_power));
      for( const auto& a : Fzs2.prime_factors )
      {
        zwt ans;
        if( a.first[1] == 0 ) // rational prime
        {
          bool r = ns.solve(a.first,ans);
          assert(r);
          res.split.push_back(make_pair(ans,a.second));
        }
        else
        {
          auto nrm = a.first.norm();
          auto md = nrm % 8;
          if( md == 1 )
          {
            bool r = ns.solve(a.first,ans);
            assert(r);
            res.split.push_back(make_pair(ans,a.second));
          }
          else
          {
            assert(md == 7);
            assert(a.second % 2 == 0);
            res.inert.push_back(make_pair(a.first,a.second/2));
          }
        }
      }
      assert(Fzs2.unit_power % 2 == 0);
      assert(Fzs2.sign == 1);
      res.unit_power = Fzs2.unit_power / 2;
    }
  }



  return res;
}

// returns all solutions to the norm equation up to a power of \w
std::vector<zwt> all_solutions(const norm_equation_solution& sln)
{
  std::vector<zwt> res;
  zwt common_factor(1,0,0,0);

  for( const auto& a : sln.inert )
  {
    for( long i = 0; i < a.second; ++i )
      common_factor = common_factor * a.first;
  }

  for( const auto& a : sln.ramified )
  {
    for( long i = 0; i < a.second; ++i )
      common_factor = common_factor * a.first;
  }

  std::vector< std::vector<zwt> > split_factors(sln.split.size());
  for( size_t k = 0; k < sln.split.size(); ++k )
  {
    const auto& a = sln.split[k];
    std::vector<zwt> powers;
    long r = a.second;
    zwt pow(1,0,0,0);
    for( long i = 0; i <= r; ++i )
    {
      powers.push_back(pow);
      pow = pow * a.first;
    }
    for( long i = 0;  i <= r; ++i)
      split_factors[k].push_back(powers[i] * powers[r-i].conjugate());
  }

  long tuples = 1;
  for( size_t k = 0; k < split_factors.size(); ++k )
    tuples *= split_factors[k].size();

  vector<long> current(split_factors.size(),0);
  for ( size_t k = 0; k < tuples; ++k )
  {
    zwt v = common_factor;
    for( size_t j = 0; j < split_factors.size(); ++j )
      v = v * split_factors[j][current[j]];
    res.push_back(v);

    // iterate to the next tuple
    for( int i = 0; i < split_factors.size(); ++i )
    {
      current[i]++;
      if( current[i] < split_factors[i].size() )
        break;
      else
        current[i] = 0;
    }
  }
  return res;
}
