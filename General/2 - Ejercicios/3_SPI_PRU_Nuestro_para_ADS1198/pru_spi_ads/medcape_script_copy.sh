#!/bin/bash
DATE_NOW=`date +%d-%m-%Y:%H:%M:%S`
PATH_MEDCAPE='/home/debian/workspace2016/pru_spi_ads'
LOG_MEDCAPE='/home/debian/workspace2016/pru_spi_ads/log_medcape.txt'
cd $(dirname $0)

exec &>> $LOG_MEDCAPE
echo "" &> $LOG_MEDCAPE
echo "Starting medcape at $DATE_NOW" 
echo "Waiting for pru and other modules to load..."
sleep 50
echo "Modules loaded"

#-------------Config Bluetooth-------------------------------
echo "Setting up bluetooth config..."
#-Reset bluetooth del dongle:
hciconfig hci0 reset
sleep 1
#Hacer visible el bluetooth del dongle:
hciconfig hci0 piscan
sleep 1
#Deactivate bluetooth service
rfkill block bluetooth
sleep 1
sudo service bluetooth stop
sleep 1
rfkill unblock bluetooth
sleep 1
#Activate bluetooth service:
bluetoothd &
sleep 1
echo "Bluetooth config loaded!"
#-------------------------------------------------------------


#---------Config shared RAM between PRU and userspace--------
echo "Unmounting shared RAM..."
rmmod uio_pruss
echo ""

sleep 1

echo "Mounting shared RAM..."
modprobe uio_pruss extram_pool_sz=0x001C20
echo ""
#-------------------------------------------------------------

sleep 2

function exec_medcape {
cd $PATH_MEDCAPE
echo "========================================="
./medcape &
echo "========================================="
}


function exec_python_bluetooth {
cd $PATH_MEDCAPE
echo "========================================="
echo ""
echo ""
echo "Everything running OK!..."
python BTConnection.py
echo "========================================="
}

echo "Executing medcape..."
#exec_medcape

sleep 10

echo ""
echo ""
echo "Executing Python Bluetooth..."
#exec_python_bluetooth

echo ""

echo "Medcape finalized..."
