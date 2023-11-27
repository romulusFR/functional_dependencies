# A CLI tool for database functional dependencies

A CLI for basic algorithms on [functional dependencies](https://en.wikipedia.org/wiki/Functional_dependency).
Most of the algorithms are from A _Guided Tour of Relational Databases and Beyond_ by Levene, Mark and Loizou, George.

See the lessons here (in French) on [YouTube](https://www.youtube.com/channel/UCp6q0_DxUdNXhoc2Oue1w9g/videos).

```raw
Basic algorithms on sets of Functional Dependencies (FD)
Attributes are strings, FD are written "attr_A attr_B -> attr_C attr_A"
Uses the algorithm by Gely et al. to enumerate closed sets [Gely10]

Options:
  -h [ --help ]               Help screen
  -i [ --input ] arg          Input file: a set of FD
  -a [ --attrs ] arg          A set of attributes
  -o [ --operation ] arg (=4) Operation to compute

Available operations:
  SCHEMA: computes the set of attributes appearing in the input
  CLOSURE: computes the closure of the given attributes
  MINIMIZE: minimizes the given set of FDs
  CLOSED: computes the sets of closed sets
  ARMSTRONG: generates an Armstrong's base in SQL

[Gely10] Gély, A.; Medina, R. & Nourine
         About the Enumeration Algorithms of Closed Sets
```

You may need <https://www.boost.org/doc/libs/release/libs/program_options/>, to be installed using a command similar to `sudo apt install libboost-program-options1.74-dev` on Debian-based systems.
