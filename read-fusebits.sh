@echo off
avrdude -q -p m168 -c stk200 -U lfuse:r:lfuse.bin:b > /dev/null
avrdude -q -p m168 -c stk200 -U hfuse:r:hfuse.bin:b > /dev/null
avrdude -q -p m168 -c stk200 -U efuse:r:efuse.bin:b > /dev/null
avrdude -q -p m168 -c stk200 -U lock:r:lock.bin:b > /dev/null
echo ...
echo lfuse: 
cat lfuse.bin
echo hfuse: 
cat hfuse.bin
echo efuse: 
cat efuse.bin
echo lock: 
cat lock.bin
