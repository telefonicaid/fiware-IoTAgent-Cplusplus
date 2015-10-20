
export HOST_IOT=127.0.0.1:8080
export HOST_MAN=127.0.0.1:8081

export SERVICE_TEST=srvchec2

export SERVICE_PATH_TEST='/spathcheck'


curl -X POST http://$HOST_IOT/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "resource": "/iot/d" }]}'

curl -X POST http://$HOST_IOT/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "resource": "/iot/mqtt" }]}'

curl -X POST http://$HOST_IOT/iot/devices \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"devices":[{"device_id":"sensor_ul", "entity_name":"room_ut1" ,"protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}'

curl -X POST http://$HOST_IOT/iot/devices \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"devices":[{"device_id":"sensor_mqtt", "entity_name":"room_ut1" ,"protocol":"PDI-IoTA-MQTT-UltraLight", "commands": [{"name": "PINGMQTT","type": "command","value": "" }]}]}'

curl -X POST http://$HOST_IOT/iot/devices \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"devices":[{"device_id":"sensor_ulbad", "entity_name":"room_ut1" ,"protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}'



curl -X POST http://$HOST_IOT/iot/ngsi/d/updateContext \
-i  \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"updateAction":"UPDATE","contextElements":[{"id":"room_ut1","type":"thingsrv","isPattern":"false","attributes":[{"name":"PING","type":"command","value":"22","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2014-11-23T17:33:36.341305Z"}]}]} ]}'



curl -X POST http://$HOST_IOT/iot/ngsi/d/updateContext \
-i  \
-H "Content-Type: application/json" \
-H "Accept: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"updateAction":"UPDATE","contextElements":[{"id":"room_ut5","type":"thingsrv","isPattern":"false","attributes":[{"name":"PING","type":"command","value":"22","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2014-11-23T17:33:36.341305Z"}]}]} ]}'


echo "150- delete device ul20"
curl -X DELETE "http://$HOST_IOT/iot/devices/sensor_ul?protocol=PDI-IoTA-UltraLight" \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST"


curl -X DELETE "http://$HOST_IOT/iot/devices/sensor_mqtt?protocol=PDI-IoTA-MQTT-UltraLight" \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST"

echo "60- delete service ul20"
curl -X DELETE "http://$HOST_MAN/iot/services?protocol=PDI-IoTA-UltraLight&device=true" \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST"


curl -X DELETE "http://$HOST_MAN/iot/services?protocol=PDI-IoTA-MQTT-UltraLight&device=true" \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST"





