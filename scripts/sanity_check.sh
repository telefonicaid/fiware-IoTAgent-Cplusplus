#!/bin/bash
# sanity_check_iotagent.sh
# Sanity check for iota agent installation


# Copyright 2015 Telefonica Investigación y Desarrollo, S.A.U
#
# This file is part of iotagent project.
#
# iotagent is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# iotagent is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with iotagent. If not, see http://www.gnu.org/licenses/.
#
# For those usages not covered by the GNU Affero General Public License
# please contact with iot_support at tid dot es

echo "sanity check for iot agent"

declare -i number_errors=0
start_time=`date +%s`

function assert_code()
{
echo $res
res_code=` echo $res | grep -c "#code:$1"`
if [ $res_code -eq 1 ]
then
echo " OKAY"
else
echo  " ERROR: " $2
((number_errors++))
delete_all_data
exit -1
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
delete_all_data
exit -1
fi

}

function write_help()
{
    echo "sanity check for iota agent installation"
    echo "usage:    sanity_check_manger.sh [-a/--iotagent IOTA_AGENT_URI] [-m/--manager IOTA_AGENT_URI]  [-v/--verbose LEVEL] "
    echo "example:  sanity_check_manger.sh --iotagent 127:0.0.1:8080 --manager 127:0.0.1:8081 -verbose 0"
    echo "other option: you can use environment variables  manager (export HOST_MAN=127.0.0.1:8081)"
    echo "                                                 iotagent (export HOST_IOT=127.0.0.1:8080)"
}

function delete_all_data()
{
# delete old data
res=$(curl -X DELETE "http://$HOST_IOT/iot/services?resource=/iot/d&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST")

}

# check parameters
if [ "$1" == "-h" ] || [ "$1" == "--help" ] ; then
    write_help
    exit -1
elif [ "$1" == "-m" ] || [  "$1" == "--manager" ] ; then
    export HOST_MAN=$2
elif [ "$1" == "-a" ] || [  "$1" == "--iotagent" ] ; then
    export HOST_IOT=$2
fi

if [ "$3" == "-h" ] || [ "$3" == "--help" ] ; then
    write_help
    exit -1
elif [ "$3" == "-m" ] || [  "$3" == "--manager" ] ; then
    export HOST_MAN='$4/iot'
elif [ "$3" == "-a" ] || [  "$3" == "--iotagent" ] ; then
    export HOST_IOT=$4
fi

if test -z "$HOST_IOT" ; then
   export HOST_IOT=127.0.0.1:8080
fi

if test -z "$HOST_MAN" ; then
   export HOST_MAN=127.0.0.1:8081
fi

if test -z "$SERVICE_TEST" ; then
   export SERVICE_TEST=srvcheck
fi

if test -z "$SERVICE_PATH_TEST" ; then
   export SERVICE_PATH_TEST='/spathcheck'
fi

#exits iota manager
echo "1- uri for iota agent is $HOST_IOT"
res=$( curl -X GET http://$HOST_IOT/iot/about \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" )

assert_code 200 "iota manager no respond"
assert_contains "IoTAgents" "respond is not from iotagent"

echo 2- uri for iot manager http://$HOST_MAN/iot/about
res=$( curl -X GET http://$HOST_MAN/iot/about \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" )

echo "uri for iota Manager is $HOST_MAN"
assert_code 200 "iota manager no respond"

delete_all_data

echo "10- create service $SERVICE_TEST  $SERVICE_PATH_TEST  for ul"
res=$( curl -X POST http://$HOST_IOT/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "resource": "/iot/d" }]}' )

assert_code 201 "cannot create service"

echo "11- get service $SERVICE_TEST  $SERVICE_PATH_TEST  for ul"
res=$( curl -X GET http://$HOST_IOT/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST"  )

assert_code 200 "get service fails"
assert_contains "srvcheck" "no correct respond from context broker  "

echo "20- create device for ul"
res=$( curl -X POST http://$HOST_IOT/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"devices":[{"device_id":"sensor_ul","protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}' )

assert_code 201 "cannot create device for ultralight"

echo "21- check type thingul to iotagent"
res=$( curl -X GET http://$HOST_IOT/iot/devices/sensor_ul \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" )

assert_code 200 "get device fails"
assert_contains "srvcheck" "no correct respond from context broker  "

echo "31- send Observation"
res=$( curl -X GET "http://$HOST_IOT/iot/d?i=sensor_ul&k=apikey" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" )

assert_code 200 "bad send observation"

echo "50- delete device ul20"
res=$( curl -X DELETE "http://$HOST_IOT/iot/devices/sensor_ul?resource=/iot/d" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" )
assert_code 204 "delete device"

echo "51- delete service ul20"
res=$( curl -X DELETE "http://$HOST_IOT/iot/services?resource=/iot/d&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" )
assert_code 204 "delete service"

echo "get PROTOCOLS"
res=$( curl -X GET http://$HOST_MAN/iot/protocols \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST"  )

assert_code 200 "no protocols"


echo "110- create $SERVICE_TEST  $SERVICE_PATH_TEST  for ul"
res=$( curl -X POST http://$HOST_MAN/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "protocol": ["PDI-IoTA-UltraLight"] }]}' )

assert_code 201 "cannot create service"

echo "111- get $SERVICE_TEST  $SERVICE_PATH_TEST  for ul"
res=$( curl -X GET http://$HOST_MAN/iot/services \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST"  )

assert_code 200 "get service fails"
assert_contains "srvcheck" "no correct respond from context broker  "

echo "121-create device for ul"
res=$( curl -X POST http://$HOST_MAN/iot/devices \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" \
-d '{"devices":[{"device_id":"sensor_ul","protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}' )

assert_code 201 "cannot create device for ultralight"

echo "122- check type thingul to iotagent"
res=$( curl -X GET http://$HOST_MAN/iot/devices/sensor_ul \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" )

assert_code 200 "get cb fails"
assert_contains "200" "no correct respond from context broker"
assert_contains "sensor_ul" "no correct respond from context broker"

echo "150- delete device ul20"
res=$( curl -X DELETE "http://$HOST_MAN/iot/devices/sensor_ul?protocol=PDI-IoTA-UltraLight" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" )
assert_code 204 "delete device"

echo "60- delete service ul20"
res=$( curl -X DELETE "http://$HOST_MAN/iot/services?protocol=PDI-IoTA-UltraLight&device=true" \
-i -s -w "#code:%{http_code}#" \
-H "Content-Type: application/json" \
-H "Fiware-Service: $SERVICE_TEST" \
-H "Fiware-ServicePath: $SERVICE_PATH_TEST" )
assert_code 204 "device already exists"

echo sanity check is OK in $(expr `date +%s` - $start_time) seg

exit


