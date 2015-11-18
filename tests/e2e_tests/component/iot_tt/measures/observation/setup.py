from lettuce import step, world
from common.steps import service_created_precond,device_created_precond
from iotqautils.gtwMeasures import Gw_Measures_Utils
from common.gw_configuration import CBROKER_URL,CBROKER_HEADER,IOT_SERVER_ROOT,DEF_ENTITY_TYPE

from common.functions import Functions
import requests, urllib
import time, datetime

gw = Gw_Measures_Utils(server_root=IOT_SERVER_ROOT)
functions = Functions()
world.field={}
world.regs_number={}
world.are_measures=False
world.multidata=False
world.measure=''
world.measure2=''
world.device={}

@step('I send a measure to the GW with name "([^"]*)", protocol "([^"]*)", type "([^"]*)" and value "([^"]*)"')
def send_measure(step, device_id, protocol, obs_type, value):
    measures= []
    world.values= []
    world.metadatas= []
    world.types = []
    measure = "#"+device_id+","
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    num_measure=0
    measure += "#"+str(num_measure)+ ","
    measure2 = "#"+str(num_measure)+ ","
    if obs_type == "GM":
        world.types.append('')
        if "/" in value:
            obs1 = value.split('/')[0]
            obs2 = value.split('/')[1]
        else:
            obs1 = value
            obs2= ''
        measure += "GM,"+obs1.split('#')[0]+","+obs1.split('#')[1]+","
        measure2 += "GM,"+obs1.split('#')[0]+","
        fill_metadatas(obs2)
        measure += world.measure       
        measure2 += world.measure2       
    elif obs_type == "GC":
        world.types.append('')
        if "/" in value:
            obs1 = value.split('/')[0]
            obs2 = value.split('/')[1]
        else:
            obs1 = value
            obs2= ''
        measure += "GC,"+obs1.split('#')[0]+","
        measure2 += "GC,"+obs1.split('#')[0]+","
        obs1_2 = obs1.split('#')[1]
        if ':' in obs1_2:
            measure += obs1_2.split(':')[0]+","
            measure2 += obs1_2.split(':')[1]+","
        else:
            measure += obs1_2+","
            measure2 += obs1_2+","
        fill_metadatas(obs2)
        measure += world.measure       
        measure2 += world.measure2       
    elif obs_type == "P1":
        world.types.append(obs_type)
        obs1 = value.split('/')[0]
        obs2 = value.split('/')[1]
        measure += str(obs_type)+ ","
        measure2 += str(obs_type)+ ","
        for i in obs1.split('#'):
            measure += i +","
            world.values.append(i)
        fill_metadatas(obs2)
        measure += world.measure       
        measure2 += world.measure2       
    elif obs_type == "B":
        world.types.append('B')
        obs1 = value.split('/')[0]
        obs2 = value.split('/')[1]
        measure += "B,"
        measure2 += "B,"
        for i in obs1.split('#'):
            measure += i +","
            world.values.append(i)
        measure2 += obs1.split('#')[1]+","+obs1.split('#')[4]+","+obs1.split('#')[5]+","
        fill_metadatas(obs2)
        measure += world.measure       
        measure2 += world.measure2       
    elif obs_type == "K1":
        world.types.append(obs_type)
        measure += "K1,"+value.split('&')[0]+"$"+value.split('&')[1]
        measure2 += "K1,"+value.split('&')[0]+"$"+value.split('&')[1]
    elif obs_type == "GPS":
        world.types.append(obs_type)
        obs1 = value.split('/')[0]
        obs2 = value.split('/')[1]
        measure += str(obs_type)+ ","
        measure2 += str(obs_type)+ ","
        for i in obs1.split('#'):
            measure += i +","
        world.metadatas.append("WGS84")
        fill_metadatas(obs2)
        measure += world.measure       
        measure2 += world.measure2       
    world.ts=ts
    world.st=st
    print 'Medida original: ' + measure
    measures.append('cadena='+urllib.quote_plus(measure))
    requests.post(CBROKER_URL+"/reset")
    req = gw.sendMeasure(protocol,"",device_id,measures)
    assert req.ok, 'ERROR: ' + req.text
    print 'Respuesta Esperada: ' + measure2
    print 'Respuesta Obtenida: ' + req.text
    assert measure2 == req.text, 'ERROR: ' + measure2 + ' response expected, ' + req.text + ' received'

@step('I send several measures to the GW with name "([^"]*)", protocol "([^"]*)"')
def send_measures(step, device_id, protocol):
    measures= []
    world.values= []
    world.metadatas= []
    world.types = []
    measure = "#"+device_id
    measure2 = ''
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    num_measure=0
    for measures_dict in step.hashes:
        obs_type = measures_dict['obs_type']
        obs_value = measures_dict['obs_value']
        if obs_type:
            measure += ",#"+str(num_measure)+ ","
        else:
            break
        if obs_type == "GM":
            world.types.append('')
            obs1 = obs_value.split('/')[0]
            obs2 = obs_value.split('/')[1]
            measure += "GM,"+obs1.split('#')[0]+","+obs1.split('#')[1]+","
            fill_metadatas(obs2)
            measure += world.measure       
            if obs1.split('#')[1]:
                if measure2:
                    measure2 += ",#"+str(num_measure)+ ","
                else:
                    measure2 += "#"+str(num_measure)+ ","                
                measure2 += "GM,"+obs1.split('#')[0]+","
                measure2 += world.measure2       
                world.are_measures=True
        elif obs_type == "GC":
            world.types.append('')
            if "/" in obs_value:
                obs1 = obs_value.split('/')[0]
                obs2 = obs_value.split('/')[1]
            else:
                obs1 = obs_value
                obs2= ''
            measure += "GC,"+obs1.split('#')[0]+","
            fill_metadatas(obs2)
            if obs1.split('#')[1]:
                if measure2:
                    measure2 += ",#"+str(num_measure)+ ","
                else:
                    measure2 += "#"+str(num_measure)+ ","                
                measure2 += "GC,"+obs1.split('#')[0]+","
                obs1_2 = obs1.split('#')[1]
                if ':' in obs1_2:
                    measure += obs1_2.split(':')[0]+","
                    measure2 += obs1_2.split(':')[1]+","
                else:
                    measure += obs1_2+","
                    measure2 += obs1_2+","
                measure2 += world.measure2       
            measure += world.measure       
            world.are_measures=True      
        elif obs_type == "P1":
            world.types.append(obs_type)
            obs1 = obs_value.split('/')[0]
            obs2 = obs_value.split('/')[1]
            measure += str(obs_type)+ ","
            for i in obs1.split('#'):
                measure += i +","
                world.values.append(i)
            fill_metadatas(obs2)
            measure += world.measure       
            if obs1.split('#')[3]:
                if measure2:
                    measure2 += ",#"+str(num_measure)+ ","
                else:
                    measure2 += "#"+str(num_measure)+ ","                
                measure2 += str(obs_type)+ ","
                measure2 += world.measure2       
                world.are_measures=True      
        elif obs_type == "B":
            world.types.append('B')
            obs1 = obs_value.split('/')[0]
            obs2 = obs_value.split('/')[1]
            measure += "B,"
            for i in obs1.split('#'):
                measure += i +","
                world.values.append(i)
            fill_metadatas(obs2)
            measure += world.measure       
            if obs1.split('#')[5]:
                if measure2:
                    measure2 += ",#"+str(num_measure)+ ","
                else:
                    measure2 += "#"+str(num_measure)+ ","                
                measure2 += "B,"
                measure2 += obs1.split('#')[1]+","+obs1.split('#')[4]+","+obs1.split('#')[5]+","
                measure2 += world.measure2       
                world.are_measures=True      
        elif obs_type == "K1":
            measure += "K1,"+obs_value.split('&')[0]+"$"+obs_value.split('&')[1]
            if measure2:
                measure2 += ",#"+str(num_measure)+ ","
            else:
                measure2 += "#"+str(num_measure)+ ","                
            measure2 += "K1,"+obs_value.split('&')[0]+"$"+obs_value.split('&')[1]
            world.are_measures=True      
        elif obs_type == "GPS":
            world.types.append(obs_type)
            obs1 = obs_value.split('/')[0]
            obs2 = obs_value.split('/')[1]
            measure += str(obs_type)+ ","
            for i in obs1.split('#'):
                measure += i +","
            world.metadatas.append("WGS84")
            fill_metadatas(obs2)
            measure += world.measure       
            if obs1.split('#')[1]:
                if measure2:
                    measure2 += ",#"+str(num_measure)+ ","
                else:
                    measure2 += "#"+str(num_measure)+ ","                
                measure2 += str(obs_type)+ ","
                measure2 += world.measure2       
                world.are_measures=True      
        num_measure=num_measure+1
    world.ts=ts
    world.st=st
    print 'Medida original: ' + measure
    measures.append('cadena='+urllib.quote_plus(measure))
    requests.post(CBROKER_URL+"/reset")
    req = gw.sendMeasure(protocol,"",device_id,measures)
    #print req.status_code + req.ok
    if world.are_measures:
        assert req.ok, 'ERROR: ' + req.text
        print 'Respuesta Esperada: ' + measure2
        print 'Respuesta Obtenida: ' + req.text
        assert measure2 == req.text, 'ERROR: ' + measure2 + ' response expected, ' + req.text + ' received'
    else:
        assert not req.ok, 'ERROR: ' + req.text
        print req      
    world.req_text=req.text
        
@step('I send a measure to the GW with name "([^"]*)", protocol "([^"]*)", type "([^"]*)", value "([^"]*)" and with wrong field "([^"]*)"')
def send_incorrect_measure(step, device_id, protocol, obs_type, value, field):
    measures= []
    world.are_measures=False                      
    world.values= []
    world.metadatas= []
    world.types = []
    if not field=='stack_id':
        measure = "#"+device_id+","
    else:
        measure = ""
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    num_measure=0
    if not field=='bus_id':
        measure += "#"+str(num_measure)+ ","
        measure2 = "#"+str(num_measure)+ ","
    else:
        measure += "#"
        measure2 = ""
    if obs_type == "GM":
        world.types.append('')
        if "/" in value:
            obs1 = value.split('/')[0]
            obs2 = value.split('/')[1]
        else:
            obs1 = value
            obs2= ''
        measure += "GM,"+obs1.split('#')[0]+","+obs1.split('#')[1]+","
        fill_metadatas(obs2)
        measure += world.measure       
        if len(obs1.split('#')) > 1:
            if obs1.split('#')[1]:
                measure2 += "GM,"+obs1.split('#')[0]+","
                measure2 += world.measure2       
                world.are_measures=True
    elif obs_type == "GC":
        world.types.append('')
        if "/" in value:
            obs1 = value.split('/')[0]
            obs2 = value.split('/')[1]
        else:
            obs1 = value
            obs2= ''
        measure += "GC,"+obs1.split('#')[0]+","
        fill_metadatas(obs2)
        if len(obs1.split('#')) > 1:
            if obs1.split('#')[1]:
                measure2 += "GC,"+obs1.split('#')[0]+","
                obs1_2 = obs1.split('#')[1]
                if ':' in obs1_2:
                    measure += obs1_2.split(':')[0]+","
                    measure2 += obs1_2.split(':')[1]+","
                else:
                    measure += obs1_2+","
                    measure2 += obs1_2+","
                measure2 += world.measure2       
                world.are_measures=True
        measure += world.measure       
    elif obs_type == "P1":
        world.types.append(obs_type)
        obs1 = value.split('/')[0]
        obs2 = value.split('/')[1]
        measure += str(obs_type)+ ","
        for i in obs1.split('#'):
            measure += i +","
            world.values.append(i)
        fill_metadatas(obs2)
        measure += world.measure       
        if len(obs1.split('#')) > 3:
            if obs1.split('#')[3]:
                measure2 += str(obs_type)+ ","
                measure2 += world.measure2       
                world.are_measures=True      
    elif obs_type == "B":
        world.types.append('B')
        obs1 = value.split('/')[0]
        obs2 = value.split('/')[1]
        measure += "B,"
        for i in obs1.split('#'):
            measure += i +","
            world.values.append(i)
        fill_metadatas(obs2)
        measure += world.measure
        if len(obs1.split('#')) > 5:
            if obs1.split('#')[5]:
                measure2 += "B,"
                measure2 += obs1.split('#')[1]+","+obs1.split('#')[4]+","+obs1.split('#')[5]+","
                measure2 += world.measure2       
                world.are_measures=True      
    elif obs_type == "K1":
        world.types.append(obs_type)
        measure += "K1,"+value.split('&')[0]+"$"+value.split('&')[1]
        measure2 += "K1,"+value.split('&')[0]+"$"+value.split('&')[1]
    elif obs_type == "GPS":
        world.types.append(obs_type)
        obs1 = value.split('/')[0]
        obs2 = value.split('/')[1]
        measure += str(obs_type)+ ","
        for i in obs1.split('#'):
            measure += i +","
        world.metadatas.append("WGS84")
        fill_metadatas(obs2)
        measure += world.measure       
        if len(obs1.split('#')) > 1:
            if obs1.split('#')[1]:
                measure2 += str(obs_type)+ ","
                measure2 += world.measure2       
                world.are_measures=True      
    world.ts=ts
    world.st=st
    print 'Medida original: ' + measure
    measures.append('cadena='+urllib.quote_plus(measure))
    requests.post(CBROKER_URL+"/reset")
    req = gw.sendMeasure(protocol,"",device_id,measures)
    if not field == "stack_id": 
        assert req.ok, 'ERROR: ' + str(req)
    else:
        assert not req.ok, 'ERROR: ' + str(req)
    world.req_text=req.text
    if '_id' in field:
        world.are_measures=False


@step('"([^"]*)" measures of asset "([^"]*)" with measures "([^"]*)" are received by context broker')
def check_measures_cbroker(step, num_measures, asset, measures):
    time.sleep(1)
    measures_count =  requests.get(CBROKER_URL+"/countMeasure")
    assert measures_count.text == str(num_measures), 'ERROR: ' + str(num_measures) + ' measures expected, ' + measures_count.text + ' received'
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    typeElement = contextElement['type']
    metadata_value=""
    attr_matches=False
    num_attr=0
    num_metadatas=0
    num_compound=0
    for attr in contextElement['attributes']:
        if attr['name'] == "TimeInstant":
            break
        print 'Atributo numero: ' + str(num_attr) + ' con tipo: ' + str(world.types[num_attr])
        if (not world.types[num_attr]) | (world.types[num_attr] == "GPS"):
            if '&' in measures:
                d = measures.split('&')
                print 'Medidas: ' + str(d)
                d1 = dict([d[num_attr].split(':')])
                print 'Medida ' + str(num_attr) + " : " + str(d1)
            else:
                d1 = dict([measures.split(':')]) 
            measure_name=str(d1.items()[0][0])
            measure_value=str(d1.items()[0][1])
        else:
            if '&' in measures:
                d = measures.split('&')
                print 'Medidas: ' + str(d)
                d1 = d[num_attr]
                print 'Medida ' + str(num_attr) + " : " + str(d1)
            else:
                d1 = measures 
            measure_name=world.types[num_attr]
            measure_value=str(d1)
        if str(measure_name) == attr['name']:
            if  "/" in measure_value:
                print measure_value
                d = measure_value.split('/')
                print d
                if not world.types[num_attr]:
                    measure_value=d[0]
                    print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                    assert attr['value'] == str(measure_value), 'ERROR: value: ' + str(measure_value) + " not found in: " + str(attr)
                    attr_matches = True
                elif world.types[num_attr] == "GPS":
                    measure_value=d[0]
                    print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                    assert attr['value'] == str(measure_value), 'ERROR: value: ' + str(measure_value) + " not found in: " + str(attr)
                    attr_matches = True
                    assert attr['type'] == "coords", 'ERROR: type: coords not found in: ' + str(attr)
                else:
                    for comp in d[0].split('#'):
                        print comp
                        while num_compound < len(world.values):
                            compound_value=world.values[num_compound]
                            compound_matches = False
                            for compound in attr['value']:
                                if compound['value'] == str(compound_value):
                                    print 'Compruebo compound {} y {} en {}'.format(comp,compound_value,compound)
                                    assert compound['name'] == str(comp), 'ERROR: {} not found in {}'.format(comp,compound)
                                    compound_matches = True
                                    num_compound=num_compound+1
                                    break
                            assert compound_matches, 'ERROR: compound: ' + str(compound_value) + " not found in: " + str(attr['value'])
                            break                    
                    assert attr['type'] == "compound", 'ERROR: type: compound not found in: ' + str(attr)
                    attr_matches = True
                num_metadata=0
                if d[1]:
                    for meta in d[1].split('#'):
                        print 'Metadato: ' + str(meta)
                        while num_metadatas < len(world.metadatas):
                            metadata_value=world.metadatas[num_metadatas]
                            print metadata_value
                            for metadata in attr['metadatas']:
                                metadata_matches = False
                                if metadata['value'] == str(metadata_value):
                                    print 'Compruebo metadata {} y {} en {}'.format(meta,metadata_value,metadata)
                                    assert metadata['name'] == str(meta), 'ERROR: {} not found in {}'.format(meta,metadata)
                                    metadata_matches = True
                                    num_metadata=num_metadata+1
                                    num_metadatas=num_metadatas+1
                                    break
                            assert metadata_matches, 'ERROR: metadata: ' + str(metadata_value) + " not found in: " + str(attr['metadatas'])
                            break
            num_attr=num_attr+1
            print 'Compruebo metadata TimeInstant y {} en {}'.format(world.st,attr['metadatas'][num_metadata])
            assert attr['metadatas'][num_metadata]['name'] == "TimeInstant", 'ERROR: ' + str(attr['metadatas'][num_metadata])
            assert functions.check_timestamp(attr['metadatas'][num_metadata]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][num_metadata])
    assert attr_matches, 'ERROR: attribute: ' + str(measure_name) + " not found in: " + str(contextElement['attributes'])
    print 'Compruebo atributo TimeInstant y {} en {}'.format(str(world.st),str(contextElement['attributes'][num_attr]))
    assert contextElement['attributes'][num_attr]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][num_attr])
    assert functions.check_timestamp(contextElement['attributes'][num_attr]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][num_attr])
    asset_name = DEF_ENTITY_TYPE + ':' + asset
    assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
    assert typeElement == DEF_ENTITY_TYPE, 'ERROR: ' + DEF_ENTITY_TYPE + ' type expected, ' + typeElement + ' received'


@step('"([^"]*)" measures of asset "([^"]*)" with measures "([^"]*)" and error "([^"]*)" are received or NOT by context broker')
def check_NOT_measures_cbroker(step, num_measures, asset_name, measures, error):
    time.sleep(1)
    measures_count =  requests.get(CBROKER_URL+"/countMeasure")
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert measures_count.text == str(num_measures), 'ERROR: ' + str(num_measures) + ' measures expected, ' + measures_count.text + ' received'
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + str(req.headers[CBROKER_HEADER])
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    typeElement = contextElement['type']
    assert typeElement == "thing", 'ERROR: ' + str(contextElement)
    if world.are_measures & (num_measures!='0'):
        check_measures_cbroker(step, num_measures, asset_name, measures)
    else:
        if not world.field=="device":
            assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
            print "Measure is NOT received"
        assert error == str(world.req_text), 'ERROR: text error: ' + error + " not found in: " + str(world.req_text)

def fill_metadatas (metadata):    
    world.measure = '$'
    world.measure2 = '$'
    if '&' in metadata:
        meta1 = metadata.split('&')[0]
        meta2 = metadata.split('&')[1]
        if ':' in meta1:
            world.measure = meta1.split(':')[0]+"$"+meta2.split(':')[0]
            world.measure2 = meta1.split(':')[1]+"$"+meta2.split(':')[1]
            if meta1.split(':')[0]:
                world.metadatas.append(meta1.split(':')[0])
            if meta2.split(':')[0]:
                world.metadatas.append(meta2.split(':')[0])
        else:
            world.measure = metadata.split('&')[0]+"$"+metadata.split('&')[1]
            world.measure2 = metadata.split('&')[0]+"$"+metadata.split('&')[1]
            if metadata.split('&')[0]:
                world.metadatas.append(metadata.split('&')[0])
            if metadata.split('&')[1]:
                world.metadatas.append(metadata.split('&')[1])        
