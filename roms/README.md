ROMs 
====
These are mostly from [zimmers.net](http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/pet/index.html):
- basic2_c000.h (basic-2-c000.901465-01.bin)
- basic2_d000.h (basic-2-d000.901465-02.bin)
- edit2.h       (edit-2-n.901447-24.bin)
- kernal2.h     (kernal-2.901465-03.bin)
- basic4_b000.h (basic-4-b000.901465-23.bin)
- basic4_c000.h (basic-4-c000.901465-20.bin)
- basic4_d000.h (basic-4-d000.901465-21.bin)
- edit4.h       (edit-4-n.901447-29.bin)
- kernal4.h     (kernal-4.901465-22.bin)
- characters2.h (characters-2.901447-10.bin)

See also [here](http://www.6502.org/users/sjgray/computer/cbmchr/cbmchr.html).

See also [this useful document](http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/pet/README).

Header files are created from binary images using [makerom](https://github.com/jscrane/emul8/tree/master/util):

	$ makerom -bp basic-2-c000.901465-01.bin basic2_c000 > basic2_c000.h

Character-set roms require further editing after conversion, e.g.:

	$ makerom -b -p images/characters-2.901447-10.bin charset | sed -e 's/^\t0/\t{ 0/' -e 's/, $/, },/' -e 's/\[\]/\[256\]\[8\]/' > characters2.h
