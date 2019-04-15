#if !defined(DEF_FUNDEP_HPP)
#define DEF_FUNDEP_HPP
#include <set>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <map>

using namespace std;

/** BEGIN : Generic printer for container ******************************************************/

template<class InputIterator> ostream& show_container(ostream& out, InputIterator first, InputIterator last, string beg="{", string end="}", string sep=", "){
  out << beg;
  if(first!=last)
    out << *first++;  
  for(; first != last ; ++first)
     out << sep << (*first);
  out << end;
  return out;
}

/** END : Generic printer for container ******************************************************/
typedef string att_t;
ostream& operator<<(ostream& out, const set<att_t>& s);

/*From [Gely10].
Definition 5. Let A and B be two sets of elements totally ordered. The set A
is lectically lower than the set B (denoted by A <lec B) if the smallest differing
element belongs to B.
More formally, there exists i ∈ B\A such that A ∩ {1, 2, . . . , i − 1} = B ∩
{1, 2, . . . , i − 1}

A < B :⇔ ∃i ∈ B − A : A ∩ {j ∈ M | j < i} = B ∩ {j ∈ M | j < i}.


For instance, the family of sets {a, ab, b, c, cd, ce} (given in the classical
lexicographic order) will be ordered {c, ce, cd, b, a, ab} in the lectic order.

[Gely10]
Gély, A.; Medina, R. & Nourine, L. Kwuida, L. & Sertkaya, B. (ed.)
About the Enumeration Algorithms of Closed Sets
Formal Concept Analysis, Springer Berlin / Heidelberg, 2010, 5986, 1-16
*/


struct lectic {
  //assume lhs and rhs use the same comparison object

  //implementation of lectic order <_lec, returns
  //  rhs.end() when !(lhs < rhs)
  //  the smallest different element of rhs when (lhs < rhs)
  set<att_t>::const_iterator diff(const set<att_t>& lhs, const set<att_t>& rhs) const {
    //http://www.cplusplus.com/reference/algorithm/mismatch/
    auto mypair = mismatch (lhs.begin(), lhs.end(), rhs.begin());    
    if (mypair.first == lhs.end())
      return mypair.second;
    else
      if (mypair.second != rhs.end() && lhs.key_comp()(*mypair.second, *mypair.first))
        return mypair.second;  
      else
        return rhs.end();    
  }

  inline bool operator() (const set<att_t>& lhs, const set<att_t>& rhs) const{
    return (diff(lhs, rhs) != rhs.end());    
  }

  inline bool diff(const set<att_t>& lhs, const set<att_t>& rhs, att_t c) const{
    auto it = diff(lhs, rhs);
    return (it != rhs.end() && *it==c);    
  }
};

struct FunDep{
//a functional dependency
  public:
    typedef set<att_t> data_t;
    
    FunDep(const data_t &s=data_t(), const data_t &d=data_t()): _src(s), _dst(d) {}

    inline bool operator < (const FunDep &rhs) const;
    inline bool operator == (const FunDep &rhs) const;

    void clear();
    bool empty() const;

    void swap (FunDep& rhs);

    friend std::ostream& operator<< (std::ostream& out, const FunDep& f);
    friend std::istream& operator>> (std::istream& in, FunDep& f);
    friend class FunSet;

  private:
    data_t _src;
    data_t _dst;

};
ostream& operator<< (ostream& out, const FunDep& f);
istream& operator>> (istream& in, FunDep& f);


struct FunSet{
//a "set" of functional dependencies
  public:
    typedef vector<FunDep> data_t;
    //typedef unordered_set< FunDep::data_t > hset_t;

    FunSet() : _set(data_t()), _idx(multimap<att_t, size_t>()), _count(vector<size_t>()) {}
    FunSet(const string &file);

    enum method_t {TRIV=0, SKIP, LIN};
    FunDep::data_t closure(const FunDep::data_t &x, method_t method = SKIP) const;
    bool models(const FunDep& f, method_t method = SKIP) const;

    FunSet minimized() const;
    FunSet reduced() const;

    FunDep::data_t schema() const;

    vector<set<att_t>> closedSets() const;
    vector<vector<size_t>> armstrong() const;

    friend std::ostream& operator<< (std::ostream& out, const FunSet& s);
    
  private:
    data_t  _set;                                           // the vector of fds
    multimap<att_t, size_t> _idx;                           // index structure
    vector<size_t> _count;                                  // for linear closure                                               

    void _buildIndex();

    FunDep::data_t closureTriv(const FunDep::data_t &x, size_t last) const;
    FunDep::data_t closureSkip(const FunDep::data_t &x, size_t last) const;
    FunDep::data_t closureLin(const FunDep::data_t &x) const;
    bool models(const FunDep& f, size_t last, method_t method = SKIP) const;

    set<att_t> nextClosure(const set<att_t>& j, const set<att_t>& a) const;


};
ostream& operator<< (ostream& out, const FunSet& s);

/****** lectic order example **************************************************/
//  cout << "lectic(s,s) = " << lectic()(s,s) << endl;
//  cout << "lectic(s,d) = " << lectic()(s,d) << endl;
//  cout << "lectic(d,s) = " << lectic()(d,s) << endl;
//  cout << "lectic(d,d) = " << lectic()(s,s) << endl;

//  vector<set<att_t>> t({ set<att_t>{'A'},set<att_t>{'A', 'B'}, set<att_t>{'B'}, set<att_t>{'C'}, set<att_t>{'C', 'D'}, set<att_t>{'C', 'E'} });
//  cout << "t = [" ;
//  for_each(t.cbegin(),t.cend(), [] (const set<att_t> &c){cout << c << ' '; });
//  cout << "]" << endl ;
//  sort(t.begin(), t.end());
//  cout << "t (lexico) = [" ;
//  for_each(t.cbegin(),t.cend(), [] (const set<att_t> &c){cout << c << ' '; });
//  cout << "]" << endl ;
//  sort(t.begin(), t.end(), lectic());
//  cout << "t (lectic) = [" ;
//  for_each(t.cbegin(),t.cend(), [] (const set<att_t> &c){cout << c << ' '; });
//  cout << "]" << endl ;

//  vector<set<att_t>> t({ set<att_t>{'A'},set<att_t>{'A', 'B'}, set<att_t>{'B'}, set<att_t>{'C'}, set<att_t>{'C', 'D'}, set<att_t>{'C', 'E'} });
//  sort(t.begin(), t.end(), lectic());

//  vector<set<att_t>> clost({ set<att_t>{'A', 'B', 'C', 'D', 'E'}, set<att_t>{'B'}, set<att_t>{'E'},set<att_t>{'B', 'E'}, set<att_t>{'D', 'E'}, set<att_t>{'B', 'C', 'D', 'E'}, set<att_t>{'C', 'D', 'E'}, set<att_t>{'B', 'D', 'E'} });
//  sort(clost.begin(), clost.end(), lectic());
//  #E #DE #CDE #B #BE #BDE #BCDE #ABCDE

//  cout << "lectic(s,s) = " << lectic()(set<att_t> {'A', 'E'}, set<att_t> {'A', 'D'}) << endl;
//  cout << "lectic(s,s) = " << lectic().diff(set<att_t> {'A', 'E'}, set<att_t> {'A', 'D'}, 'D') << endl;
//  cout << "lectic(s,s) = " << lectic().diff(set<att_t> {'A', 'E'}, set<att_t> {'A', 'D'}, 'E') << endl;
//  cout << "lectic(s,s) = " << lectic().diff(set<att_t> {'A', 'D'}, set<att_t> {'A', 'E'}, 'D') << endl;
//  cout << "lectic(s,s) = " << lectic().diff(set<att_t> {'A', 'D'}, set<att_t> {'A', 'E'}, 'E') << endl;

#endif //DEF_FUNDEP_HPP
