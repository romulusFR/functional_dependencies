// Copyright Romuald THION

#include <iostream>
#include <algorithm>
#include <sstream>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
// see https://www.boost.org/doc/libs/1_70_0/doc/html/program_options.html
//     https://theboostcpplibraries.com/boost.program_options

#include "FunDep.hpp"

using namespace boost::program_options;
using namespace std;



enum operation_t {CLOSURE=0, MINIMIZE, ARMSTRONG, CLOSED, SCHEMA};
istream& operator>> (istream &in, operation_t &op)
{
    string token;
    in >> token;
    boost::to_upper (token);

    if (token == "CLOSURE")
      op = CLOSURE;
    else if (token == "MINIMIZE")
      op = MINIMIZE;
    else if (token == "ARMSTRONG")
      op = ARMSTRONG;
    else if (token == "CLOSED")
      op = CLOSED;
    else if (token == "SCHEMA")
      op = SCHEMA;
    else
      throw validation_error(validation_error::invalid_option_value);

    return in;
}


int main(int argc, const char *argv[]) {
  try {
    string input;
    vector<att_t> attrs;
    operation_t op;

    options_description desc{"Options"};
    desc.add_options()
      ("help,h", "Help screen")
      ("input,i", value<string>(&input), "Input file: a set of FD")
      ("attrs,a", value<vector<att_t>>(&attrs)->multitoken()->zero_tokens(), "A set of attributes")
      ("operation,o", value<operation_t>(&op)->default_value(SCHEMA), "Operation to compute");

    variables_map vm;
    command_line_parser parser{argc, argv};
    parser.options(desc).style(
      command_line_style::unix_style |
      command_line_style::case_insensitive);
    parsed_options parsed_options = parser.run();

    store(parsed_options, vm);
    notify(vm);

    if (vm.count("help") || argc == 1){
      cout << "Basic algorithms on sets of Functional Dependencies (FD)" << endl;
//      cout << "Computes: closure, minimization, and Armstrong's base" << endl;
      cout << "Attributes are strings, FD are written \"attr_A attr_B -> attr_C attr_A\""<<endl;
      cout << "Uses the algorithm by Gely et al. to enumerate closed sets [Gely10]" << endl;
      cout << endl;
      cout << desc << endl;
      cout << "Available operations:" << endl;
      cout << "\tSCHEMA: computes the set of attributes appearing in the input" << endl;
      cout << "\tCLOSURE: computes the closure of the given attributes" << endl;
      cout << "\tMINIMIZE: minimizes the given set of FDs" << endl;
      cout << "\tCLOSED: computes the sets of closed sets" << endl;
      cout << "\tARMSTRONG: generates an Armstrong's base in SQL" << endl;
      cout << endl;
      cout << "[Gely10] Gély, A.; Medina, R. & Nourine" << endl;
      cout << "         About the Enumeration Algorithms of Closed Sets" << endl;
      return 0;
    }

    set<att_t> attrs_set(attrs.cbegin(), attrs.cend());
    FunSet sigma(input);
   
    if (op == SCHEMA) {
      cout << sigma.schema() << endl  << endl; 
    }
    else if (op == CLOSURE) {
      
      cout << sigma.closure(attrs_set, FunSet::TRIV) << endl;
    }
    else if (op == MINIMIZE) {
      cout << sigma.minimized().reduced();
    }
    else if (op == CLOSED) {
      auto clf = sigma.closedSets();
      for_each(clf.cbegin(), clf.cend(), [] (const set<att_t>  &c){cout << c << endl; });
      cout << endl;
    }    
    else if (op == ARMSTRONG) {
      auto arm(sigma.armstrong());
      auto sch(sigma.schema());
      ostringstream insert_str; 
      insert_str << "INSERT INTO ARMSTRONG";
      show_container(insert_str, sch.cbegin(), sch.cend(), "(", ")");
      insert_str <<  " VALUES(";
      for (size_t i = 0; i < arm.size(); ++i){
        cout << insert_str.str() << arm[i][0];
        for (size_t j = 1; j < arm[i].size(); ++j){
          cout  << ", " << arm[i][j];
        }
        cout << ");" << endl;
      }
    }


  } catch (const error &ex) {
    std::cerr << ex.what() << '\n';
  }
}

