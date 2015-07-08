curl -X PUT http://127.0.0.1:8080/iot/devices/device_id \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: testservice" \
-H "Fiware-ServicePath: /testsubservice" \
-d '{ "timezone": "Europe/Madrid", "static_attributes": [ { "name": "att_name_changed", "type": "string", "value": "value", "metadatas":[ {"name": "metaname", "type": "string", "value": "changed"} ] } ] }'
