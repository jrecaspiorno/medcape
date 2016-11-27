#Uso:
#sudo sshfs -o idmap=user [mi_usuario_del_PC]@[la_IP_de_mi_PC]:[ruta_al_directorio_shared_del_PC] [ruta_al_directorio_shared_de_la_BBB] 

#Yo por ejemplo lo hice así. Sustituir las rutas por las de vuestro PC:
sudo sshfs -o idmap=user ubuntu@192.168.7.1:/home/ubuntu/workspace /home/debian/workspace2016
