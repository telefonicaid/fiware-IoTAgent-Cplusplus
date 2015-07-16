echo "polling command of devices with same service and subservices, different entity_type"


export HOST_MAN=127.0.0.1:8081
echo "HOST_IOT $HOST_MAN  ip and port for iotagent"
export HOST_CB=10.95.213.36:1026
echo "HOST_CB $HOST_CB ip and port for CB (Context Broker)"
export SERVICE=serv2f
echo "SERVICE  $SERVICE to create device"
export SRVPATH=/srf1
echo "SRVPATH $SRVPATH1  new service_path to create device"

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
echo "get PROTOCOLS"
res=$( curl -X GET http://$HOST_MAN/iot/protocols \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )

echo $res

echo " debe haber 8080 80 los dos iotagents en PDI-IoTA-UltraLight y PDI-IoTA-MQTT-UltraLight "

echo "create $SERVICE  $SRVPATH  for ul1"
res=$( curl -X POST http://$HOST_MAN/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "protocol": ["PDI-IoTA-UltraLight","PDI-IoTA-MQTT-UltraLight"] }]}' )

sleep 1

echo $res
assert_code 201 "service already exists"

echo "get $SERVICE  $SRVPATH "
res=$( curl -X GET http://$HOST_MAN/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )

echo $res
assert_code 200 "get service no ok"
assert_contains '"count": 4' "no 4 elemnts"

echo "create device for ul"
res=$( curl -X POST http://$HOST_MAN/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_ul","protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "sensor_ul@command|%s" }]}]}' )

echo "create device for mqtt"
res=$( curl -X POST http://$HOST_MAN/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_mqtt","protocol":"PDI-IoTA-MQTT-UltraLight", "commands": [{"name": "PING","type": "command","value": "sensor_mqtt@command|%s" }]}]}' )

sleep 1

echo "check type thingul to iotagent"
res=$( curl -X GET http://$HOST_MAN/iot/devices/sensor_ul \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )

echo "check sensor_ul in CB"

res=$( curl -X POST http://$HOST_CB/v1/queryContext \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"entities": [{ "id": "thingsrv:sensor_ul", "type": "thingsrv", "isPattern": "false" }]}' )
echo $res
assert_code 200 "get service no ok"
assert_contains 'sensor_ul@command|%s' "no element in CB"

echo "check sensor_mqtt in CB"

res=$( curl -X POST http://$HOST_CB/v1/queryContext \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"entities": [{ "id": "thingsrv:sensor_mqtt", "type": "thingsrv", "isPattern": "false" }]}' )
echo $res
assert_code 200 "get service no ok"
assert_contains 'sensor_mqtt@command|%s' "no element in CB"

echo "mandamos un comando"

res=$( curl -X POST http://$HOST_CB/v1/updateContext \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"updateAction":"UPDATE","contextElements":[{"id":"thingsrv:sensor_ul","type":"thingsrv","isPattern":"false","attributes":[{"name":"PING","type":"command","value":"22","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2014-11-23T17:33:36.341305Z"}]}]} ]}' )
echo $res
assert_code 200 "device already exists"
assert_contains 'sensor_mqtt@command|%s' "no element in CB"

echo "150- delete service mqtt"
res=$( curl -X DELETE "http://$HOST_MAN/iot/services?protocol=PDI-IoTA-MQTT-UltraLight&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )
assert_code 204 "device already exists"


echo "160- delete service ul20"
res=$( curl -X DELETE "http://$HOST_MAN/iot/services?protocol=PDI-IoTA-UltraLight&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )
assert_code 204 "device already exists"

echo "ALL OK"
