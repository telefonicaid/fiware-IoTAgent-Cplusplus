from lettuce import step, world
from iotqautils.cbUtils import CBUtils
from iotqautils.gtwMeasures import Gw_Measures_Utils
from common.gw_configuration import CBROKER_URL,CBROKER_HEADER,GW_HOSTNAME,IOT_PORT,PATH_IOT_COMMAND,IOT_SERVER_ROOT,DEF_ENTITY_TYPE,PATH_UL20_COMMAND,TIMEOUT_COMMAND
from common.user_steps import UserSteps
import requests
import time,datetime

cb = CBUtils(instance=GW_HOSTNAME,port=IOT_PORT,path_update=PATH_IOT_COMMAND)
gw = Gw_Measures_Utils(server_root=IOT_SERVER_ROOT)
user_steps = UserSteps()

@step('a service with name "([^"]*)" and protocol "([^"]*)" created')
def service_created_precond(step, service_name, protocol):
    if protocol:
        world.service_name = service_name
        world.protocol = protocol
        user_steps.service_precond(service_name, protocol)

@step('a device with device id "([^"]*)", device name "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_created_precond(step, device_id, device_name, cmd_name, cmd_value):
    replaces = {
        "#": "|"
    }
    for kreplace in replaces:
        cmd_value = cmd_value.replace(kreplace,replaces[kreplace])
    command=[
             {
              "name": cmd_name,
              "type": 'command',
              "value": cmd_value
              }
             ]
    user_steps.device_precond(device_id, CBROKER_URL+PATH_UL20_COMMAND, command, device_name)
    world.device_name=device_name

@step('a device with device id "([^"]*)", device name "([^"]*)", endpoint "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_with_endpoint_created_precond(step, device_id, device_name, endpoint, cmd_name, cmd_value):
    replaces = {
        "#": "|"
    }
    for kreplace in replaces:
        cmd_value = cmd_value.replace(kreplace,replaces[kreplace])
    command=[
             {
              "name": cmd_name,
              "type": 'command',
              "value": cmd_value
              }
             ]
    if endpoint:
        if not "http://" in endpoint:
            endpoint = CBROKER_URL + endpoint       
    if cmd_value:
        user_steps.device_precond(device_id, endpoint, command, device_name)
        world.device_name=device_name

@step('a device with device id "([^"]*)", entity type "([^"]*)", entity name "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_with_entity_values_created_precond(step, device_id, ent_type, ent_name, cmd_name, cmd_value):
    replaces = {
        "#": "|"
    }
    for kreplace in replaces:
        cmd_value = cmd_value.replace(kreplace,replaces[kreplace])
    command=[
             {
              "name": cmd_name,
              "type": 'command',
              "value": cmd_value
              }
             ]
    user_steps.device_precond(device_id, CBROKER_URL+PATH_UL20_COMMAND, command, ent_name, ent_type)
    world.device_id=device_id

@step('I send a command to the IoTAgent with service "([^"]*)", device "([^"]*)", command "([^"]*)", entity_type "([^"]*)" and value "([^"]*)"')
def send_command(step, service, device_id, cmd_name, entity_type, value):
    entityData= {}
    command = {}
    commands = []
    replaces = {
        "#": "|"
    }
    for kreplace in replaces:
        value = value.replace(kreplace,replaces[kreplace])
    world.cmd_name=cmd_name
    world.value=value
    world.req_text=''
    world.code=''
    command['name'] = str(cmd_name)
    command['type'] = "command"
    command['value'] = str(value)
    commands.append(command)
    if entity_type:
        ent_type = entity_type
    else:
        ent_type = DEF_ENTITY_TYPE
    if not device_id:
            device_id = ent_type + ':' + world.device_id
    entityData['ent_type'] = ent_type
    entityData['ent_pattern'] = "false"
    entityData['ent_id'] = str(device_id)
    entityData['attributes'] = commands
#    print "Espero 60 segundos antes de enviar el comando"
#    time.sleep(60)
    print "Voy a enviar Comando: " + str(entityData)
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    print st
    req = cb.entity_update(service,entityData,'path_'+world.service_name)
    #print req.status_code + req.ok
    assert req.ok, 'ERROR: ' + req.text
    world.req_text=req.json()

@step('I send a wrong command to the IoTAgent with service "([^"]*)", device "([^"]*)", command "([^"]*)", value "([^"]*)" and wrong "([^"]*)"')
def send_wrong_command(step, service, device_id, cmd_name, value, field):
    entityData= {}
    command = {}
    commands = []
    replaces = {
        "#": "|"
    }
    for kreplace in replaces:
        value = value.replace(kreplace,replaces[kreplace])
    world.device_id=device_id
    world.cmd_name=cmd_name
    world.value=value
    world.req_text=''
    command['name'] = str(cmd_name)
    command['type'] = "command"
    command['value'] = str(value)
    commands.append(command)
    entityData['ent_type'] = "thing"
    entityData['ent_pattern'] = "false"
    entityData['ent_id'] = str(device_id)
    entityData['attributes'] = commands
    print "Comando: " + str(entityData)
    req = cb.entity_update(service,entityData,'path_'+world.service_name)
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    if ("nonexistent" in field):
        world.code = 404
    elif ("wrong" in field):
        world.code = 400
#        assert not req.ok, 'ERROR: ' + str(req)
#        world.req_text=req.text
    else:
        world.code = 200
    assert req.ok, 'ERROR: ' + str(req)
    world.req_text=req.json()

@step('I get the command and send the response "([^"]*)" for device "([^"]*)"')
def send_response(step, response, device_id):
    check_command_cbroker(world.device_name, "pending")
#    command = str(world.cmd_name)+"@"+str(world.value)
    command = str(device_id)+"@"+str(world.value)
    if response != 'not_read':
        if not 'measure' in response:
            print "Voy a recuperar los comandos del device: " + str(device_id)
            req = gw.getCommand(world.protocol, 'apikey_'+world.service_name, device_id)
        else:
            print "Voy a enviar medida para recuperar los comandos del device: " + str(device_id)
            measures= []
            measure = {}
            measure['alias'] = 'measure'
            measure['value'] = response
            measures.append(measure)
            req = gw.sendMeasure(world.protocol,'apikey_'+world.service_name,device_id,measures, 'getCmd')
        print str(req.text)
        assert req.ok, 'ERROR: ' + req.text
        assert req.text == command, 'Comando recuperado incorrecto: ' + command + ' esperado ' + str(req.text) + ' recibido' 
        ts = time.time()
        st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
        world.st=st
        world.ts=ts
        print st
    else:
        print "No recupero los comandos del device: " + str(device_id)
    if not response == 'not_read':
        check_command_cbroker(world.device_name, "delivered")
        ts = time.time()
        st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
        world.st=st
        world.ts=ts
        print st
    else:
        check_command_cbroker(world.device_name, "pending")
    if response:
        if not response == 'not_read':
            if 'expired' in response:
                time.sleep(TIMEOUT_COMMAND)
                print "Voy a enviar respuesta del Comando fuera de tiempo: " + str(command)
            else:
                print "Voy a enviar respuesta del Comando dentro de tiempo: " + str(command)
            req = gw.SendCmdResponse("IoTUL2", 'apikey_'+world.service_name, device_id, command, response)
#           req = gw.SendCmdResponse("IoTUL2CmdResp", UL20_APIKEY, device_id, command, response)
            print str(req.text)
            ts = time.time()
            if 'expired' in response:
                ts=ts-2
            st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
            world.st=st
            world.ts=ts
            print st
        else:
            print "No se envia respuesta del Comando: " + str(command)
            time.sleep(TIMEOUT_COMMAND)            
            ts = time.time()
            st = datetime.datetime.utcfromtimestamp(ts-2).strftime('%Y-%m-%dT%H:%M:%S')
            world.st=st
            world.ts=ts-2
            print st
    else:
        print "No se envia respuesta del Comando: " + str(command)
        time.sleep(TIMEOUT_COMMAND)
        ts = time.time()
        st = datetime.datetime.utcfromtimestamp(ts-2).strftime('%Y-%m-%dT%H:%M:%S')
        world.st=st
        world.ts=ts-2
        print st

@step('the command of device "([^"]*)" with response "([^"]*)" and status "([^"]*)" is received by context broker')
def check_status_info(step, asset_name, response, status):
    world.response={}
    if status=='OK':
        check_command_cbroker(asset_name, status, response)
    else:
        if response:
            world.response=status
            check_command_cbroker(asset_name, status)
            check_NOT_command_cbroker(asset_name, response, "Info")

@step('the command of device "([^"]*)" with response "([^"]*)" and status "([^"]*)" is received or NOT by context broker')
def check_wrong_status_info(step, asset_name, response, status):
    world.response={}
    if status != "fail":
        check_command_cbroker(asset_name, status, response)
    else:
        check_NOT_command_cbroker(asset_name, response, "Status")
        check_NOT_command_cbroker(asset_name, response, "Info")
    
@step('the command of device "([^"]*)" with response "([^"]*)", entity_type "([^"]*)" and status "([^"]*)" is received by context broker')
def check_status_entity_info(step, asset, response, entity_type, status):
    if asset:
        asset_name = asset
    else:
        if entity_type:
            asset_name = entity_type + ":" + world.device_id
        else:
            asset_name = DEF_ENTITY_TYPE + ":" + world.device_id
    if entity_type:
        check_command_cbroker(asset_name, status, response, entity_type)
    else:
        check_command_cbroker(asset_name, status, response)

def check_command_cbroker(asset_name, status, response={}, entity_type={}):
    time.sleep(1)
    timeinstant=1
    if response:
        replaces = {
                "#": "|"
        }
        for kreplace in replaces:
            response = response.replace(kreplace,replaces[kreplace])
    req =  requests.get(CBROKER_URL+"/last")
    cmd_name=str(world.cmd_name)+"_status"
    print "Voy a comprobar el STATUS del Comando: " + str(cmd_name)
    resp = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {} en last'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    contextElement = resp['contextElements'][0]
    assetElement = contextElement['id']
    #print 'Dispositivo {}'.format(assetElement)
    valueElement = contextElement['attributes'][0]['value']
    #print '{}.{}'.format(model_name,asset_name)
    nameElement = contextElement['attributes'][0]['name']
    #print 'Metadatas {}'.format(metasElement)
    assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
    typeElement = contextElement['type']
    if entity_type:
        ent_type = entity_type
    else:
        ent_type = DEF_ENTITY_TYPE
    assert typeElement == ent_type, 'ERROR: ' + ent_type + ' type expected, ' + typeElement + ' received'
    print 'TYPE: ' + str(typeElement)
    assert nameElement == cmd_name, 'ERROR: ' + cmd_name + ' name expected, ' + nameElement + ' received'
    assert status in valueElement, 'ERROR: ' + status + ' value expected, ' + valueElement + ' received'
    assert contextElement['attributes'][0]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][0]['metadatas'][0])
    assert check_timestamp(contextElement['attributes'][0]['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0]['metadatas'][0])
    if response:
        cmd_name=str(world.cmd_name)+"_info"
        print "Voy a comprobar el INFO del Comando: " + str(cmd_name)
        valueElement = contextElement['attributes'][1]['value']
        nameElement = contextElement['attributes'][1]['name']
        assert nameElement == cmd_name, 'ERROR: ' + cmd_name + ' name expected, ' + nameElement + ' received'
        assert response in valueElement, 'ERROR: ' + response + ' value expected, ' + valueElement + ' received'
        assert contextElement['attributes'][1]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][1]['metadatas'][0])
        assert check_timestamp(contextElement['attributes'][1]['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][1]['metadatas'][0])
        timeinstant+=1
    nameTime = contextElement['attributes'][timeinstant]['name']
    assert nameTime == "TimeInstant", 'ERROR: ' + "TimeInstant" + ' name expected, ' + nameTime + ' received'
    assert check_timestamp(contextElement['attributes'][timeinstant]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][timeinstant])

def check_NOT_command_cbroker(asset_name, response, cmd_type):
    time.sleep(1)
    if cmd_type == "Status":
        req =  requests.get(CBROKER_URL+"/lastStatus")
    else:
        req =  requests.get(CBROKER_URL+"/lastInfo")
    resp = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
#    assert req.headers[CBROKER_HEADER] == UL20_SERVICE, 'ERROR de Cabecera: ' + str(req.headers[CBROKER_HEADER])
    print 'Compruebo la cabecera {} con valor {} en last{}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER],cmd_type)
    contextElement = resp['contextElements'][0]
    assetElement = contextElement['id']
    #print 'Metadatas {}'.format(metasElement)
    assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
    print "Command is NOT received"
    resp = world.req_text
    if world.code:
        assert resp['errorCode']['code'] == world.code, 'ERROR: code error expected ' + str(world.code) + " received " + str(resp['errorCode']['code'])
        if world.response:
            assert resp['errorCode']['reasonPhrase'] == str(world.response), 'ERROR: text error expected ' + str(world.response) + " received " + resp['errorCode']['reasonPhrase']
        else:
            assert resp['errorCode']['reasonPhrase'] == response, 'ERROR: text error expected ' + response + " received " + resp['errorCode']['reasonPhrase']
    
def check_timestamp (timestamp):
    st = datetime.datetime.utcfromtimestamp(world.ts).strftime('%Y-%m-%dT%H:%M:%S')
    if st in timestamp:
        return True
    else:
        st = datetime.datetime.utcfromtimestamp(world.ts+1).strftime('%Y-%m-%dT%H:%M:%S')
        if st in timestamp:
            return True
        else:
            st = datetime.datetime.utcfromtimestamp(world.ts-1).strftime('%Y-%m-%dT%H:%M:%S')
            if st in timestamp:
                return True
            else:
                return False