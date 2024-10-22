import os
import logging
import RPi.GPIO as GPIO
import requests
from time import sleep
from SimpleMFRC522_ import SimpleMFRC522
# from mfrc522 import SimpleMFRC522


logging.basicConfig(
    format='%(asctime)s [%(levelname)s] %(message)s', level=logging.INFO)
logging.getLogger(__name__)

GPIO.setwarnings(False)


class Rfid():
    def __init__(self, BLOCK_ADDRS, TRAILER_BLOCK):
        self.reader = SimpleMFRC522(BLOCK_ADDRS, TRAILER_BLOCK)

    def run(self):
        try:
            while 1:
                self.read()
                sleep(0.5)
        except KeyboardInterrupt:
            GPIO.cleanup()
        finally:
            GPIO.cleanup()

    def read(self):
        id, text = self.reader.read()
        logging.info(f'Tag ID {id}, Data: {text}')
        r = requests.post(os.getenv('API_URL'), json={"id": id})
        logging.info("response: %s" % (r.text))

    def write(self, text):
        self.reader.write(text)
        logging.info(f'Tag written: {text}')
