ptrie
=====

PATRICIA trie implementation. 

Patricia tries are documented in Robert Sedgewick's Algorithms in C book. This implementation
differs from his in that I use separate node types for internal and external nodes, rather than 
using upwards pointers to denote external nodes as he does. This means that my implementation
uses more memory, but it also makes it easier to understand and maintain.

The key comparison code is based off of Danny Dulai's Patricia trie implementation in libishiboo
(http://ishiboo.com/~danny/Projects/libishiboo/).

References:

Algorithms in C by Robert Sedgewick
DJB Critbit http://cr.yp.to/critbit.html
TCP/IP Illustrated Volume 2 by Richard Stevens & Gary Wright


