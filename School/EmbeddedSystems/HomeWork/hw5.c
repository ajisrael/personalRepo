union                              // Union for two sabertooth command bytes
{
    uint16_t u16;
    uint8_t  u8[2];
}   cmdPair;             

cmdPair.u16 = 0x5096;              // Init. pair of command bytes for robot
uint8_t i = 0;                     // Init. index of command byte to send

while(1)                           // Do forever:
{
    while (!(IFG2 & UCA0TXIFG)){}; // Wait for TX buff to be ready
    UCA0TXBUF = cmdPair.u8[i];     // Send command
    i ^= 0x01;                     // Index to next command
}
