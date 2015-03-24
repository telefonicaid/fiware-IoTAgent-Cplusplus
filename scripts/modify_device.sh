curl -X PUT http://127.0.0.1:8080/iot/devices/device_id \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: TestService" \
-H "Fiware-ServicePath: /TestSubservice" \
-d '{ "timezone": "Europe/Madrid" }'
