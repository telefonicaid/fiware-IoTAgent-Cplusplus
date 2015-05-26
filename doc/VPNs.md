# Using SmartM2M VPNs (draft in progress)
The goal is to configure a Linux cluster to keep active a GRE tunnel and receive requests on a _squid_ proxy (http://www.squid-cache.org). This tunnel connects the IoTA infrastructure with the M2M router.


## HA configuration
Activate _ip forwarding_ editing _/etc/sysctl.conf_ (as root if needed):

```
net.ipv4.ip_forward=1
```

Check if it is enabled:
```
cat /proc/sys/net/ipv4/ip_forward
```
If it is 0 execute:
```
sysctl –w net.ipv4.ip_forward=1
```

Before starting the installation process, disabling SELinux is required.
Execute: 
```
getenforce
```
If it returns _Permissive_ SELinux is disabled. If it returns _Enforcing_ execute 
```
setenforce 0
``` 
and edit _etc/selinux/config_:
```
SELINUX=permisive
```

To configure high availability (HA), two servers will be used, with corresponding IPs (IP-A and IP-B in the configuration examples), and an additional IP to be used for HA (IP-HA) . Hearbeat will be used to monitor and manage the nodes of the cluster. To do this, install _hearbeat_ (¿release? rpms from EPEL repository), make sure that the names of the nodes that will be part of the cluster are properly resolved (using the /etc/hosts and IPs IP-proxygre2 IP-proxygre1) . Create a script called iotagre-heartbeat.sh with execute permissions for root in the home for that user (in proxygre1 and proxygre2) with the following contents:

```
#!/bin/bash

if [ "$1" != "master" -a "$1" != "slave" ];then
        echo "Error: Use master/slave parameter"
        echo "install.sh master or install.sh slave"
        exit 127
fi
NODE_TO_INSTALL=$1
##################
# IPs Enviroment #
##################
NODE1_IP=<ip_proxygre1>
NODE1_HOSTNAME=proxygre1
NODE1_HA_DEV=eth1
NODE2_IP=<ip_proxygre2>
NODE2_HOSTNAME=proxygre2
NODE2_HA_DEV=eth1
HA_IPMASK=<ip_proxygreHAmask>

echo "Adding HA hostname in your /etc/hosts..."
        echo "A backup of your current hosts file is saved in /etc/hosts.save"
        cp -ax /etc/hosts /etc/hosts.save
        echo -e "$NODE1_IP\t$NODE1_HOSTNAME" > /etc/hosts
        echo -e "$NODE2_IP\t$NODE2_HOSTNAME" >> /etc/hosts
        cat /etc/hosts.save >> /etc/hosts
echo "Done!"

echo "Checking Hearbeat 3 installation..."
if [ `rpm -qa | grep heartbeat-3 | wc -l` -ne 1 ];then
        echo "Heartbeat 3 is not installed. Exiting..."
        exit 127
fi
echo "Configurating..."
cp -ax /usr/share/doc/heartbeat-3.0.4/ha.cf /etc/ha.d/
cp -ax /usr/share/doc/heartbeat-3.0.4/haresources /etc/ha.d/
cp -ax /usr/share/doc/heartbeat-3.0.4/authkeys /etc/ha.d/
chmod 600 /etc/ha.d/authkeys
echo "auth 1" >> /etc/ha.d/authkeys
echo "1 md5 1D4S-ha" >> /etc/ha.d/authkeys
echo "$NODE1_HOSTNAME $HA_IPMASK" >> /etc/ha.d/haresources
if [ "$NODE_TO_INSTALL" = "master" ];then
        echo "ucast $NODE1_HA_DEV $NODE2_HOSTNAME" >> /etc/ha.d/ha.cf
else
        echo "ucast $NODE2_HA_DEV $NODE1_HOSTNAME" >> /etc/ha.d/ha.cf
fi
echo "node $NODE1_HOSTNAME" >> /etc/ha.d/ha.cf
echo "node $NODE2_HOSTNAME" >> /etc/ha.d/ha.cf
if [ `getenforce` == "Enforcing" ];then
        echo "Warning!! SELinux activated"
        setenforce 0
        if [ `getenforce` == "Permissive" ];then
                echo "SELinux disabled correctly"
        else
                echo "Disabled SELinux configuration. Please verify your SELinux state"
        fi
        echo "Please, set your SELinux setup to Permissive in /etc/selinux/config"
fi
echo "Done!"
service heartbeat start
```

Execute the script as root, in proxygre1
```
./ iotagre-heartbeat.sh master
```
in proxygre2:
```
./iotagre-heartbeat.sh slave
```

## GRE tunnel
Create _/etc/sysconfig/network-scripts/ifcfg-iotagre01_ :
```
DEVICE=iotagre01
TYPE=GRE
PEER_OUTER_IPADDR=<ip_routerM2M>
MY_OUTER_IPADDR=<ip_internal_tunnel>
ONBOOT=yes
MY_INNER_IPADDR=10.0.0.1
```

Create _/etc/sysconfig/network-scripts/route-iotagre01_:
```
default dev iotagre01 table iotagre01
```
Create _/etc/sysconfig/network-scripts/rule-iotagre01_:
```
from 192.168.96.1 to 192.168.20.0/24 table iotagre01
```
The file _/etc/init.d/nat-proxy_ has following content:
```
#!/bin/bash
iptables -F -t nat
iptables -t nat -A POSTROUTING -o iotagre01 -j SNAT --to-source 172.32.255.100
```

Permissions an run levels:
```
chmod a+x /etc/init.d/nat-proxy
ln -s /etc/init.d/nat-proxy /etc/rc3.d/K99nat-proxy
ln -s /etc/init.d/nat-proxy /etc/rc3.d/S99nat-proxy
ln -s /etc/init.d/nat-proxy /etc/rc5.d/K99nat-proxy
ln -s /etc/init.d/nat-proxy /etc/rc5.d/S99nat-proxy
```
Loopback IP in _/etc/sysconfig/network-scripts/ifcfg-lo1_
```
DEVICE=lo:1
IPADDR=192.168.96.1
NETMASK=255.255.255.255
NETWORK=192.168.96.0
BROADCAST=192.168.96.1
ONBOOT=yes
NAME=loopback
```

In _/etc/iproute2/rt_tables_ add:
```
1 iotagre01
```

Get up interfaces:
```
ifdown lo
ifup lo
ifup iotagre01
/etc/init.d/nat-proxy
```

## 5.3	Proxy configuration
We will use squid as proxy. Configuration files are in _/etc/squid/conf.d_.
File _iotagre01-tunnel.conf_:
```
acl GRE_iotagre01 req_header X-M2M-Outgoing-Route <DEVICES_APN>
http_access allow CONNECT GRE_iotagre01
http_access allow GRE_iotagre01
tcp_outgoing_address 192.168.96.1 GRE_iotagre01
```
File _NoTunnel.conf_:
```
acl NoTunnel req_header X-M2M-Outgoing-Route .*
http_access allow CONNECT !NoTunnel1
```
Add this line in _/etc/squid/squid.conf_:
```
include "/etc/squid/conf.d/*.conf"
```
Only a deny clause must exists before include clause:
```
     35 http_access deny manager
     36
     37 include "/etc/squid/conf.d/*.conf"
```
Run levels:
```
chkconfig --level 2345 squid on
```
and start service
```
service squid restart
```
## Add new client
First, we create a new GRE tunnel (iotagre0X, where X is a number)
File _/etc/sysconfig/network-scripts/ifcfg-iotagre0X_
```
DEVICE=iotagre0X
TYPE=GRE
PEER_OUTER_IPADDR=<ip_routerM2M>
MY_OUTER_IPADDR=<ip_internal_tunnel>
ONBOOT=yes
MY_INNER_IPADDR=10.0.0.X
```
File _/etc/sysconfig/network-scripts/route-iotagre0X_ 
```
default dev iotagre0X table iotagre0X
```
File _/etc/sysconfig/network-scripts/rule-iotagre0X_
```
from 192.168.96.X to XXX.XXX.XXX.XXX/XX table iotagre0X
```
Add a new rule to file _/etc/init.d/nat-proxy_ 
```
iptables -t nat -A POSTROUTING -o iotagre0X -j SNAT --to-source 172.32.255.100
```
Configuring loopback in _/etc/sysconfig/network-scripts/ifcfg-loX_:
```
DEVICE=lo:X
IPADDR=192.168.96.X
NETMASK=255.255.255.255
NETWORK=192.168.96.0
BROADCAST=192.168.96.X
ONBOOT=yes
NAME=loopback
```
In _/etc/iproute2/rt_tables_ ädd:
```
X iotagre0X
```
Get up interfaces:
```
ifdown lo
ifup lo
ifup iotagre0X
/etc/init.d/nat-proxy
```
Configuring tunnel in proxy _/etc/squid/conf.d/_:
```
iotagre0X-tunnel.conf:
acl GRE_iotagre0X req_header X-M2M-Outgoing-Route <DEVICES_APN>
http_access allow CONNECT GRE_iotagre0X
http_access allow GRE_iotagre0X
tcp_outgoing_address 192.168.96.X GRE_iotagre0X 
```
## Configuring router
Templates in order to configure M2M router:
Incomming connections:

|    DCA_DEST_PRIVATE    |   IP interna del servicio DCA, a la que será traducida desde la IP pública de servicio que ataca el cliente.     |
|DCA_PORT_1|Puerto número 1 del servicio DCA (8002)
|DCA_PORT_2|	Puerto número 2 del servicio DCA (8003)
|SERVICE_IP	|IP pública de servicio DCA
|CUST_ID|	Identificador de cliente
ip access-list extended ACL_PAT_cust<CUST_ID>
! nueva entrada en ACL, podría haber más servicios
permit any <SERVICE_IP>
 
ip nat outside source static tcp <DCA_DEST_PRIVATE> <DCA_PORT_1> <SERVICE_IP> <DCA_PORT_1> vrf cust<CUST_ID> extendable
ip nat outside source static tcp <DCA_DEST_PRIVATE> <DCA_PORT_2> <SERVICE_IP> <DCA_PORT_2> vrf cust<CUST_ID> extendable
 
ip route vrf cust<CUST_ID> <SERVICE_IP> 255.255.255.255 vasiright<CUST_ID>
Conexiones salientes:
Realizar la configuración por cada proxy que haya que configurar, teniendo en cuenta que el túnel GRE configurado cambia sus datos para cada configuración de proxy. Es decir, por cada proxy  los siguientes campos serán distintos:
•         PROXY_ID (Contador creciente. Mínimo 1)
•         TUNNEL_GRE_DCA_SRC
•         TUNNEL_GRE_DCA_DST
•         PROXY_IP
 
Datos requeridos
PROXY_ID	Identificador de proxy (contador creciente desde 1). Formato con 2 dígitos.
TUNNEL_GRE_DCA_SRC	Extremo de túnel GRE del servicio proxy VPN en el router
TUNNEL_GRE_DCA_DST	Extremo de túnel GRE del servicio proxy VPN en el proxy
SERVICE_ID	Identificador del servicio. Formato con 2 dígitos
PROXY_IP	IP pública de proxy VPN
CUST_ID	Identificador de cliente. Formato con 3 dígitos
El túnel tiene una subred interna con /30 cómo máximo. La subred está formada por 4 direcciones. La primera y la última no se utilizan (subred y broadcast). La segunda se configura en el lado del router M2M y la tercera en el extremo remoto. Por ello, el extremo router M2M tendrá la IP 4*PROXY_ID-3 y el extremo remoto 4*PROXY_ID-2. Estas IPs no son relevantes para el servicio, pero son útiles para la gestión de la conectividad del túnel.
 
 
interface Loopback1<CUST_ID><SERVICE_ID><PROXY_ID>
description Source IP of Tunnel GRE with proxy VPN
vrf forwarding m2m_global
ip address <TUNNEL_GRE_DCA_SRC> 255.255.255.255
 
interface Tunnel1<CUST_ID><SERVICE_ID><PROXY_ID>
description Customer <CUST_ID> - Internal GRE Tunnel <SERVICE_ID> <PROXY_ID>
vrf forwarding cust<CUST_ID>
ip address 20.<CUST_ID>.<SERVICE_ID>.{4*<PROXY_ID>-3} 255.255.255.252
load-interval 30
tunnel source Loopback1<CUST_ID><SERVICE_ID><PROXY_ID>
tunnel destination <TUNNEL_GRE_DCA_DST>
tunnel vrf m2m_global
 
ip route vrf cust<CUST_ID> <PROXY_IP> 255.255.255.255 Tunnel1<CUST

