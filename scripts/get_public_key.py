import binascii

from ledgerblue.comm import getDongle
from ledgerblue.commException import CommException


bip44_path = ( # example of bip44 path
      "8000002C"
    + "80000378"
    + "00000000"
    + "00000000"
    + "00000000"
    )


dongle = getDongle(True)

public_key = dongle.exchange(
    bytes(bytearray.fromhex("80040000FF" + bip44_path)))

print "\n"
print "public_key [" + str(len(binascii.hexlify(public_key))) + "] = " + binascii.hexlify(public_key)
