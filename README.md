# aps-kiosk-rfid

Raspberry pi4 os Debian GNU/Linux 12 (bookworm)

Using picamera2 in virtual environments
```
python3 -m venv .venv

pip install mfrc522
pip install python-dotenv
pip install requests

pm2 --name=rfid start app.py
