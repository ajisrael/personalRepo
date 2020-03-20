while (UCB0STAT | UCBBUSY) {};  // Wait for idle bus
UCB0I2CSA = devAddr;            // Set the slave address
UCB0CTL1 |= UCT0STT;            // Send the start bit and the address
for (uint8_t i = 0; i < N; i++) // Begin looping through messages
{   
    while (!(IFG2 | UCB0TXIFG)) {}; // Wait for tx buf to clear
    UCB0TXBUF = msgBuff[i];         // Xmit the current message
}                               // When all messages sent...
UCB0CTL1 |= UCT0STP;            // Send the stop bit