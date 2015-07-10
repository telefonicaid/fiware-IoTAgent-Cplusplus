import time, re, datetime, requests
from lettuce import step, world
from iotqautils.cbUtils import CBUtils
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps
from common.gw_mqtt_commands import mqtt_command
from threading import Thread
from iotqautils.gtwMeasures import Gw_Measures_Utils
from common.gw_configuration import CBROKER_URL,CBROKER_HEADER,GW_HOSTNAME,IOT_PORT,PATH_MQTT_COMMAND,IOT_SERVER_ROOT,DEF_ENTITY_TYPE,TIMEOUT_COMMAND

cb = CBUtils(instance=GW_HOSTNAME,port=IOT_PORT,path_update=PATH_MQTT_COMMAND)
api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
gw = Gw_Measures_Utils(server_root=IOT_SERVER_ROOT)
user_steps = UserSteps()

def envia_comando(service, entityData):
    time.sleep(3)
    print 'sending command ' + str(entityData)
    req = cb.entity_update(service, entityData, 'path_'+service)
    assert req.ok, 'ERROR: ' + req.text
    world.req_text=req.json()

# Steps
@step('a service with name "([^"]*)" and protocol "([^"]*)" created')
def service_created_precond(step, service_name, protocol):
    if protocol:
        world.service_name = service_name
        world.protocol = protocol
        user_steps.service_precond(service_name, protocol)

@step('a device with device id "([^"]*)", device name "([^"]*)", endpoint "([^"]*)", protocol "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_with_endpoint_created_precond(step, device_id, device_name, endpoint, protocol, cmd_name, cmd_value):
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
        user_steps.device_precond(device_id, endpoint, protocol, command, device_name)
        world.device_name=device_name
    
@step('I send a command to the IoTAgent with service "([^"]*)", device "([^"]*)", command "([^"]*)", cmd_type "([^"]*)" and value "([^"]*)"')
def send_command(step, service, device_id, cmd_name, cmd_type, value):
    world.cmd_type=cmd_type
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
    ent_type = DEF_ENTITY_TYPE
    if not device_id:
            device_id = ent_type + ':' + world.device_id
    entityData['ent_type'] = ent_type
    entityData['ent_pattern'] = "false"
    entityData['ent_id'] = str(device_id)
    entityData['attributes'] = commands
    print "Voy a enviar Comando: " + str(entityData)
    cmd = re.match( r'push(.*)', cmd_type)
    if cmd:
        th = Thread(target=envia_comando, args=(service,entityData))
        th.setDaemon(True)
        th.start()
        th.join(1)
    else:
        print 'sending pull command ' + str(entityData)
        req = cb.entity_update(service, entityData, 'path_'+service)
        assert req.ok, 'ERROR: ' + req.text
        world.req_text=req.json()

@step('And I wait the pooling period')
def wait_pooling_period(step):
    time.sleep(1)
    apikey='apikey_' + str(world.service_name)
    #get_message_topic = apikey + '/' + world.device_id + '/cmdget' 
    mqtt_command(apikey, world.device_id, world.cmd_type, world.num_commands)
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    print st
    world.st=st
    world.ts=ts

@step('And I wait the command timeout "([^"]*)" for "([^"]*)"')
def wait_timeout_period(step, timeout, cmd_type):
    cmd = re.match( r'(.*)_not_resp', cmd_type)
    if cmd:
        time.sleep(1)
        apikey='apikey_' + str(world.service_name)
        mqtt_command(apikey, world.device_id, cmd_type, world.num_commands)
    else:
        cmd = re.match( r'push(.*)', cmd_type)
        if cmd:
            time.sleep(2)
    time.sleep(int(timeout))
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    print st
    world.st=st
    world.ts=ts
    
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

@step('And I request the command status')
def requests_command_status(step):
    commands = api.get_listCommands(world.service_name, world.asset_name)
    world.command_id = commands.json()['data'][len(commands.json()['data'])-1]['command_id']
    world.command = api.get_command(world.service_name, world.asset_name,world.command_id)
 

@step('Then I receive "([^"]*)" and the parameters "([^"]*)"')
def check_command_result_with_parameters(step, status, params):
    params_replaced = params.replace(':','|')
    assert world.command.json()['data']['status'] == status, 'El comando no se ha enviado correctamente'
    assert params_replaced in world.command.json()['data']['response'], 'No se han recuperado los parametros correctamente'


@step('I request the list of commands detailed')
def request_list_of_comands(step):
    params = {"detailed": 1}
    commands = api.get_listCommands(world.service_name, world.asset_name, {}, params)
    world.command_list = []
#As many as command have been specified will be included in the command list
    for i in range(world.num_commands):
        world.command_list.append(commands.json()['data'][len(commands.json()['data'])-(int(i)+1)])
 
 
@step('Then I receive "([^"]*)"')
def check_command_result(step, status):
    assert world.command.json()['data']['status'] == status, 'El comando no se ha enviado correctamente'

@step('Then I validate "([^"]*)"')
def check_command_status(step, status):
    assert world.command.json()['data']['status'] == status, 'El estado del comando no es correcto: ' + str(world.command.json()['data']['status'])
    
@step('I receive the list of "([^"]*)" with "([^"]*)"')
def check_command_list_results(step, command_list, status):
    for cmd in command_list.split(','):
        command_exists=False
        for i in world.command_list:
            if cmd in i['cmd']:
                command_exists=True
                assert i['status'] == status, 'ERROR: El comando {} no tiene el estado {}'.format(cmd,status)
                break
        assert command_exists, 'ERROR: El comando {} no se encuentra en la lista de comandos'.format(cmd)    

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
    print 'Compruebo la cabecera {} con valor {} en last{}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER],cmd_type)
    contextElement = resp['contextElements'][0]
    assetElement = contextElement['id']
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