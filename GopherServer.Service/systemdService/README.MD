Copy contents of this folder to some location on your raspberry pi ex: rpi-service (create the dir if it doesn't exists)
Example using scp tool (it works like this: scp -r folder-on-your-pc username@computer-ip:absolute/path/on/your/remote-pc/folder-to-copy-to
```bash
scp -r service pi@raspberrypi.local:/home/pi/rpi-service
```
Allow install script to be executable by user
```bash
sudo chmod +x install.sh
```
run install script
```bash
sudo ./install.sh
```

Enable Service
```bash
systemctl enable gopherServer
```

Start Service
```bash
systemctl start gopherServer
```

Stop Service
```bash
systemctl stop gopherServer
```

see its status
```bash
sudo service gopherServer status
```

see if it it started (or if it didn't see why)
```bash
journalctl -u gopherServer.service
```
