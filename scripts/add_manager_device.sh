
curl -X POST http://127.0.0.1:8081/iot/devices \
-i \
-H "Accept: application/json,text/json" \
-H "Content-Type: application/json" \
-H "Fiware-Service: testsrv" \
-H "Fiware-ServicePath: /ssrvtest" \
-d ' { "devices": [ { "device_id": "device_2", "entity_name": "room_2", "entity_type": "thing", "protocol": "PDI-IoTA-UltraLight", "endpoint": "http://0.0.0.0:3333", "commands": [ { "name": "ping", "type": "command", "value": ""} ], "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int" } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value" } ] } ] }'
