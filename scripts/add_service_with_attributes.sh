curl -X POST http://127.0.0.1:8080/iot/services \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: testservice" \
-H "Fiware-ServicePath: /testsubserviceatrributes" \
-d '{"services": [{ "apikey": "apikey3", "token": "token2", "cbroker": "http://127.0.0.1:1026", "entity_type": "thing", "resource": "/iot/d", "static_attributes": [{"name": "color", "type": "string", "value": "orange"}] }]}'
