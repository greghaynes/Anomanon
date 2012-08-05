Anomanon
========
Simple network filter with aberrant behavior detection.

License
-------
All code and software is licensed under the BSD 3-clause license.
See LICENSE for more information.

Building
--------
<pre>
cmake .
make
</pre>

Using
-----
<pre>
./src/anomanon -i interface -a alpha -s season [-b beta] [-q filter]
</pre>

Aberrant Behavior Detection
---------------------------
rrdtool's Holt-Winters forecasting algorithm is used for anomalie detection.
More information can be found at:
http://oss.oetiker.ch/rrdtool/doc/rrdcreate.en.html

