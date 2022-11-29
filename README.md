Beam-ACO for the Travelling Salesman Problem with Time Windows
==========================================================
                                       
[Christian Blum](christian.blum@csic.es) and [Manuel Lopez-Ibanez](https://lopez-ibanez.eu)


Introduction
------------

This is the Beam-ACO software used in our papers

 * Manuel López-Ibáñez and Christian Blum. **[Beam-ACO for the travelling salesman
   problem with time windows](http://doi.org/10.1016/j.cor.2009.11.015)**. _Computers & Operations Research_, 37(9):1570-1583,
   2010. doi: [10.1016/j.cor.2009.11.015](http://doi.org/10.1016/j.cor.2009.11.015) ([bibtex](https://lopez-ibanez.eu/LopezIbanez_bib.html#LopBlu2010cor))

 * Manuel López-Ibáñez, Christian Blum, Jeffrey W. Ohlmann, and Barrett W. Thomas. **[The Travelling Salesman Problem with Time Windows: Adapting Algorithms from Travel-time to Makespan Optimization](https://doi.org/10.1016/j.asoc.2013.05.009)**. _Applied Soft Computing_, 13(9):3806–3815, 2013. doi: [10.1016/j.asoc.2013.05.009](https://doi.org/10.1016/j.asoc.2013.05.009)
  ([bibtext](https://lopez-ibanez.eu/LopezIbanez_bib.html#LopBlu2013asoc)) 
  
This is not the exact version of the software used in those papers, since
further modifications have been done along the way. Therefore, if something is
not working or you cannot reproduce the results in those papers, please [report a bug](https://github.com/MLopez-Ibanez/beam-aco-tsptw/issues)
 and we will find out what has changed with respect to older
versions.

Please, if you use this software in any publication, give proper credit to us
by citing the above papers. See the section "License" below for details.


Building
--------

The programming language is C++, and the software has been tested on
GNU/Linux using GCC >= 4.8.

Change the current directory to `src` and invoke `make`. For testing
use:
```sh
make DEBUG=1
```
For final results, use
```sh
make DEBUG=0
```
The objective type can be selected by adding either `OBJECTIVE=TOURCOST` or
`OBJECTIVE=MAKESPAN` to the call to `make`. By default, the code is compiled
for integer values. If your data is real-valued, you need to recompile with
`NUMBER_TYPE=DOUBLE`. For example,
```sh
make DEBUG=0 OBJECTIVE=MAKESPAN NUMBER_TYPE=DOUBLE
```
See the parameters available using:
```sh
./beamaco_tsptw --help
```

You can find test instances at https://lopez-ibanez.eu/tsptw-instances


License
-------

This software is Copyright (C) 2008 - 2015

Manuel Lopez-Ibanez and Christian Blum

This program is free software (software libre); you can redistribute
it and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

The file LICENSE contains a copy of the GNU General Public License; if
not, you can obtain a copy of the GNU General Public License at
http://www.gnu.org/copyleft/gpl.html

This software includes code from various sources:

  * The GNU Scientific Library (GSL) is Copyright (C) The GSL Team,
    under the terms of the GPL.

IMPORTANT NOTE: Please be aware that the fact that this program is
released as Free Software does not excuse you from scientific
propriety, which obligates you to give appropriate credit! If you
write a scientific paper describing research that made substantive use
of this program, it is your obligation as a scientist to (a) mention
the fashion in which this software was used in the Methods section;
(b) mention the algorithm in the References section. The appropriate
citation is:

   Manuel López-Ibáñez and Christian Blum. Beam-ACO for the travelling salesman
   problem with time windows. Computers & Operations Research, 37(9):1570-1583,
   2010. doi: 10.1016/j.cor.2009.11.015

   Manuel López-Ibáñez, Christian Blum, Jeffrey W. Ohlmann, and Barrett
   W. Thomas. The Travelling Salesman Problem with Time Windows: Adapting
   Algorithms from Travel-time to Makespan Optimization. Applied Soft
   Computing, 13(9):3806–3815, 2013. doi: 10.1016/j.asoc.2013.05.009
