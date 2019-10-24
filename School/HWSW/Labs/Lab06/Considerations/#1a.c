tempReg = __builtin_rdctl(0) | 0x1;  // Read ctl0 and set pio bit
__builtin_wrctl(0, tempReg)          // Write tempReg to clt0