echo "Creation and modification of devices with same service and different subservices"


export HOST_IOT=127.0.0.1:8080
echo "HOST_IOT $HOST_IOT  ip and port for iotagent"
export HOST_CB=127.0.0.1:1026
echo "HOST_CB $HOST_CB ip and port for CB (Context Broker)"
export SERVICE=serv22
echo "SERVICE  $SERVICE to create device"
export SRVPATH1=/srf1
echo "SRVPATH1 $SRVPATH1  new service_path to create device"
export SRVPATH2=/srf2
echo "SRVPATH1 $SRVPATH2  new service_path to create device"

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
-H "Fiware-ServicePath: $SRVPATH1" \
-d '{"services": [{ "apikey": "apikeyul", "token": "tokenul", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingul", "resource": "/iot/d" }]}' )

assert( res , 200)

echo "create $SERVICE  $SRVPATH  for ul20"
res=$( curl -X POST http://$HOST_IOT/iot/services \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH2" \
-d '{"services": [{ "apikey": "apikeyul20", "token": "tokenul20", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingul20", "resource": "/iot/d" }]}' )

echo "create device for ul"
res=$( curl -X POST http://$HOST_IOT/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH1" \
-d '{"devices":[{"device_id":"sensor_1","protocol":"PDI-IoTA-UltraLight"}]}' )

echo "create device for ul20"
res=$( curl -X POST http://$HOST_IOT/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH2" \
-d '{"devices":[{"device_id":"sensor_2","protocol":"PDI-IoTA-UltraLight"}]}' )

echo "check type thingul to iotagent"
res=$( curl -X GET http://$HOST_IOT/iot/devices/sensor_1 \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH1" )

echo "check type thingmqtt to CB"

res=$( curl -X POST http://$HOST_CB/v1/queryContext \
-i \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH1" \
-d '{"entities": [{ "id": "thingul:sensor_1", "type": "thingul", "isPattern": "false" }]}' )

echo "check type thingul20"
res=$( curl -X GET http://$HOST_IOT/iot/devices/sensor_2 \
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
-H "Fiware-ServicePath: $SRVPATH2" \
-d '{"entities": [{ "id": "thingul20:sensor_2", "type": "thingul20", "isPattern": "false" }]}' )

res=$( curl -X PUT http://$HOST_IOT/iot/devices/sensor_1 \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH1" \
-d '{"endpoint" : "http://10.95.213.81:1026"}' )

res=$( curl -X PUT http://$HOST_IOT/iot/devices/sensor_2 \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH2" \
-d '{"endpoint" : "http://10.95.213.81:1026"}' )

echo "check type thingul"
res=$( curl -X GET http://$HOST_IOT/iot/devices/sensor_1 \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH1" )

echo "check type thingul20"
res=$( curl -X GET http://$HOST_IOT/iot/devices/sensor_2 \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH2" )
