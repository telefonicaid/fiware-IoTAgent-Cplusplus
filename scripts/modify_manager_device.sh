
curl -X PUT http://127.0.0.1:8081/iot/devices/device_2?protocol=PDI-IoTA-UltraLight \
-i \
-H "Accept: application/json,text/json" \
-H "Content-Type: application/json" \
-H "Fiware-Service: testsrv" \
-H "Fiware-ServicePath: /ssrvtest" \
-d ' { "endpoint": "http://0.0.0.0:4444"}'
