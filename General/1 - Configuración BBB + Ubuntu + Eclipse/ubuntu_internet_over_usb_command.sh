sudo iptables --table nat --append POSTROUTING --out-interface wlp8s0 -j MASQUERADE
sudo iptables --append FORWARD --in-interface enx544a16e725b8 -j ACCEPT
sudo echo 1 > /proc/sys/net/ipv4/ip_forward

