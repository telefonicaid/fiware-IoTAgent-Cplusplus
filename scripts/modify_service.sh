curl -X PUT 'http://127.0.0.1:8080/iot/services/testservice?apikey=apikey3&resource=/iot/d' \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: testservice" \
-H "Fiware-ServicePath: /testsubservice" \
-d '{ "apikey": "NEWapikey", "token": "token2", "cbroker": "http://127.0.0.1:1026", "entity_type": "thing", "resource": "/iot/d" }'
