echo "Creation and modification of devices with same service and diferente subservices"


export HOST_IOT=127.0.0.1:8080
echo "HOST_IOT $HOST_IOT  ip and port for iotagent"
export HOST_CB=127.0.0.1:1026
echo "HOST_CB $HOST_CB ip and port for CB (Context Broker)"
export SERVICE=serv22
echo "SERVICE  $SERVICE to create device"
export SRVPATH=/srf
echo "SRVPATH $SRVPATH  new service_path to create device"

echo "create $SERVICE  $SRVPATH  for mqtt"
curl -X POST http://$HOST_IOT/iot/services \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtt" }]}'


echo "create $SERVICE  $SRVPATH  for ul20"
curl -X POST http://$HOST_IOT/iot/services \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeyul20", "token": "tokenul20", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingul20", "resource": "/iot/d" }]}'

echo "create device for mqtt"
curl -X POST http://$HOST_IOT/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_mqtt","protocol":"PDI-IoTA-MQTT-UltraLight"}]}'

echo "create device for ul20"
curl -X POST http://$HOST_IOT/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"devices":[{"device_id":"sensor_ul20","protocol":"PDI-IoTA-UltraLight"}]}'

echo "check type thingmqtt to iotagent"
curl -X GET http://$HOST_IOT/iot/devices/sensor_mqtt \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"

echo "check type thingmqtt to CB"

curl -X POST http://$HOST_CB/v1/queryContext \
-i \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"entities": [{ "id": "thingmqtt:sensor_mqtt", "type": "thingmqtt", "isPattern": "false" }]}'

echo "check type thingul20"
curl -X GET http://$HOST_IOT/iot/devices/sensor_ul20 \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"


echo "check type thingul20 to CB"

curl -X POST http://$HOST_CB/v1/queryContext \
-i \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"entities": [{ "id": "thingul20:sensor_ul20", "type": "thingul20", "isPattern": "false" }]}'

curl -X PUT http://$HOST_IOT/iot/devices/sensor_mqtt \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"endpoint" : "http://10.95.213.81:1026"}'

curl -X PUT http://$HOST_IOT/iot/devices/sensor_ul20 \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"endpoint" : "http://10.95.213.81:1026"}'

echo "check type thingmqtt"
curl -X GET http://$HOST_IOT/iot/devices/sensor_mqtt \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"

echo "check type thingmqtt"
curl -X GET http://$HOST_IOT/iot/devices/sensor_ul20 \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH"
