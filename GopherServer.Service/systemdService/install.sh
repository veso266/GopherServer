#!/usr/bin/env bash

SERVICE_FILE=/etc/systemd/system
PROGRAM_FILE=/home/veso266/gopherServer/GopherServer.Service

echo "create Gopher Server service ..."


if [[ -f "$SERVICE_FILE" ]]; then
    echo "$SERVICE_FILE exists, removing it"
    sudo rm $SERVICE_FILE
fi

sudo cp gopherServer.service $SERVICE_FILE
chmod +x $PROGRAM_FILE
echo "created Gopher Server service"
sudo systemctl daemon-reload


echo "Reloading deamons"
echo "Enabling Service"
sudo systemctl enable gopherServer
echo "Gopher Server is enabled now"
echo "Starting Service"
sudo service gopherServer restart
echo "Gopher Server is started now, you can view its logs with: tail -f /var/log/gopherServer.log"
