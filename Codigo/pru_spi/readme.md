# Cargar el Device Tree Overlay:

## Manualmente 
1. Deshabilitar los que estuviesen activos

```
sudo su
cat /sys/devices/bone_capemgr.?/slots
 0: 54:PF--- 
 1: 55:PF--- 
 2: 56:PF--- 
 3: 57:PF--- 
 4: ff:P-O-L Bone-LT-eMMC-2G,00A0,Texas Instrument,BB-BONE-EMMC-2G
 5: ff:P-O-- Bone-Black-HDMI,00A0,Texas Instrument,BB-BONELT-HDMI
 6: ff:P-O-- Bone-Black-HDMIN,00A0,Texas Instrument,BB-BONELT-HDMIN
 8: ff:P-O-L Override Board Name,00A0,Override Manuf,BB-SPI1-01
echo -8 > /sys/devices/bone_capemgr.*/slots

```
2. Insaramos el nuevo:

```
sudo su
echo BB-PRU-SPI-E1 > /sys/devices/bone_capemgr.*/slots
```

## Automáticamente
Añade `CAPE=BB-PRU-SPI-E1` a `/etc/default/capemgr`