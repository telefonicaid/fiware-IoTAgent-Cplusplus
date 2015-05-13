curl -X POST http://127.0.0.1:8080/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: testservice" \
-H "Fiware-ServicePath: /testsubservice" \
-d ' { "devices": [ { "device_id": "device_id", "entity_name": "entity_name", "entity_type": "entity_type",  "protocol" : "PDI-IoTA-UltraLight", "timezone": "America/Santiago", "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int" } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value" } ] } ] }'
