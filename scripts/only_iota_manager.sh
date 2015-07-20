echo "polling command of devices with same service and subservices, different entity_type"


export HOST_MAN=127.0.0.1:8081
echo "HOST_IOT $HOST_MAN  ip and port for iotagent"
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
echo "get PROTOCOLS"
res=$( curl -X GET http://$HOST_MAN/iot/protocols \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )

echo " debe haber 8080 80 los dos iotagents en PDI-IoTA-UltraLight y PDI-IoTA-MQTT-UltraLight "

echo "create $SERVICE  $SRVPATH  for ul1"
res=$( curl -X POST http://$HOST_MAN/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "protocol": ["PDI-IoTA-UltraLight","PDI-IoTA-MQTT-UltraLight"] }]}' )

assert( res , 200)

echo "get $SERVICE  $SRVPATH  for ul1"
res=$( curl -X GET http://$HOST_MAN/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"  )

----> ERROR me devuelve 0, cuando se han creado todos bien

assert( res , 200)
echo "deberia haber 4 servicios"

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

echo "check type thingul to iotagent"
res=$( curl -X GET http://$HOST_MAN/iot/devices/sensor_ul \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )

echo "check type thingmqtt to CB"

res=$( curl -X POST http://$HOST_CB/v1/queryContext \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"entities": [{ "id": "thingul:sensor_ul", "type": "thingul", "isPattern": "false" }]}' )
echo "comprobar que viene PING"

echo "check type thingul20"
res=$( curl -X GET http://$HOST_MAN/iot/devices/sensor_mqtt \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )


echo "check type thingul20 to CB"

res=$( curl -X POST http://$HOST_CB/v1/queryContext \
-i -s -w "#code:%{http_code}#" \
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
-d '{"updateAction":"UPDATE","contextElements":[{"id":"thingsrv:sensor_ul","type":"thingsrv","isPattern":"false","attributes":[{"name":"PING","type":"command","value":"22","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2014-11-23T17:33:36.341305Z"}]}]} ]}' )
echo $res
assert_code 200 "device already exists"
#assert_contains '{"updateAction":"UPDATE","contextElements":[{"id":"thingul:sensor_ul","type":"thingul","isPattern":"false","attributes":[{"name":"PING","type":"command","value":"22","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2014-11-23T17:33:36.341305Z"}]}]} ]}'
echo "devuelve ping y 200"


echo "queryContext para ver e  PING_status  pending"

echo "sleep"

echo "queryContext para ver e  PING_status  expired_read"
----> ERROR se crea una entidad nueva "id" : "thing:sensor_ul",  con "PING_status" : {  "expired read"  }

echo "send a command"
res=$( curl -X POST http://$HOST_CB/v1/updateContext \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"updateAction":"UPDATE","contextElements":[{"id":"thingsrv:sensor_ul","type":"thingsrv","isPattern":"false","attributes":[{"name":"PING","type":"command","value":"22","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2014-11-23T17:33:36.341305Z"}]}]} ]}' )
echo $res


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
