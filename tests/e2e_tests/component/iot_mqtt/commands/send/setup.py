import time, re, datetime, requests
from common.steps import service_created_precond,device_with_commands_created_precond,check_status_info
from lettuce import step, world
from iotqautils.cbUtils import CBUtils
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps
from common.gw_mqtt_commands import mqtt_command
from threading import Thread
from iotqautils.gtwMeasures import Gw_Measures_Utils
from common.gw_configuration import CBROKER_URL,CBROKER_HEADER,GW_HOSTNAME,IOT_PORT,PATH_MQTT_COMMAND,IOT_SERVER_ROOT,DEF_ENTITY_TYPE

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