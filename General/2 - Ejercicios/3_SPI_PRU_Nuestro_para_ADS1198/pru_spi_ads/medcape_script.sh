#!/bin/bash
DATE_NOW=`date +%d-%m-%Y:%H:%M:%S`
PATH_MEDCAPE='/home/debian/workspace2016/pru_spi_ads'
LOG_MEDCAPE='/home/debian/workspace2016/pru_spi_ads/log_medcape.txt'
cd $(dirname $0)

exec &>> $LOG_MEDCAPE
echo "" &> $LOG_MEDCAPE
echo "Starting medcape at $DATE_NOW"
echo "Waiting for modules to load..."
sleep 50
echo ""

function exec_medcape {
cd $PATH_MEDCAPE
./medcape &> $PATH_MEDCAPE/medcape_c.log
}

if [ ! -f $PATH_MEDCAPE/medcape_executed.txt ]; then #Check if medcape has been executed at least 1 time

        echo "Unmounting shared RAM..."
        rmmod uio_pruss
        echo ""

        sleep 1

        echo "Mounting shared RAM..."
        modprobe uio_pruss extram_pool_sz=0x001C20
        echo ""

fi
sleep 2
echo "Executing medcape..."
touch $PATH_MEDCAPE/medcape_executed.txt
#$PATH_MEDCAPE/medcape &> $PATH_MEDCAPE/medcape_c.log
exec_medcape

echo ""

echo "Medcape finalized..."
