import time, re, datetime
from common.steps import service_created_precond,device_with_endpoint_created_precond,check_status_info
from lettuce import step, world
from iotqautils.cb_utils import CBUtils
from common.gw_mqtt_commands import mqtt_command
from threading import Thread
from common.gw_configuration import IOT_HOSTNAME,IOT_PORT,PATH_MQTT_COMMAND,DEF_ENTITY_TYPE

cb = CBUtils(instance=IOT_HOSTNAME,port=IOT_PORT,path_update=PATH_MQTT_COMMAND)
world.device={}
world.thing={}

def envia_comando(service, entityData):
    time.sleep(3)
    print 'sending command ' + str(entityData)
    req = cb.entity_update(service, entityData)
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
        req = cb.entity_update(service, entityData)
        assert req.ok, 'ERROR: ' + req.text
        world.req_text=req.json()

@step('And I wait the pooling period')
def wait_pooling_period(step):
    time.sleep(1)
    apikey='apikey_' + str(world.service_name)
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