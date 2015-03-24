curl -X PUT 'http://127.0.0.1:8080/iot/services/TestService?apikey=apikey3&resource=/iot/test' \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: TestService" \
-H "Fiware-ServicePath: /TestSubservice" \
-d '{ "apikey": "NEWapikey", "token": "token2", "cbroker": "http://127.0.0.1:1026", "entity_type": "thing", "resource": "/iot/test" }'
