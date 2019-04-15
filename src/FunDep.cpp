#include "FunDep.hpp"
#include <iostream>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <fstream>

const string ARROW_STR="->";


/** BEGIN : FunSet::set<att_t> ******************************************************/

ostream& operator<<(ostream& out, const set<att_t>& s){
  //for_each(s.cbegin(),s.cend(), [&out] (const att_t &c){out << c; });
  show_container(out, s.begin(), s.end(), "", "");
  return out;
}

/** END : FunSet::set<att_t> ******************************************************/


/** BEGIN : FunDep:: readers and showers ******************************************************/

ostream& operator<<(ostream& out, const FunDep& f){
//  cerr << "ostream& operator<<(ostream& out, const FunDep& f)"<< endl;
  out << f._src << " " << ARROW_STR << " "<< f._dst;
  return out;
}

istream& operator>> (istream& in, FunDep& f){
  string str;
  getline(in,str);
  istringstream isstr(str);
  bool src(true);

  while(isstr.good()){
    isstr >> str;
    if(str.empty() || str[0]=='#')
      break;
    if(str==ARROW_STR)
      src=false;
    else{
      if(src)
        f._src.insert(str);
        //f._src.insert(str.begin(),str.end());
      else
        f._dst.insert(str);
        //f._dst.insert(str.begin(),str.end());
    }
  }
  return in;
}

/** END : FunDep:: readers and showers ******************************************************/

/** BEGIN : FunDep:: operators ******************************************************/
void FunDep::clear(){
  _src.clear();
  _dst.clear();
}

bool FunDep::empty() const{
  return (_src.empty() && _dst.empty());
}

inline bool FunDep::operator < (const FunDep &rhs) const {
  return (this->_src < rhs._src || (this->_src == rhs._src && this->_dst < rhs._dst))  ;
}

inline bool FunDep::operator == (const FunDep &rhs) const {
  return (this->_src == rhs._src && this->_dst == rhs._dst)  ;
}

FunDep::data_t FunSet::schema() const{
  FunDep::data_t r;
  for(auto it = _set.cbegin(); it!=_set.cend(); ++it){
    r.insert(it->_src.cbegin(),it->_src.cend());
    r.insert(it->_dst.cbegin(),it->_dst.cend());
  }
  return r;
}

/** END : FunDep:: operators ******************************************************/


/** BEGIN : FunDep:: swap ******************************************************/
void FunDep::swap (FunDep& rhs){
//TODO : how to check that std::swap is "efficient" for std::vector
//       it's OK : /usr/include/c++/4.5/bits/stl_vector.h LINE 924
    this->_src.swap(rhs._src);
    this->_dst.swap(rhs._dst);
}

/** END : FunDep:: swap ******************************************************/


/** BEGIN : FunSet:: readers and showers ******************************************************/

FunSet::FunSet(const string &file): _set(FunSet::data_t()), _idx(multimap<att_t, size_t>()), _count(vector<size_t>()) {
  ifstream myfile (file.c_str(), ios::in); 
  FunDep f;
  while(myfile.good()){
    f.clear();
    myfile >> f;
    if(!f.empty()) // && find(_set.begin(), _set.end(), f)==_set.end()
    //TODO : sublinear membership test
      _set.push_back(f);
  }
  _buildIndex();
}

ostream& operator<< (ostream& out, const FunSet& s){
  for_each(s._set.cbegin(),s._set.cend(), [&out] (const FunDep &f){out << f << endl; });
  return out;
}
/** END : FunSet:: readers and showers ******************************************************/


/** BEGIN : FunSet:: closure ******************************************************/

void FunSet::_buildIndex(){
  _count.clear();
  _count.resize(_set.size());
  _idx.clear();
  for(size_t i = 0; i < _set.size(); ++i){
    for(auto it = _set[i]._src.cbegin(); it != _set[i]._src.cend(); ++it){
      _idx.insert(pair<att_t, size_t>(*it,i));
      ++_count[i];
    }
  }

//check index structures
//  for(auto it=idx.begin(); it!=idx.end(); ++it){
//    cerr << (*it).first << " => " << (*it).second << endl;
//  }  
//  for(size_t i=0; i<count.size(); ++i){
//    cerr << "count[" << i << "]"<< " => " << count[i] << endl;
//  }
}

FunDep::data_t FunSet::closureTriv(const FunDep::data_t &x, size_t last) const{
//TRIVIAL
//compute the closure of a set of attributes using only the fds upto the 'last' given one
  FunDep::data_t r(x);
  size_t i;
  do{
    i=r.size();
    for(size_t it=0 ; it < last ; ++it){
      if(includes(r.begin(),r.end(),_set[it]._src.begin(),_set[it]._src.end()))
        r.insert(_set[it]._dst.begin(),_set[it]._dst.end());      
    }    
  } while(i!=r.size());
  return r;
}


FunDep::data_t FunSet::closureSkip(const FunDep::data_t &x, size_t last) const{
//SKIP
  size_t i(0);
  FunDep::data_t r(x);
  FunSet::data_t l(_set.begin(), _set.begin() + last);
  do{
    i = r.size();
    for(size_t it=last ; it > 0  ; --it){
      if(includes(r.begin(),r.end(),l[it-1]._src.begin(),l[it-1]._src.end())){
        r.insert(l[it-1]._dst.begin(),l[it-1]._dst.end());
        swap(l[it-1],l[last-1]);
        --last;
      }
    }    
  } while(i!=r.size());
  return r;
}

FunDep::data_t FunSet::closureLin(const FunDep::data_t &x) const{
// WARNING MUST ENSURE CONSISTENCY BETWEEN _set and (_idx, _count) !!!
// WARNING CANNOT BE USED WHILE _set IS BEING MODIFIED !
  auto count = _count;
  FunDep::data_t r(x);
  list<att_t> rnew(x.begin(), x.end());
  att_t att;
  while(rnew.size()>0){
    att = rnew.back();
    rnew.pop_back();
//    cerr << "\tclosureLin::pop_back " << att << endl;
    auto ret = _idx.equal_range(att);
    for (auto it=ret.first; it!=ret.second; ++it){
      --count[(*it).second];
      if(count[(*it).second]==0){
//        cerr << "\tclosureLin:: _set["<< (*it).second <<"] = " << _set[(*it).second] << " is done" << endl;
        for(auto d = _set[(*it).second]._dst.cbegin(); d != _set[(*it).second]._dst.cend(); ++d){
          if(r.find(*d)==r.end()){
            r.insert(*d);
            rnew.push_back(*d);
//            cerr << "\tclosureLin:: added "<< *d << endl;
          }
        }
      }        
    }      
  }
  return r;
}

bool FunSet::models(const FunDep& f, size_t last, method_t method) const{
  FunDep::data_t r ;
  switch(method){
    case TRIV:
      r = closureTriv(f._src, last);
      break;
    case SKIP:
      r = closureSkip(f._src, last);
      break;    
    case LIN:
      r = closureLin(f._src);
      break;
  }
  return includes(r.begin(),r.end(),f._dst.begin(),f._dst.end());
}

//public member
FunDep::data_t FunSet::closure(const FunDep::data_t &x, method_t method) const{
  switch(method){
    case TRIV:
      return closureTriv(x,_set.size());
    case SKIP:
      return closureSkip(x,_set.size());      
    case LIN:
      return closureLin(x);
  }
  return FunDep::data_t();
}

//public member
bool FunSet::models(const FunDep& f, method_t method) const{
  return models(f, _set.size(), method);
}

/** END : FunSet:: closure ******************************************************/


/** BEGIN : FunSet:: minimization ******************************************************/
FunSet FunSet::minimized() const{
//MUST NOT USE LINEAR CLOSURE !!! (swap between fd)
  FunSet r;
  FunDep f;
  for(auto it=_set.cbegin();it!=_set.cend();++it){
    f.clear();
    f._src=it->_src;
    f._dst=closure(f._src);
    r._set.push_back(f);
  }
  //last stands for the number of dependencies kept
  size_t last = r._set.size();
  //scan dependencies from end to beginning of _set
  for(size_t i = r._set.size() ; i >0 ;--i){
    //swap current one and last kept one
    swap(r._set[i-1], r._set[last-1]);
    //check if dependencies BUT THE LAST implies the last one 'r._set[last-1]'
    //if so, 'r._set[last-1]' is redundant
    bool t = r.models(r._set[last-1], last-1, SKIP);
    //'r._set[last-1]' wont be used anymore for implication tests as last is decremented
    if(t)
      --last;
  }
  r._set.resize(last);
  r._buildIndex();
  return r;
}


/** END : FunSet:: minimization ******************************************************/



/** BEGIN : FunSet:: rhs and lhs attribute reduction of dfs ******************************************************/

FunSet FunSet::reduced() const{
//MUST NOT USE LINEAR CLOSURE !!! (modification of index)

  //WARNING : thus algorithm is dependent on attributes order !!! here, order is fixed
  //TODO : check if test (in algo 1 from TD6) is Min |= (W-A) -> X or Min |= (W-A) -> Y
  //       >> should be the same (by transitivity) !
  //TODO : faire un truc un peu mieux que des copies brutales, en particulier pour rhs...

  FunSet r=*this;
  FunDep f,b;
  FunDep::data_t x,a;
  
  //left hand side reduction
  for(size_t i=0; i < r._set.size() ; ++i){    
    a = r._set[i]._src;                               //backup to keep iterator valid
    x = r._set[i]._src;
    f._dst = r._set[i]._dst;
    for(auto it = a.cbegin(); it != a.cend(); ++it){
      x.erase(*it);
      f._src=x;
      if (!r.models(f, SKIP))
        x.insert(*it);
    }
    f._src=x;
    r._set[i]=f;  
  }
  
  //right hand side reduction
  for(size_t i=0; i < r._set.size() ; ++i){    
    a = r._set[i]._dst;                               //backup to keep iterator valid
    x = r._set[i]._dst;
    f._src = r._set[i]._src;
    b = r._set[i];
    for(auto it = a.cbegin(); it != a.cend(); ++it){
      x.erase(*it);
      f._dst=x;
      r._set[i]=f;
      if (!r.models(b, SKIP)){
        x.insert(*it);
      }
    }
    f._dst=x;
    r._set[i]=f;  
  }
  r._buildIndex();
  return r;
}
/** BEGIN : FunSet:: rhs and lhs attribute reduction of dfs ******************************************************/



/** BEGIN : FunSet:: closed sets ******************************************************/
vector<set<att_t>> FunSet::closedSets() const{
//  result is sorted according to lectic order by construction
  vector<set<att_t>> r;
  set<att_t> sch(schema());
  set<att_t> cl;
  do{
    cl = nextClosure(sch, cl);
    r.push_back(cl);
//    cerr << "closedSets() =" << cl << endl;
  } while(cl != sch);
  return r;
}

set<att_t> FunSet::nextClosure(const set<att_t>& j, const set<att_t>& a) const{
  //WARNING : PUTAIN DE TYPO sur 'X' et 'ascending order' DANS L'ALGO 1 DE [Gely10] !!!!
//  cerr << "\tnextClosure(" << j << "," << a << ")" << endl;
  set<att_t> k,b,cl;
  set_difference(j.begin(), j.end(), a.begin(), a.end(), std::inserter(k, k.end()));
//  cerr << "\tnextClosure k =" << k << endl;
  for(auto itk = k.rbegin(); itk != k.rend(); ++itk){
//    cerr << "\tnextClosure *itk =" << *itk << endl;
    auto ita = find_if(a.begin(), a.end(), [&] (const att_t& c){return (c >= *itk); });
    b = set<att_t>(a.begin(), ita);
    b.insert(*itk); 
    cl = closure(b, LIN);
    if (lectic().diff(a, cl, *itk))
      return cl;
  }
  return j;
}

vector<vector<size_t>> FunSet::armstrong() const{
  vector<set<att_t>> s(closedSets());
  vector<vector<size_t>> r(s.size());
  set<att_t> sch(schema()); 
  size_t j(0);

  for(size_t i = s.size(); i>0; --i){
    r[s.size()-i].resize(sch.size());
    j = 0;
    for(set<att_t>::const_iterator it = sch.cbegin(); it != sch.cend();++it){
      if (find(s[i-1].begin(), s[i-1].end(), *it)== s[i-1].end())
        r[s.size()-i][j]=s.size()-i;
      else
        r[s.size()-i][j]=0;
      ++j;
    }
  }
  return r;
}

/** END : FunSet:: closed sets ******************************************************/


