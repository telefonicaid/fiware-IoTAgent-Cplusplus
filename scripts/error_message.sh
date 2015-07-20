#!/bin/bash
#
# Check limit and offset with 2 iotagents in GET device of iot manager

echo "Check limit and offset with 2 iotagents in GET device of iot manager"

# define varibles values for test
export HOST_IOT1=127.0.0.1:8080
echo "HOST_IOT $HOST_IOT1  ip and port for iotagent 1"
export HOST_IOT2=127.0.0.1:80
echo "HOST_IOT $HOST_IOT2  ip and port for iotagent 2"
export HOST_MAN=127.0.0.1:8081
echo "HOST_IOT $HOST_MAN  ip and port for iot manager"
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

echo "without mongod started"
echo '{"reason":"Conctact with your administrator, there was an internal error","details":"can't connect couldn't connect to server 127.0.0.1:27017 (127.0.0.1), connection attempt failed"}'

echo "database unauthorized"
echo '{"reason":"Database error","details":"error: SERVICE: { service: serv22, service_path: /srf1 } what=count fails:{ ok: 0.0, errmsg: not authorized on iot to execute command { count: SERVICE, query: { service: serv22, service_path: /srf1 } }, code: 13 }"}'


echo "0- delete old data"
res=$( curl -X DELETE "http://$HOST_IOT1/iot/services?resource=/iot/mqtt&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" )


# TEST

echo "10- create service without apikey"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{  "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtt" }]}' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason":"The request is not well formed","details":"Missing required property: apikey '  "no apikey"

#echo "11- create service with bad resource"
#res=$( curl -X POST http://$HOST_IOT1/iot/services \
#-i -s -w "#code:%{http_code}#" \
#-H "Content-Type: application/json" \
#-H "Fiware-Service: $SERVICE" \
#-H "Fiware-ServicePath: $SRVPATH" \
#-d '{"services": [{ "apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtttt" }]}' )
#echo $res
#assert_code 201 "no 400"
#echo "apuntado, es correcto"

echo "12- create service with bad service"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: BAD_SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtt" }]}' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason": "Malformed header","details": "Fiware-Service not accepted - a service string must not be longer than 50' "bad service"

echo "13- create service with bad service_path"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: bad_srv_path" \
-d '{"services": [{ "apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtttt" }]}' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason": "Malformed header","details": "Fiware-ServicePath not accepted - a ser' "bad service_path"

echo "14- create service with  no service header"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtttt" }]}' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason": "Malformed header","details": "Fiware-Service not accepted' "no service header"

echo "15- create service with bad type"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeymqtt", "token": 1234, "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtttt" }]}' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason":"The request is not well formed","details":"invalid data type: UnsignedType expected: string' "bad type"

echo "16- create service with bad cbroker"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtttt" }]}' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason":"A parameter of the request is invalid/not allowed[http10.95.213.36:1026]","details":"http10.95.213.36:1026 is not a correct uri' "bad uri"

echo "17- create service with bad atributes"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "attributes": [ { "objectid":"t","type": "string", "name":"temperature" } ],"apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtttt" }]}' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason":"The request is not well formed","details":"Missing required property: object_id' "bad parameter"


echo "18- create service with bad json"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{  [ { "objectid":"t","type": "string", "name":"temperature" } ],"apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtttt" }]}' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason":"The request is not well formed","details":"JSONParser: An error occurred on line 1 column 18' "bad json"

echo "19- create with extra data in json"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{  "extra_data" :"not alllowed","apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtttt" }]}' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason":"The request is not well formed","details":"Additional properties not allowed' "extra data"



echo "20- create a device"
res=$( curl -X POST http://$HOST_IOT1/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{ "devices": [ { "device_id": "device_id", "entity_name": "entity_name", "entity_type": "entity_type",  "protocol" : "PDI-IoTA-UltraLight", "timezone": "America/Santiago", "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int", "metadatas":[ {"name": "metaname", "type": "int", "value": "45"} ] } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value", "metadatas":[ {"name": "metaname", "type": "string", "value": "45"} ] } ] } ] }' )
echo $res
assert_code 404 "no 404"
assert_contains '{"reason":"The service does not exist","details":" service:serv22 service_path:/srf"}' "no service"


echo "21- create service"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtt" }]}' )
echo $res
assert_code 201 "create service"

echo "21.1- duplicated service"
res=$( curl -X POST http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{"services": [{ "apikey": "apikeymqtt", "token": "tokenmqtt", "cbroker": "http://10.95.213.36:1026", "entity_type": "thingmqtt", "resource": "/iot/mqtt" }]}' )
echo $res
assert_code 409 "create service"
assert_contains '{"reason":"There are conflicts, object already exists","details":"duplicate key: iot.SERVICE' "duplicated data"


echo "22- GET no service"
res=$( curl -X GET http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: noservice" \
-H "Fiware-ServicePath: $SRVPATH"  )
echo $res
assert_code 200 "create service"
assert_contains '{ "count": 0,"services": []}' "no service"

echo "23- put  no resource"
res=$( curl -X PUT http://$HOST_IOT1/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{ "token": "tokenmqtt"}' )
echo $res
assert_code 400 "create service"
assert_contains '{"reason":"A parameter is missing in the request","details":"resource parameter is mandatory in PUT operation"}' "no resource"


echo "24- put   service"
res=$( curl -X PUT "http://$HOST_IOT1/iot/services?resource=/iot/mqtt&apikey=apikeymqtt" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{ "token": "tokenmqtt"}' )
echo $res
assert_code 204 "create service"

echo "30- create a device with empty device_id"
res=$( curl -X POST http://$HOST_IOT1/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{ "devices": [ { "device_id": "", "entity_name": "entity_name", "entity_type": "entity_type",  "protocol" : "PDI-IoTA-UltraLight", "timezone": "America/Santiago", "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int", "metadatas":[ {"name": "metaname", "type": "int", "value": "45"} ] } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value", "metadatas":[ {"name": "metaname", "type": "string", "value": "45"} ] } ] } ] }' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason":"The request is not well formed","details":"String is too short (0 chars), minimum 1 [/devices[0]/device_id]"}' "empty device_id"

echo "31- create a device  bad protocol"
res=$( curl -X POST http://$HOST_IOT1/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE" \
-H "Fiware-ServicePath: $SRVPATH" \
-d '{ "devices": [ { "device_id": "device_id", "entity_name": "entity_name", "entity_type": "entity_type",  "protocol" : "PDI-IoTA-UltraLightt", "timezone": "America/Santiago", "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int", "metadatas":[ {"name": "metaname", "type": "int", "value": "45"} ] } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value", "metadatas":[ {"name": "metaname", "type": "string", "value": "45"} ] } ] } ] }' )
echo $res
assert_code 400 "no 400"
assert_contains '{"reason":"There are conflicts, protocol is not correct","details":" [ protocol: PDI-IoTA-UltraLightt]"}' "bad protocol"

