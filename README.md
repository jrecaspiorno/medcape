# Concetarse:
Para conectarse usar:
```
ssh 192.168.7.2 -l debian
#default username:password is [debian:temppwd]

```
# Paquetes extra:
* `ntp`: sicronizar la hora con la red.
* `sshfs`: montar directorios remotos.
* `apt-get install linux-headers-$(uname -r)`: cabeceras del kernel.

# Network
[Share internet mediante USB](https://elementztechblog.wordpress.com/2014/12/22/sharing-internet-using-network-over-usb-in-beaglebone-black/)

Default ip `192.168.7.2`
BBB console can be got by using the following command:
```
ssh 192.168.7.2 -l root
```
In the BBB console type the following
```
ifconfig usb0 192.168.7.2
route add default gw 192.168.7.1
```
Averigua la interfaz por la que sale el equipo de sobremesa. La lista de interfaces se consulata `ls /sys/class/net`. Su nombre está relacionado con el tipo a veces (`en*` para ethernet, `wl*` wireless). En mi caso `enp5s0` es la interfaz de red para el PC y `enx689e1956a3a0` es la conexión USB de la BBB.

In the linux console of host system type
```bash
sudo su
#enp5s0 is my internet facing interface, enx689e1956a3a0 is the BeagleBone USB connection
ifconfig enx689e1956a3a0 192.168.7.1
iptables --table nat --append POSTROUTING --out-interface enp5s0 -j MASQUERADE
iptables --append FORWARD --in-interface enx689e1956a3a0 -j ACCEPT
echo 1 > /proc/sys/net/ipv4/ip_forward
```

Incase network not accessible then type the following in BBB terminal
```
echo "nameserver 8.8.8.8" >> /etc/resolv.conf
```

Note: Assuming that wlan0 of host system should be shared with USB-Ethernet eth5. User may change these configuration matching to their interface names.

Now the network can be accessed using the BBB...


# Montar un directorio de mi servidor
Tras instalar `sshfs` y crear el directgorio `/home/debian/shared/`:
```
sshfs -o idmap=user jrecas@jrecas-ws.dacya.ucm.es:/home/jrecas/Trabajo/Programacion/BBB /home/debian/shared/
```

# SPI

He creado el fichero `/media/jrecas/BEAGLEBONE/uEnv.txt` en el que especifico que se deshabilita el HDMI y se carga el `device tree` de SPI `/lib/firmware/BB-SPI1-01-00A0.dtbo`:

```
optargs=quiet drm.debug=7 capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN capemgr.enable_partno=BB-SPI1-01
```
Este fichero lo he sacado de `http://elinux.org/BeagleBone_Black_Enable_SPIDEV`. Para comprobar que está cargado despues del rebbot y que el HDMI está deshabilitado hago:

```
cat /sys/devices/bone_capemgr.?/slots
 0: 54:PF---
 1: 55:PF---
 2: 56:PF---
 3: 57:PF---
 4: ff:P-O-L Bone-LT-eMMC-2G,00A0,Texas Instrument,BB-BONE-EMMC-2G
 5: ff:P-O-- Bone-Black-HDMI,00A0,Texas Instrument,BB-BONELT-HDMI
 6: ff:P-O-- Bone-Black-HDMIN,00A0,Texas Instrument,BB-BONELT-HDMIN

```

y veo que no está la `L` en las líneas de HDMI. También veo que no me hace ni puto caso con lo de cargar el SPI, por lo que hago :
```
sudo su
echo BB-SPI1-01 > /sys/devices/bone_capemgr.*/slots
exit
```
En esta web `http://elinux.org/Beagleboard:BeagleBoneBlack_Debian#Loading_custom_capes` hay un `workarround`: añade `CAPE=BB-SPI1-01` a `/etc/default/capemgr`. Despues de reiniciar ya está todo bien y disponemos del driver:

```
ls -al /dev/spidev1.*
crw-rw---T 1 root spi 153, 0 Mar 10 22:30 /dev/spidev1.0
```
Configurción:
```
D1 Output
D0 Input
```
# LEDs

There are four user LEDs on the BeagleBone. You can modify them, but they each have their own purposes by default. USER0 is the closest to the top in the picture at the right, and USER3 is the bottom one closest to the ethernet port.
```
USER0 is the heartbeat indicator from the Linux kernel.
USER1 turns on when the SD card is being accessed
USER2 is an activity indicator. It turns on when the kernel is not in the idle loop.
USER3 turns on when the onboard eMMC is being accessed.
```
You can change each of the LED's behaviors at the following locations:
```
/sys/class/leds/beaglebone\:green\:usr0/
/sys/class/leds/beaglebone\:green\:usr1/
/sys/class/leds/beaglebone\:green\:usr2/
/sys/class/leds/beaglebone\:green\:usr3/
```
Para apagar el LED0:
```
sudo sh -c "echo 0 > /sys/class/leds/beaglebone\:green\:usr0/brightness"
```
Para encender:
```
sudo sh -c "echo 255 > /sys/class/leds/beaglebone\:green\:usr0/brightness"
```

Para dejarlo como estaba:
```
sudo sh -c "echo heartbeat > /sys/class/leds/beaglebone\:green\:usr0/trigger"
```

# Interrupciones

Mirar en `/proc/interrupts` para ver los eventos

# Configuración de pines
Los pines están así configurados para 2 leads en la primera versión:

  * Pin1 (IN2N-IN1P+51k1) -> **RA/R**
  * Pin2 (IN1P+51k1)      -> **LA/L**
  * Pin3 (IN2P+51k1)      -> **LL/F**
  * Pin4 (RLD*)           -> **RL/N** (Sólo para evitar artefactos)

# ADS1x98ECG
Conexión de pines:

  * Alimentación J4:


J4 Pin        | J4 Pin
:------------:|:-----------:
2             | 1
4             | 3
6             | 5 (GND->P9-1)
8             | 7
10 (5V->P9-5) | 9 (3V3->P9-3)

  * SPI

 Signal        | J3 Pin Num  | J3 Pin Num  | Signal       
:-------------:|:-----------:|:-----------:|:-----------:
 CLKSEL        | 2           | 1  (P9-28)  | START/**CS** 
 GND           | 4           | 3  (P9-31)  | CLK          
 GPIO1         | 6           | 5           | NC           
 RESETB        | 8  (P9-25)  | 7           | CS           
 GND           | 10          | 9           | NC           
 GPIO2         | 12          | 11 (P9-30)  | DIN          
 NC/**START**  | 14 (P9-27)  | 13 (P9-23)  | DOUT         
 NC            | 16          | 15 (P9-23)  | DRDYB        
 GND           | 18          | 17          | NC           
 NC            | 20          | 19          | NC          