
curl -X POST http://127.0.0.1:8081/iot/protocols \
-i \
-H "Accept: application/json,text/json" \
-H "Content-Type: application/json" \
-H "Fiware-Service: service2" \
-H "Fiware-ServicePath: /ssrv2" \
-d '{"protocol": "PDI-IoTA-UltraLight", "description": "Ultralight 2.0","iotagent" : "http://127.0.0.1:8023/iot", "resource" : "/iot/d", "services": [{ "apikey": "apikey3", "service": "testsrv", "service_path": "/ssrvtest",  "token": "token2", "cbroker": "http://127.0.0.1:1026", "entity_type": "thing", "resource": "/iot/d" }]}'
 

