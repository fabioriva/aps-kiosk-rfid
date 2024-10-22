from dotenv import load_dotenv
from rfid import Rfid

load_dotenv()

BLOCK_ADDRS = [4]
TRAILER_BLOCK = 7

# BLOCK_ADDRS = [8, 9, 10]
# TRAILER_BLOCK = 11

reader = Rfid(BLOCK_ADDRS, TRAILER_BLOCK)
reader.run()
