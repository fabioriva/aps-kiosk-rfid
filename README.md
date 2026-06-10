To compile programs with wiringPi Library, you need to include wiringPi.h as well as link against wiringPi

```sh
git clone https://github.com/WiringPi/WiringPi.git
cd wiringPi
./build
```

Create a script to modify the permissions of /dev/gpiomem
```sh
touch gpio-open.sh
```

```sh
#!/bin/bash
sudo chown root:gpio /dev/gpiomem && sudo chmod g+rw /dev/gpiomem
```
Make the script executable
```sh
chmod +x gpio-open.sh
```

Create systemd startup file
```sh
sudo touch /etc/systemd/system/gpio-open-startup.service
```

```sh
[Unit]
Description=gpio-open
# After=network.target
# After=systemd-user-sessions.service
# After=network-online.target

[Service]
# User=aps-kiosk
# Type=simple
# PIDFile=/run/gpio-open.pid
ExecStart=/home/aps-kiosk/gpio-open.sh start
# ExecReload=/home/aps-kiosk/gpio-open.sh reload
# ExecStop=/home/aps-kiosk/gpio-open.sh stop
# TimeoutSec=30
# Restart=on-failure
# RestartSec=30
# StartLimitInterval=350
# StartLimitBurst=10

[Install]
WantedBy=multi-user.target
```

Enable systemd file
```sh
sudo systemctl daemon-reload
sudo systemctl enable --now gpio-open-startup.service
```