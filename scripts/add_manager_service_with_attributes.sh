
curl -X POST http://127.0.0.1:8081/iot/services \
-i \
-H "Accept: application/json,text/json" \
-H "Content-Type: application/json" \
-H "Fiware-Service: testsrv" \
-H "Fiware-ServicePath: /ssrvtestattr" \
-d '{ "services": [{ "apikey": "apikey3", "token": "token2","protocol": ["PDI-IoTA-UltraLight"], "cbroker": "http://127.0.0.1:1026", "entity_type": "thing", "static_attributes": [{"name": "color", "type": "string", "value": "blue"}] }]}'
