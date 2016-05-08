@echo off
avrdude -q -p m168 -c stk200 -U lfuse:r:lfuse.bin:h > /dev/null
avrdude -q -p m168 -c stk200 -U hfuse:r:hfuse.bin:h > /dev/null
avrdude -q -p m168 -c stk200 -U efuse:r:efuse.bin:h > /dev/null
avrdude -q -p m168 -c stk200 -U lock:r:lock.bin:h > /dev/null
echo ...
echo lfuse: 
cat lfuse.bin
echo hfuse: 
cat hfuse.bin
echo efuse: 
cat efuse.bin
echo lock: 
cat lock.bin
