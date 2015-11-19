from lettuce import step, world
from common.steps import service_created_precond,service_with_attributes_created_precond,device_with_entity_values_created_precond,device_with_attributes_created_precond,check_measure_cbroker,check_measures_cbroker,check_NOT_measure_cbroker,check_measure_cbroker_entity,check_measure_cbroker_with_attributes
from iotqautils.gtwMeasures import Gw_Measures_Utils
from common.gw_configuration import CBROKER_URL,IOT_SERVER_ROOT,UL20_APIKEY
import requests
import time,datetime

gw = Gw_Measures_Utils(server_root=IOT_SERVER_ROOT)
world.device={}

@step('I send a measure to the GW with apikey, id "([^"]*)", protocol "([^"]*)", alias "([^"]*)", timestamp "([^"]*)" and value "([^"]*)"')
def send_measure(step, device_id, protocol, alias, timestamp, value):
    measures= []
    measure = {}
    measure['alias'] = alias
    measure['value'] = value
    world.timestamp=False
    world.def_entity=True
    if timestamp:
        measure['timestamp'] = timestamp
        world.st=timestamp
        world.timestamp=True
    else:
        ts = time.time()
        st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
        world.st=st
        world.ts=ts
    measures.append(measure)
    if world.service_name:
        apikey = 'apikey_'+world.service_name
    else:
        apikey = UL20_APIKEY
    req = gw.sendMeasure(protocol,apikey,device_id,measures)
    assert req.ok, 'ERROR: ' + req.text


@step('I send several measures to the GW with apikey, id "([^"]*)", protocol "([^"]*)"')
def send_measures(step, device_id, protocol):
    measures = []
    world.measures_count=0
    world.def_entity=True
    for measures_dict in step.hashes:
        measure = {}
        measure['alias'] = measures_dict['alias']
        measure['value'] = measures_dict['value']
        measures.append(measure)
        world.measures_count+=1
    print 'measures: ' + str(measures)
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    requests.post(CBROKER_URL+"/reset")
    if world.service_name:
        apikey = 'apikey_'+world.service_name
    else:
        apikey = UL20_APIKEY
    req = gw.sendMeasure(protocol,apikey,device_id,measures)
    assert req.ok, 'ERROR: ' + req.text
        

@step('I send a wrong measure to the GW with apikey, id "([^"]*)", protocol "([^"]*)", alias "([^"]*)", value "([^"]*)", timestamp "([^"]*)" and wrong field "([^"]*)"')
def send_wrong_measure(step, device_id, protocol, alias, value, timestamp, field):
    measures= []
    measure = {}
    measure['alias'] = alias
    measure['value'] = value
    if field=="timestamp":
        measure['timestamp'] = timestamp
        ts = time.time()
        st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
        world.st=st
        world.ts=ts
    measures.append(measure)
    world.field=field
    world.def_entity=True    
    if world.service_name:
        apikey = 'apikey_'+world.service_name
    else:
        apikey = UL20_APIKEY
    if field=="not_apikey":
        req = gw.sendMeasure("/iot/d?i={}","",device_id,measures)
    elif field=="not_device":
        req = gw.sendMeasure("/iot/d?k={}",apikey,"",measures)        
    elif field=="wrong_apikey":
        req = gw.sendMeasure(protocol,field,device_id,measures)
    else:
        req = gw.sendMeasure(protocol,apikey,device_id,measures)
    if field:
        if field=="timestamp":
            assert req.ok, 'ERROR: ' + str(req)
        else:
            assert not req.ok, 'ERROR: ' + str(req)
    else:
        assert req.ok, 'ERROR: ' + str(req)