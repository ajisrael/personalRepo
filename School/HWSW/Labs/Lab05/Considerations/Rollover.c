void checkRollover()
{
   volatile alt_u16 * timStatPtr = (alt_u16 *) TIME_ADDR;
   if ((*timStatPtr & 0x1))
   {
      str = "Rollover!"
      str[10] = 0x00;
      PutTerm(str);
   }
} 