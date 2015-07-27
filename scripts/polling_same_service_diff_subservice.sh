echo "polling command of devices with same service and subservices, different entity_type"


export HOST_IOT=127.0.0.1:8080
echo "HOST_IOT $HOST_IOT  ip and port for iotagent"
export HOST_CB=127.0.0.1:1026
echo "HOST_CB $HOST_CB ip and port for CB (Context Broker)"
export SERVICE=serv22
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

echo "create $SERVICE  $SRVPATH  for ul"
res=$( curl -X POST http://$HOST_IOT/iot/services \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeyul", "token": "tokenul", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingul", "resource": "/iot/d" }]}' )

assert( res , 200)

echo "create $SERVICE  $SRVPATH  for ul20"
res=$( curl -X POST http://$HOST_IOT/iot/services \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeyul20", "token": "tokenul20", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtt" }]}' )

echo "create device for ul"
res=$( curl -X POST http://$HOST_IOT/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_ul","protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}' )

echo "create device for ul20"
res=$( curl -X POST http://$HOST_IOT/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_mqtt","protocol":"PDI-IoTA-MQTT-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}' )

echo "check type thingul to iotagent"
res=$( curl -X GET http://$HOST_IOT/iot/devices/sensor_ul \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )

echo "check type thingmqtt to CB"

res=$( curl -X POST http://$HOST_CB/v1/queryContext \
-i \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"entities": [{ "id": "thingul:sensor_ul", "type": "thingul", "isPattern": "false" }]}' )
echo "comprobar que viene PING"

echo "check type thingul20"
res=$( curl -X GET http://$HOST_IOT/iot/devices/sensor_mqtt \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )


echo "check type thingul20 to CB"

res=$( curl -X POST http://$HOST_CB/v1/queryContext \
-i \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"entities": [{ "id": "thingmqtt:sensor_mqtt", "type": "thingmqtt", "isPattern": "false" }]}' )
echo "OJO este no tiene PING, pero e registro ha sido bueno"


res=$( curl -X POST http://$HOST_CB/v1/updateContext \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"updateAction":"UPDATE","contextElements":[{"id":"thingul:sensor_ul","type":"thingul","isPattern":"false","attributes":[{"name":"PING","type":"command","value":"22","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2014-11-23T17:33:36.341305Z"}]}]} ]}' )
echo $res
assert_code 200 "device already exists"
#assert_contains '{"updateAction":"UPDATE","contextElements":[{"id":"thingul:sensor_ul","type":"thingul","isPattern":"false","attributes":[{"name":"PING","type":"command","value":"22","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2014-11-23T17:33:36.341305Z"}]}]} ]}'
echo "devuelve ping y 200"


echo "queryContext para ver e  PING_status  pending"

echo "sleep"

echo "queryContext para ver e  PING_status  expired_read"
----> ERROR se crea una entidad nueva "id" : "thing:sensor_ul",  con "PING_status" : {  "expired read"  }

echo "150- delete service mqtt"
res=$( curl -X DELETE "http://$HOST_IOT/iot/services?resource=/iot/mqtt&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )
assert_code 204 "device already exists"


echo "160- delete service ul20"
res=$( curl -X DELETE "http://$HOST_IOT/iot/services?resource=/iot/d&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )
assert_code 204 "device already exists"
