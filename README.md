```
sudo apt-get install pigpio
sudo systemctl enable pigpiod
sudo systemctl start pigpiod

gcc -o fan_control fan_control.c -lpigpio -lrt -lpthread

sudo nano /etc/systemd/system/fan_control.service
sudo systemctl daemon-reload
sudo systemctl enable fan_control.service
sudo systemctl start fan_control.service
```
