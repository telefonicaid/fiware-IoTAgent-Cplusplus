# Puppet Deployment

This chapter describe how to deploy using puppet an iotagent, configure and start it.

#### Index
1. [Introduction](#def-introduction)
2. [Obtaining the puppet code](#def-rpms)
3. [Configure the environment](#def-rpms)
4. [Deployment](#def-rpms)
5. [Basic Checks](#def-rpms)

<a name="def-introduction"></a>
## 1. Introduction

This guide explains how install the IoTAgent using puppet in development enviroments.

The puppet code asume that in this environment, every component that the IoTAgent dependends (like CB, Squid, IoTAgent Manager, etc) is installed in localhost. 

If your environment have different architecture, please refer to the chapter 3.

## 2. Obtaining the puppet code

The puppet code for the deployment of the IoTAgent is not contained in this repository. 

For obtain the deployment puppet code:

- Set the proxy:
```
git config --global http.proxy http://prod-epg-ost-proxy-01.hi.inet:6666
```

- Clone the repository:
```
git clone http://puppet-code.aislada.hi.inet/puppet/iot_puppet.git && cd iot_puppet
```
## 3. Configure the environment

### 3.1 Defaults 

The deployment using this puppet code asume by default that several components are installed locally:

- ContextBroker (LISTEN=0.0.0.0, Port=1026)
- No Squid
- No AUTH with PEP
- No HA for IoTAgents

#### 3.2 Configure the components

If your components are installed/configured following other architecture you must adjust some variables in the puppet hieradata properly:

- Edit the puppet development hieradata:
```
vi +535 hieradata/environments/development.yaml 
```

- Set the specific variables into the development hieradata:

	- General
    ```
    # IoTAgent Server Name
    iotagent::iotagent_server_name      : 'xxxx'
    # IoTAgent Log Level
    iotagent::iotagent_log_level        : 'xxxx'
    # IoTAgent Server Port
    iotagent::iotagent_server_port      : 'xxxx'
    # IoTAgent Config file
    iotagent::iotagent_config_file      : 'xxxx'
    ```

	- IoTAgent Manager
    ```
    # Install IoTAgent Manager?
    iotagent::with_manager              : 'xxxx'
    # IoTAgent Log Level
	iotagent::iotagentm_log_level       : 'xxxx'
    # IoTAgent Manager Port
    iotagent::iotagentm_server_port     : "xxxx"
    # IoTAgent Manager Config file
    iotagent::iotagentm_config_file      : 'xxxx'
	```
	
    - ContextBroker 
	```
    # ContextBroker Endpoint
    iotagent::context_broker_endpoint   : 'xxxx'
	```

	- Mongo
	```
    # Mongo Host
    iotagent::iotagent_mongodb_host     : ['xxxx']
    # Mongo Port
    iotagent::iotagent_mongodb_port     : 'xxxx'
    # Mongo Replicaset Name (if you have Mongo in RPSet)
    iotagent::iotagent_rplSet_name      : ''
	```

	- Pep
	```
	# Pep Config Stuff
    iotagent::trust_token_url           : ''
    iotagent::user_token_url            : ''
    iotagent::user_roles_url            : ''
    iotagent::user_subservices_url      : ''
    iotagent::user_access_control_url   : ''
    iotagent::pep_user                  : 'xxxx'
    iotagent::pep_password              : 'xxxx'
    iotagent::pep_domain                : 'xxxx'
    iotagent::trust_token_user          : 'xxxx'
    iotagent::trust_token_password      : 'xxxx'
    iotagent::auth_timeout              : 'xxxx'
    ```

#### 3.3 Protocols in IoTAgent

By default, the puppet install only the UL and SMS-Repsol protocols. If you want to install other protocols (UL, MQTT, SMS-REPSOL, TT, EVADTS), you must to add these in development hieradata.

This is an example, installing UL, SMS-Repsol and MQTT, but NOT installing TT and Evadts:

    iotagent:
        ul:
            order            : "20"
            resource         : "d"
            fileName         : "UL20Service"
        mqtt:
           order            : "30"
            resource         : "mqtt"
            fileName         : "MqttService"
            options          : "\"ConfigFile\": \"/etc/iot/MqttService.xml\""
            mosquitto_host   : "localhost"
            mosquitto_port   : "1883"
            mosquitto_user   : "admin"
            mosquitto_pass   : "1234"
        sms-repsol:
            order            : "40"
            resource         : "repsol"
            fileName         : "RepsolService"
            options          : "\"ConfigFile\": \"/etc/iot/Repsol.xml\""
            smpp_adapter_host: "/simulaClient/smpp1"
            smpp_adapter_port: "5371"
            smpp_adapter_url : "10.95.213.159"
            from_number      : "682996050"
        # tt:
        #     order            : "50"
        #     resource         : "tt"
        #     fileName         : "TTService"
        #     options          : "\"ConfigFile\": \"/etc/iot/TTService.xml\""
        # evadts:
        #     order            : "60"
        #     resource         : "evadts"
        #     fileName         : "EvadtsService"
        #     options          : "\"ConfigFile\": \"/etc/iot/evadts.json\""

NOTE: There are some issues with MQTT HA. Please if you want to install Mosquitto in HA, use an Active/Pasive architecture.

## 4. Deployment

To deploy the IoTAgent using puppet:

- Deploy the prerequisites (repositories, iptables, selinux, etc)
```
sudo puppet apply --environment development --debug --hiera_config=hiera.yaml --modulepath=modules -e "include preinstall"
```

- Deploy the IoTAgent:
```
sudo puppet apply --environment development --debug --hiera_config=hiera.yaml --modulepath=modules -e "include iot-iotagent-fe"
```
This deployment (by default) install and configure these components:

	- IoTAgent (Port=8080) with UL and SMS-Repsol protocols
    - IoTAgent Manager (Port=8081)
    - Monit 
    - MongoDB 2.6.9

## 5. Basic checks:

When the puppet is finished without errors, there are some procedures that you can follow for check if IoTAgent (and their dependencies) are deployed correctly:

NOTE: This checks are made asuming the default configuration of puppet deployment.

- Check monit summary:

    ```
    # monit summary
    The Monit daemon 5.1.1 uptime: 2h 18m 

    Process 'iotagent_manager'          running
    Process 'iotagent'                  running
    ```

- Check the iotagent processes:

	```
    # ps -ef | grep iotagent | grep -v grep
    iotagent  3285     1  0 16:34 ?        00:00:01 /usr/local/iot/bin/iotagent -m -n Manager -v INFO -i 10.0.0.35 -p 8081 -d /usr/local/iot/lib -c /etc/iot/config.json
    iotagent  3333     1  0 16:34 ?        00:00:01 /usr/local/iot/bin/iotagent -n IoTPlatform -v INFO -i 10.0.0.35 -p 8080 -d /usr/local/iot/lib -c /etc/iot/config.json
    ```

- Check the logs:

	```
    # ls -lhrt /var/log/iot/
    total 80K
    -rw-r-----. 1 iotagent iotagent 27K May 12 18:55 IoTAgent-Manager.log
    -rw-r-----. 1 iotagent iotagent 44K May 12 18:55 IoTAgent-IoTPlatform.log
	```

- Check the ports listened by IoTAgent:

	```
    # lsof -nPi | grep iotagent | grep LISTEN
    iotagent 3285 iotagent   39u  IPv4  27514      0t0  TCP 10.0.0.35:8081 (LISTEN)
    iotagent 3333 iotagent   44u  IPv4  27578      0t0  TCP 10.0.0.35:8080 (LISTEN)
	```