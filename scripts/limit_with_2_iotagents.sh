#!/bin/bash
#
# Check limit and offset with 2 iotagents in GET device of iot manager

echo "Check limit and offset with 2 iotagents in GET device of iot manager"

# define varibles values for test
export HOST_IOT1=127.0.0.1:8080
echo "HOST_IOT $HOST_IOT1  ip and port for iotagent 1"
export HOST_IOT2=127.0.0.1:80
eecho "HOST_IOT $HOST_IOT2  ip and port for iotagent 2"
export HOST_MAN=127.0.0.1:8081
eecho "HOST_IOT $HOST_MAN  ip and port for iot manager"
export SERVICE=serv22
echo "SERVICE  $SERVICE to create device"
export SRVPATH=/srf
echo "SRVPATH $SRVPATH  new service_path to create device"

#general functions
trap "exit 1" TERM
export TOP_PID=$$

declare -i number_errors=0

function assert_code()
{
res_code=` echo $res | grep -c "#code:$1"`
if [ $res_code -eq 1 ]
then
echo " OKAY"
else
echo  " ERROR: " $2
((number_errors++))
kill -s TERM $TOP_PID
fi
}

function assert_contains()
{
if [[ "$res" =~ .*"$1".* ]]
then
echo " OKAY"
else
echo  " ERROR: " $2
((number_errors++))
kill -s TERM $TOP_PID
fi

}

# TEST
echo "10- create $SERVICE  $SRVPATH  for mqtt"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtt" }]}' )
echo $res
assert_code 201 "service already exists"

echo "20-create $SERVICE  $SRVPATH  for ul20"
res=$( curl -X POST http://$HOST_IOT2/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeyul20", "token": "tokenul20", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingul20", "resource": "/iot/d" }]}' )
echo $res
assert_code 201 "service already exists"

echo "30- create device for mqtt"
res=$( curl -X POST http://$HOST_IOT1/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_mqtt","protocol":"PDI-IoTA-MQTT-UltraLight"}]}' )
echo $res
assert_code 201 "device already exists"

res=$( curl -X POST http://$HOST_IOT1/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_mqtt2","protocol":"PDI-IoTA-MQTT-UltraLight"}]}' )
echo $res
assert_code 201 "device already exists"


echo "40- create device for ul20"
res=$( curl -X POST http://$HOST_IOT2/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_ul20","protocol":"PDI-IoTA-UltraLight"}]}' )
echo $res
assert_code 201 "device already exists"

res=$( curl -X POST http://$HOST_IOT2/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_ul202","protocol":"PDI-IoTA-UltraLight"}]}' )
echo $res
assert_code 201 "device already exists"

echo "50- check type thingmqtt to iotagent"
res=$( curl -X GET http://$HOST_IOT1/iot/devices/sensor_mqtt \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )
echo $res
assert_code 200 "device already exists"

echo "60- check type thingul20"
res=$( curl -X GET http://$HOST_IOT2/iot/devices/sensor_ul20 \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )
echo $res
assert_code 200 "device already exists"

echo "70- GET all iot manager"

res=$( curl -X GET http://$HOST_MAN/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )
echo $res
assert_code 200 "device already exists"
assert_contains '{ "count": 2'  "no 2 services"

echo "80- GET all iot manager"

res=$( curl -X GET "http://$HOST_MAN/iot/devices?limit=-1" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )
echo $res
assert_code 200 "device already exists"
assert_contains '{ "count" : 4'  "no 4 devices"

echo "90- GET all iot manager offset 2 limit 1"

res=$( curl -X GET "http://$HOST_MAN/iot/devices?offset=2&limit=1" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )
echo $res
assert_code 200 "device already exists"
assert_contains '{ "count" : 4'  "no 4 devices"
assert_contains '{ "device_id" : "sensor_ul20" }'  "no 4 devices"

echo "100- GET all iot manager offset 1 limit 2"

res=$( curl -X GET "http://$HOST_MAN/iot/devices?offset=1&limit=2" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )
echo $res
assert_code 200 "device already exists"
assert_contains '{ "count" : 4'  "no 4 devices"
assert_contains '{ "device_id" : "sensor_mqtt2" }'  "no 4 devices"
assert_contains '{ "device_id" : "sensor_ul20" }'  "no 4 devices"

echo "110- GET all iot manager offset 3 limit 22"

res=$( curl -X GET "http://$HOST_MAN/iot/devices?offset=3&limit=22" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )
echo $res
assert_code 200 "device already exists"
assert_contains '{ "count" : 4'  "no 4 devices"
assert_contains '{ "device_id" : "sensor_ul202" }'  "no 4 devices"

echo "120- GET all iot manager offset 0 limit 1"

res=$( curl -X GET "http://$HOST_MAN/iot/devices?offset=0&limit=1" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )
echo $res
assert_code 200 "device already exists"
assert_contains '{ "count" : 4'  "no 4 devices"
assert_contains '{ "device_id" : "sensor_mqtt" }'  "no 4 devices"


echo "150- delete service mqtt"
res=$( curl -X DELETE "http://$HOST_IOT1/iot/services?resource=/iot/mqtt&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )
assert_code 204 "device already exists"


echo "160- delete service ul20"
res=$( curl -X DELETE "http://$HOST_IOT2/iot/services?resource=/iot/d&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )
assert_code 204 "device already exists"


echo " ALL tests are OK"
