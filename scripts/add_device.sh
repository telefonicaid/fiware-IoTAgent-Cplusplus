curl -X POST http://127.0.0.1:8080/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: testservice" \
-H "Fiware-ServicePath: /testsubservice" \
-d ' { "devices": [ { "device_id": "device_id", "entity_name": "entity_name", "entity_type": "entity_type",  "protocol" : "PDI-IoTA-UltraLight", "timezone": "America/Santiago", "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int", "metadatas":[ {"name": "metaname", "type": "int", "value": "45"} ] } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value", "metadatas":[ {"name": "metaname", "type": "string", "value": "45"} ] } ] } ] }'
