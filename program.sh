#! /bin/bash

avrdude -p m168 -P /dev/parport0 -c stk200 -U flash:w:demo.hex
