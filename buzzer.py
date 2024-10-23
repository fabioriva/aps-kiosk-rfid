import RPi.GPIO as GPIO
import time

CHANNEL = 2

GPIO.setmode(GPIO.BCM)
GPIO.setup(CHANNEL, GPIO.OUT)
print(GPIO.getmode())
while 1:
  print("in")
  GPIO.output(CHANNEL, GPIO.HIGH)
  time.sleep(1)
  print("out")
  GPIO.output(CHANNEL, GPIO.LOW)
  time.sleep(1)
