from rfid import Rfid

BLOCK_ADDRS = [4]
TRAILER_BLOCK = 7

# BLOCK_ADDRS = [8, 9, 10]
# TRAILER_BLOCK = 11

reader = Rfid(BLOCK_ADDRS, TRAILER_BLOCK)
text = input('New data:')
print("Now place your tag to write")
reader.write(text)
