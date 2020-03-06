while (!(IFG2 & UCB0TXIFG)) {};         // Wait till it is safe to transmit
P3OUT &= ~0x20;                         // Assert SS* low
for (i = 0; i < N; i++)                 // Loop through N bytes of msgBuf
{
    UCB0TXBUF = msgBuf[i];              // Transmit i'th byte
    while (!(UCB0STAT & UCBUSY)) {};    // Wait till Tx is complete
}