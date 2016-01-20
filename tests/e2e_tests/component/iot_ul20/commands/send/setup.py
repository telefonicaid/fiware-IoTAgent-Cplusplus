from lettuce import step, world
from common.steps import service_created_precond,device_with_commands_created_precond,device_with_endpoint_created_precond,device_with_cmds_entity_values_created_precond,check_status_info,check_wrong_status_info,check_status_entity_info
from iotqautils.cb_utils import CBUtils
from iotqautils.gtwMeasures import Gw_Measures_Utils
from common.gw_configuration import IOT_HOSTNAME,IOT_PORT,PATH_UL20_COMMAND,IOT_SERVER_ROOT,DEF_ENTITY_TYPE,TIMEOUT_COMMAND
from common.functions import Functions
import time,datetime

cb = CBUtils(instance=IOT_HOSTNAME,port=IOT_PORT,path_update=PATH_UL20_COMMAND)
gw = Gw_Measures_Utils(server_root=IOT_SERVER_ROOT)
functions = Functions()
world.device={}
world.thing={}

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
    print "Voy a enviar Comando: " + str(entityData)
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    print st
    req = cb.entity_update(service,entityData)
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
    req = cb.entity_update(service,entityData)
    if field == 'unreachable_dest':
        time.sleep(TIMEOUT_COMMAND)
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    if ("nonexistent" in field):
        world.code = 404
    elif ("wrong" in field):
        world.code = 400
    else:
        world.code = 200
    if field=="nonexistent_service":
        assert not req.ok, 'ERROR: ' + str(req)
        world.req_text=req.text
        world.code = 4000
    else:    
        assert req.ok, 'ERROR: ' + str(req)
        world.req_text=req.json()

@step('I get the command and send the response "([^"]*)" for device "([^"]*)"')
def send_response(step, response, device_id):
    functions.check_command_cbroker(world.device_name, "pending")
#    command = str(device_id)+"@"+str(world.value)
    command = str(device_id)+"@"+str(world.cmd_name)
    if world.value:
        command =command+"|"+str(world.value)
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
        functions.check_command_cbroker(world.device_name, "delivered")
        ts = time.time()
        st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
        world.st=st
        world.ts=ts
        print st
    else:
        functions.check_command_cbroker(world.device_name, "pending")
    if response:
        if not response == 'not_read':
            if 'expired' in response:
                time.sleep(TIMEOUT_COMMAND)
                print "Voy a enviar respuesta del Comando fuera de tiempo: " + str(command)
            else:
                print "Voy a enviar respuesta del Comando dentro de tiempo: " + str(command)
            req = gw.SendCmdResponse("IoTUL2", 'apikey_'+world.service_name, device_id, command, response)
            print str(req.text)
            ts = time.time()
            if 'expired' in response:
                ts=ts-3
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