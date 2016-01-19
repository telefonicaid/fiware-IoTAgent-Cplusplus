from lettuce import step, world
from iotqatools.iota_utils import Rest_Utils_IoTA
from common.functions import Functions, URLTypes, ProtocolTypes
from common.gw_configuration import CBROKER_URL,CBROKER_URL_TLG,CBROKER_HEADER,CBROKER_PATH_HEADER,IOT_SERVER_ROOT,DEF_ENTITY_TYPE,MANAGER_SERVER_ROOT,SMPP_URL,SMPP_FROM
import time, requests

iotagent = Rest_Utils_IoTA(server_root=IOT_SERVER_ROOT+'/iot')
functions = Functions()
iota_manager = Rest_Utils_IoTA(server_root=MANAGER_SERVER_ROOT+'/iot')

# IoTA STEPS

@step('a Service with name "([^"]*)" and protocol "([^"]*)" created')
def service_created_precond(step, service_name, protocol):
    if protocol:
        world.protocol = protocol
        if protocol == 'IoTModbus':
            functions.service_precond(service_name, protocol, {}, {}, CBROKER_URL_TLG)
        else:
            functions.service_precond(service_name, protocol)

@step('a Service with name "([^"]*)", path "([^"]*)" and protocol "([^"]*)" created')
def service_with_path_created_precond(step, service_name, service_path, protocol):
    world.service_name = service_name
    world.srv_path = service_path
    if (service_name == 'void'):
        return
    world.protocol=protocol
    resource = URLTypes.get(protocol)
    world.resource = resource
    prot = ProtocolTypes.get(protocol)
    world.prot = prot
    world.cbroker= 'http://myurl:80'    
    if (protocol == 'IoTRepsol') | (protocol == 'IoTModbus'):
        apikey = ''      
    else:
        apikey='apikey_' + str(service_name)    
    world.apikey = apikey
    if protocol == 'IoTModbus':
        world.cbroker= CBROKER_URL_TLG    
    functions.service_with_params_precond(service_name, service_path, resource, apikey, world.cbroker)

@step('a Service with name "([^"]*)", protocol "([^"]*)" and atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)" created')
def service_with_attributes_created_precond(step, service_name, protocol, typ1, typ2, name1, name2, type1, type2, value1, value2):
    functions.fill_attributes(typ1, name1, type1, value1, typ2, name2, type2, value2)
    functions.service_precond(service_name, protocol, world.attributes, world.st_attributes)

@step('a Service with name "([^"]*)", path "([^"]*)", resource "([^"]*)" and apikey "([^"]*)" not created')
def service_not_created_precond(step, service_name, service_path, resource, apikey):
    functions.not_service_precond(service_name, service_path, resource, apikey)

@step('a Service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)" and apikey "([^"]*)" not created')
def service_not_created_manager_precond(step, service_name, service_path, protocol, apikey):
    resource = URLTypes.get(protocol)
    if not resource:
        print "No hay que buscar servicio"
        return
    functions.not_service_precond(service_name, service_path, resource, apikey)

@step('a Service with name "([^"]*)", path "([^"]*)", resource "([^"]*)" and apikey "([^"]*)" created')
def service_with_params_precond(step, service_name, service_path, resource, apikey):
    world.srv_path = service_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker= 'http://myurl:80'
    world.entity_type = {}
    world.token = {}
    world.typ1 = {}
    world.typ2 = {}
    world.protocol={}
    functions.service_with_params_precond(service_name, service_path, resource, apikey, world.cbroker)

@step('two Services with name "([^"]*)", paths "([^"]*)" and "([^"]*)", protocols "([^"]*)" and "([^"]*)" and apikey "([^"]*)" created')
def services_with_params_precond(step, service_name, service_path, service_path2, protocol, protocol2, apikey):
    world.apikey = apikey
    world.cbroker= 'http://myurl:80'
    if service_path2:
        world.protocol=protocol2
        world.srv_path2 = service_path2
        resource2 = URLTypes.get(protocol2)
        world.resource2 = resource2
        functions.service_with_params_precond(service_name, service_path2, resource2, apikey, world.cbroker)
    if service_path:
        world.protocol=protocol
        world.srv_path = service_path
        resource = URLTypes.get(protocol)
        world.resource = resource
        functions.service_with_params_precond(service_name, service_path, resource, apikey, world.cbroker)

@step('a Service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)" created')
def service_with_all_params_precond(step, service_name, service_path, resource, apikey, cbroker, entity_type, token):
    world.service_name = service_name
    world.srv_path = service_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    world.typ1 = {}
    world.typ2 = {}
    world.protocol={}
    functions.service_with_params_precond(service_name, service_path, resource, apikey, cbroker, entity_type, token)

@step('a Service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)" and atribute "([^"]*)", with name "([^"]*)", type "([^"]*)" and value "([^"]*)" created')
def service_with_attribute_created_precond(step, service_name, service_path, resource, apikey, cbroker, typ, name, type1, value):
    world.service_name = service_name
    world.srv_path = service_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = {}
    world.token = {}
    world.attributes=[]
    world.st_attributes=[]
    world.protocol={}
    functions.fill_attributes(typ, name, type1, value)
    functions.service_with_params_precond(service_name,service_path,resource,apikey,cbroker,{},{},world.attributes,world.st_attributes)

@step('a Service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)" and apikey "([^"]*)" created')
def service_with_params_manager_precond(step, service_name, service_path, protocol, apikey):
    world.service_name = service_name
    world.srv_path = service_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.protocol = protocol
    world.apikey = apikey
    prot = ProtocolTypes.get(protocol)
    world.prot = prot
    world.cbroker= 'http://myurl:80'
    world.entity_type = {}
    world.token = {}
    world.typ1 = {}
    world.typ2 = {}
    functions.service_with_params_precond(service_name, service_path, resource, apikey, world.cbroker)

@step('a Service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)" created')
def service_with_all_params_manager_precond(step, service_name, service_path, protocol, apikey, cbroker, entity_type, token):
    world.service_name = service_name
    world.srv_path = service_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.protocol = protocol
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    world.typ1 = {}
    world.typ2 = {}
    functions.service_with_params_precond(service_name, service_path, resource, apikey, cbroker, entity_type, token)

@step('a Service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)" and atribute "([^"]*)", with name "([^"]*)", type "([^"]*)" and value "([^"]*)" created')
def service_with_attribute_created_manager_precond(step, service_name, service_path, protocol, apikey, cbroker, typ, name, type1, value):
    world.service_name = service_name
    world.srv_path = service_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.protocol = protocol
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = {}
    world.token = {}
    world.attributes=[]
    world.st_attributes=[]
    functions.fill_attributes(typ, name, type1, value)
    functions.service_with_params_precond(service_name,service_path,resource,apikey,cbroker,{},{},world.attributes,world.st_attributes)

@step('I create a service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)"')
def create_service(step,srv_name,srv_path,resource,apikey,cbroker,entity_type,token):
    world.typ1 = {}
    world.typ2 = {}
    world.srv_path = srv_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    service=functions.create_service_with_params(srv_name,srv_path,resource,apikey,cbroker,entity_type,token)
    assert service.status_code == 201, 'ERROR: ' + service.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio {}'.format(srv_name)

@step('I create a service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)"')
def create_service_manager(step,srv_name,srv_path,protocol,apikey,cbroker,entity_type,token):
    world.typ1 = {}
    world.typ2 = {}
    world.srv_path = srv_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.protocol = protocol
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    service=functions.create_service_with_params(srv_name,srv_path,{},apikey,cbroker,entity_type,token,{},{},protocol)
    assert (service.status_code == 201) | (service.status_code == 200), 'ERROR: ' + service.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio:{} path:{} protocol:{} y apikey:{}'.format(srv_name,srv_path,protocol,apikey)

@step('I create a service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)" and atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)"')
def create_service_with_attrs(step,srv_name,srv_path,resource,apikey,cbroker,typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.srv_path = srv_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = {}
    world.token = {}
    world.typ1 = {}
    world.typ2 = {}
    world.attributes=[]
    world.st_attributes=[]
    world.protocol = {}
    functions.fill_attributes(typ1, name1, type1, value1, typ2, name2, type2, value2)
    service=functions.create_service_with_params(srv_name,srv_path,resource,apikey,cbroker,{},{},world.attributes,world.st_attributes)
    assert service.status_code == 201, 'ERROR: ' + service.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio:{} path:{} resource:{} y apikey:{}'.format(srv_name,srv_path,resource,apikey)

@step('I create a service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)" and atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)"')
def create_service_with_attrs_manager(step, srv_name, srv_path, protocol, apikey, cbroker, typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.srv_path = srv_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.protocol = protocol
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = {}
    world.token = {}
    world.typ1 = {}
    world.typ2 = {}
    world.attributes=[]
    world.st_attributes=[]
    functions.fill_attributes(typ1, name1, type1, value1, typ2, name2, type2, value2)
    service=functions.create_service_with_params(srv_name,srv_path,{},apikey,cbroker,{},{},world.attributes,world.st_attributes,protocol)
    assert (service.status_code == 201) | (service.status_code == 200), 'ERROR: ' + service.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio:{} path:{} protocol:{} y apikey:{}'.format(srv_name,srv_path,protocol,apikey)

@step('the Service with name "([^"]*)" and path "([^"]*)" is created')
def service_created(step, service_name, service_path):
    functions.get_service_created(service_name, service_path, world.resource)
    functions.check_service_data(1)

@step('I update the attribute "([^"]*)" of service "([^"]*)" with value "([^"]*)"')
def update_service_data(step, attribute, service_name, value):
    service=functions.update_service_with_params(attribute, service_name, value, world.srv_path, world.resource, world.apikey)
    assert service.status_code == 204, 'ERROR: ' + service.text + "El servicio {} no se ha actualizado correctamente".format(service_name)
    print 'Se ha actualizado el servicio:{} path:{} resource:{} y apikey:{}'.format(service_name,world.srv_path,world.resource,world.apikey)

@step('I update in manager the attribute "([^"]*)" of service "([^"]*)" with value "([^"]*)"')
def update_service_data_manager(step, attribute, service_name, value):
    service=functions.update_service_with_params(attribute, service_name, value, world.srv_path, {}, world.apikey, False, True, world.protocol)
    assert (service.status_code == 204) | (service.status_code == 200), 'ERROR: ' + service.text + "El servicio {} no se ha actualizado correctamente".format(service_name)
    print 'Se ha actualizado el servicio:{} path:{} protocol:{} y apikey:{}'.format(service_name,world.srv_path,world.protocol,world.apikey)

@step('I retrieve the service data of "([^"]*)", path "([^"]*)", resource "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_service_data(step,service_name, service_path, resource, limit, offset):
    req = functions.get_service_created(service_name, service_path, resource, limit, offset)
    assert req.ok, 'ERROR: ' + req.text

@step('I retrieve the service data of "([^"]*)", path "([^"]*)", protocol "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_service_data_manager(step,service_name, service_path, protocol, limit, offset):
    req = functions.get_service_created(service_name, service_path, {}, limit, offset, protocol, True)
    assert req.ok, 'ERROR: ' + req.text

@step('I delete the service "([^"]*)" with path "([^"]*)"')
def delete_service_data(step, service_name, service_path):
    world.service_path_del = service_path
    req = functions.delete_service_data(service_name, service_path, world.resource, world.apikey)
    assert req.status_code == 204, 'ERROR: ' + req.text + "El servicio {} no se ha borrado correctamente".format(service_name)
    print 'Se ha borrado el servicio:{} path:{} resource:{} y apikey:{}'.format(service_name,service_path,world.resource,world.apikey)

@step('I delete the service "([^"]*)" with param "([^"]*)" and path "([^"]*)"')
def delete_service_with_device(step, service_name, device, service_path):
    world.service_path_del = service_path
    req = functions.delete_service_data(service_name, service_path, world.resource, world.apikey, device)
    assert req.status_code == 204, 'ERROR: ' + req.text + "El servicio {} no se ha borrado correctamente".format(service_name)
    print 'Se ha borrado el servicio:{} path:{} resource:{} y apikey:{}'.format(service_name,service_path,world.resource,world.apikey)
    
@step('I receive the service data of "([^"]*)" services')
def check_service_data(step, num_services):
    functions.check_service_data(num_services)

@step('the service data NOT contains attribute "([^"]*)" with value "([^"]*)"')
def check_NOT_service_data(step, attribute, value):
    if (world.req.status_code == 400) | (world.req.status_code == 404):
        print 'No se comprueba el servicio'
        return
    functions.get_service_created(world.service_name, world.srv_path, world.resource)
    functions.check_NOT_service_data(attribute, value)

@step('the service data contains attribute "([^"]*)" with value "([^"]*)"')
def check_service_data_updated(step, attribute, value):
    if attribute == 'resource':
        resource = value
    else:
        resource= world.resource    
    functions.get_service_created(world.service_name, world.srv_path, resource)
    functions.check_service_data(1, attribute, value)

@step('the service data of manager contains attribute "([^"]*)" with value "([^"]*)"')
def check_service_data_updated_manager(step, attribute, value):
    if attribute == 'protocol':
        resource = URLTypes.get(value)
    else:
        resource = URLTypes.get(world.protocol)
    functions.get_service_created(world.service_name, world.srv_path, resource)
    functions.check_service_data(1, attribute, value)

@step('the Services with name "([^"]*)" and paths "([^"]*)" and "([^"]*)" are deleted or not')
def check_services_data_deleted(step, service_name, service_path, service_path2):
    if service_path == 'true':
        assert not functions.check_service_created(service_name, world.srv_path, world.resource, True)
    else:
        assert functions.check_service_created(service_name, world.srv_path, world.resource)       
    if service_path2:
        if service_path2 == 'true':
            assert not functions.check_service_created(service_name, world.srv_path2, world.resource2, True)
        else:
            assert functions.check_service_created(service_name, world.srv_path2, world.resource2)       

@step('the Service with name "([^"]*)" and path "([^"]*)" is not deleted')
def check_service_data_deleted(step, service_name, service_path):
    if service_name != 'void':
        if '/' in service_path:
            if world.resource:
                assert functions.check_service_created(service_name, service_path, world.resource)
            else:
                assert functions.check_service_created(service_name, service_path)
                
@step('a Device with name "([^"]*)" and protocol "([^"]*)" created')    
def device_created_precond(step, device_name, protocol):
    functions.device_precond(device_name, {}, protocol)

@step('devices for services with name "([^"]*)", paths "([^"]*)" and "([^"]*)" and protocols "([^"]*)" and "([^"]*)" created')
def devices_created_precond(step, service_name, service_path, service_path2, protocol, protocol2):
    world.device_name={}
    world.device_name2={}
    if service_path:
        device_name='device1'
        functions.device_of_service_precond(service_name, service_path, device_name, {}, {}, {}, {}, {}, {}, protocol)
        world.device_name=device_name
    if service_path2:
        device_name2='device2'
        functions.device_of_service_precond(service_name, service_path2, device_name2, {}, {}, {}, {}, {}, {}, protocol2)
        world.device_name2=device_name2

@step('a Device with name "([^"]*)" and path "([^"]*)" not created')
def device_not_created_precond(step, device_name, service_path):
    world.srv_path=service_path
    if (world.service_name == 'void'):
        if  (not '/' in service_path) and (not service_path=='void'):
            return
    functions.not_device_precond(device_name)

@step('a Device with name "([^"]*)", protocol "([^"]*)", entity type "([^"]*)" and entity name "([^"]*)" created')
def device_with_entity_values_created_precond(step, device_id, protocol, ent_type, ent_name):
    if not ent_type=="fail":
        functions.device_precond(device_id, {}, protocol, {}, ent_name, ent_type)
    else:
        world.device_id=device_id

@step('a Device with name "([^"]*)", protocol "([^"]*)", atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)" created')
def device_with_attributes_created_precond(step, device_id, protocol, typ1, typ2, name1, name2, type1, type2, value1, value2):
    functions.fill_attributes(typ1, name1, type1, value1, typ2, name2, type2, value2, False)
    functions.device_precond(device_id, {}, protocol, {}, {}, {}, world.attributes, world.st_attributes)

@step('a Device with name "([^"]*)", protocol "([^"]*)", entity_name "([^"]*)" and entity_type "([^"]*)" created')
def device_with_entity_values_of_service_precond(step, device_id, protocol, entity_name, entity_type):
    world.typ1 = {}
    world.typ2 = {}
    world.endpoint = {}
    world.entity_name = entity_name
    world.entity_type = entity_type
    world.protocol=protocol
    functions.device_of_service_precond(world.service_name, world.srv_path, device_id, {}, {}, entity_name, entity_type, {}, {}, protocol)

@step('a Device with name "([^"]*)", entity_name "([^"]*)", entity_type "([^"]*)", endpoint "([^"]*)", protocol "([^"]*)" and atribute or command "([^"]*)", with name "([^"]*)", type "([^"]*)" and value "([^"]*)" created')
def device_with_attr_or_cmd_created_precond(step, device_id, entity_name, entity_type, endpoint, protocol, typ, name, type1, value):
    world.entity_name = entity_name
    world.entity_type = entity_type
    world.endpoint = endpoint
    world.protocol=protocol
    functions.fill_attributes(typ, name, type1, value, {}, {}, {}, {}, False)
    functions.device_of_service_precond(world.service_name, world.srv_path, device_id, endpoint, world.commands, entity_name, entity_type, world.attributes, world.st_attributes, protocol)

@step('a Device with id "([^"]*)", name "([^"]*)", protocol "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_with_commands_created_precond(step, device_id, device_name, protocol, cmd_name, cmd_value):
    functions.device_with_commands_precond(device_id, device_name, protocol, cmd_name, cmd_value, {}, {})

@step('a Device with id "([^"]*)", name "([^"]*)", endpoint "([^"]*)", protocol "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_with_endpoint_created_precond(step, device_id, device_name, endpoint, protocol, cmd_name, cmd_value):
    functions.device_with_commands_precond(device_id, device_name, protocol, cmd_name, cmd_value, endpoint, {})

@step('a Device with id "([^"]*)", entity type "([^"]*)", entity name "([^"]*)", protocol "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_with_cmds_entity_values_created_precond(step, device_id, ent_type, ent_name, protocol, cmd_name, cmd_value):
    functions.device_with_commands_precond(device_id, ent_name, protocol, cmd_name, cmd_value, {}, ent_type)

@step('I create a Device with name "([^"]*)", entity_name "([^"]*)", entity_type "([^"]*)", endpoint "([^"]*)" and protocol "([^"]*)"')
def create_device(step, dev_name, entity_name, entity_type, endpoint, protocol):
    world.typ1 = {}
    world.typ2 = {}
    world.entity_name = entity_name
    world.entity_type = entity_type
    world.endpoint = endpoint
    req=functions.create_device(world.service_name, world.srv_path, dev_name, endpoint, {}, entity_name, entity_type, {}, {}, protocol)
    assert req.status_code == 201, 'ERROR: ' + req.text + "El device {} no se ha creado correctamente".format(dev_name)
    assert req.headers['Location'] == "/iot/devices/"+str(dev_name), 'ERROR de Cabecera: /iot/devices/' + str(dev_name) + ' esperada ' + str(req.headers['Location']) + ' recibida'
    print 'Se ha creado el device {}'.format(dev_name)

@step('I create in manager a Device with name "([^"]*)", entity_name "([^"]*)", entity_type "([^"]*)", endpoint "([^"]*)" and protocol "([^"]*)"')
def create_device_manager(step, dev_name, entity_name, entity_type, endpoint, protocol):
    world.typ1 = {}
    world.typ2 = {}
    world.entity_name = entity_name
    world.entity_type = entity_type
    world.endpoint = endpoint
    req=functions.create_device(world.service_name, world.srv_path, dev_name, endpoint, {}, entity_name, entity_type, {}, {}, protocol, True)
    assert req.status_code == 201, 'ERROR: ' + req.text + "El device {} no se ha creado correctamente".format(dev_name)
    print 'Se ha creado el device {}'.format(dev_name)

@step('I create a Device with name "([^"]*)", protocol "([^"]*)", atributes and/or commands "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)"')
def create_device_with_attrs_cmds(step, dev_name, protocol, typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.entity_name = {}
    world.entity_type = {}
    world.endpoint = {}
    world.commands=[]
    world.attributes=[]
    world.st_attributes=[]
    functions.fill_attributes(typ1, name1, type1, value1, typ2, name2, type2, value2, False)
    req=functions.create_device(world.service_name, world.srv_path, dev_name, {}, world.commands, {}, {}, world.attributes, world.st_attributes, protocol)
    assert req.status_code == 201, 'ERROR: ' + req.text + "El device {} no se ha creado correctamente".format(dev_name)
    assert req.headers['Location'] == "/iot/devices/"+str(dev_name), 'ERROR de Cabecera: /iot/devices/' + str(dev_name) + ' esperada ' + str(req.headers['Location']) + ' recibida'
    print 'Se ha creado el device {}'.format(dev_name)

@step('I create in manager a Device with name "([^"]*)", protocol "([^"]*)", atributes and/or commands "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)"')
def create_device_with_attrs_cmds_manager(step, dev_name, protocol, typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.entity_name = {}
    world.entity_type = {}
    world.endpoint = {}
    world.commands=[]
    world.attributes=[]
    world.st_attributes=[]
    functions.fill_attributes(typ1, name1, type1, value1, typ2, name2, type2, value2, False)
    req=functions.create_device(world.service_name, world.srv_path, dev_name, {}, world.commands, {}, {}, world.attributes, world.st_attributes, protocol, True)
    assert req.status_code == 201, 'ERROR: ' + req.text + "El device {} no se ha creado correctamente".format(dev_name)
    print 'Se ha creado el device {}'.format(dev_name)

@step('I update the attribute "([^"]*)" of device "([^"]*)" with value "([^"]*)"')
def update_device_data(step, attribute, device_name, value):
    device=functions.update_device_with_params(attribute, device_name, value, world.service_name, world.srv_path)
    assert device.status_code == 204, 'ERROR: ' + device.text + "El device {} no se ha actualizado correctamente".format(device_name)
    print 'Se ha actualizado el device:{} del servicio:{} path:{} resource:{} y apikey:{}'.format(device_name, world.service_name,world.srv_path,world.resource,world.apikey)

@step('I update in manager the attribute "([^"]*)" of device "([^"]*)" with value "([^"]*)"')
def update_device_data_manager(step, attribute, device_name, value):
    device=functions.update_device_with_params(attribute, device_name, value, world.service_name, world.srv_path, False, True, world.protocol)
    assert (device.status_code == 204) | (device.status_code == 200), 'ERROR: ' + device.text + "El device {} no se ha actualizado correctamente".format(device_name)
    print 'Se ha actualizado el device:{} del servicio:{} path:{} resource:{} y apikey:{}'.format(device_name, world.service_name,world.srv_path,world.resource,world.apikey)

@step('I retrieve the device data of "([^"]*)"')
def get_device_data(step, dev_name):
    world.manager=False
    req=functions.get_device_created(world.service_name, world.srv_path, dev_name)
    assert req.ok, 'ERROR: ' + req.text

@step('I retrieve in manager the device data of "([^"]*)" with protocol "([^"]*)"')
def get_device_data_manager(step, dev_name, protocol):
    world.manager=True
    req=functions.get_device_created(world.service_name, world.srv_path, dev_name, protocol, True)
    assert req.ok, 'ERROR: ' + req.text

@step('I list the devices of "([^"]*)", path "([^"]*)", entity "([^"]*)", protocol "([^"]*)", detailed "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_devices_list(step, service_name, service_path, entity, protocol, detailed, limit, offset):
    world.manager=False
    world.detailed = detailed
    req=functions.get_devices_created(service_name, service_path, entity, limit, offset, detailed, protocol)
    assert req.ok, 'ERROR: ' + req.text

@step('I list in manager the devices of "([^"]*)", path "([^"]*)", entity "([^"]*)", protocol "([^"]*)", detailed "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_devices_list_manager(step, service_name, service_path, entity, protocol, detailed, limit, offset):
    world.manager=True
    world.detailed = detailed
    req=functions.get_devices_created(service_name, service_path, entity, limit, offset, detailed, protocol, True)
    assert req.ok, 'ERROR: ' + req.text

@step('I delete the device "([^"]*)"')
def delete_device_data(step, device_name):
    device = functions.delete_device_data(device_name, world.service_name, world.srv_path)
    assert device.status_code == 204, 'ERROR: ' + device.text + "El device {} no se ha borrado correctamente".format(device_name)
    print 'Se ha borrado el device:{} del servicio:{} path:{} resource:{} y apikey:{}'.format(device_name, world.service_name,world.srv_path,world.resource,world.apikey)

@step('I delete in manager the device "([^"]*)"')
def delete_device_data_manager(step, device_name):
    device = functions.delete_device_data(device_name, world.service_name, world.srv_path, True, world.protocol)
    assert (device.status_code == 204) | (device.status_code == 200), 'ERROR: ' + device.text + "El device {} no se ha borrado correctamente".format(device_name)
    print 'Se ha borrado el device:{} del servicio:{} path:{} resource:{} y apikey:{}'.format(device_name, world.service_name,world.srv_path,world.resource,world.apikey)

@step('I receive the device data of "([^"]*)"')
def check_device_data(step, dev_name):
    functions.check_device_data(dev_name, world.manager)

@step('I receive the device data of "([^"]*)" devices with data "([^"]*)"')
def check_devices_data(step, num_devices, data):
    functions.check_devices_data(num_devices, data)

@step('the device data contains attribute "([^"]*)" with value "([^"]*)"')
def check_device_data_updated(step, attribute, value):
    functions.get_device_created(world.service_name, world.srv_path, world.device_id)
    functions.check_device_data(world.device_id, False, 'on', attribute, value)

@step('the Device with name "([^"]*)" is created')
def device_created(step, dev_name):
    functions.get_device_created(world.service_name, world.srv_path, dev_name)
    functions.check_device_data(dev_name)

@step('the Device with name "([^"]*)" is deleted')
def check_device_data_deleted(step, device_name):
    assert not functions.check_device_created(world.service_name, device_name, world.srv_path, device_name==world.device_id)
    
@step('devices "([^"]*)" of services with name "([^"]*)" and paths "([^"]*)" and "([^"]*)" are deleted or not')
def check_devices_data_deleted(step, devices, service_name, service_path, service_path2):
    if (service_path == 'true') & (devices=='true'):
        assert not functions.check_device_created(service_name, world.device_name, world.srv_path, True)
    else:
        assert functions.check_device_created(service_name, world.device_name, world.srv_path)       
    if service_path2:
        if (service_path2 == 'true') & (devices=='true'):
            assert not functions.check_device_created(service_name, world.device_name2, world.srv_path2, True)
        else:
            assert functions.check_device_created(service_name, world.device_name2, world.srv_path2)

@step('the measure of asset "([^"]*)" with measures "([^"]*)" is received by context broker')
def check_measure_cbroker(step, asset_name, measures):
    functions.check_measure(asset_name, measures)

@step('the measure of asset "([^"]*)" with measures "([^"]*)" and timestamp "([^"]*)" is received by context broker')
def check_measure_cbroker_timestamp(step, asset_name, measures, timestamp):
    functions.check_measure(asset_name, measures, timestamp)

@step('the measure of asset "([^"]*)" with entity_type "([^"]*)", entity_name "([^"]*)" and measures "([^"]*)" is received by context broker')
def check_measure_cbroker_entity(step, asset_name, entity_type, entity_name, measures):
    functions.check_measure(asset_name, measures, {}, entity_type, entity_name)

@step('the measure of asset "([^"]*)" with measures "([^"]*)" and attributes are received by context broker')
def check_measure_cbroker_with_attributes(step, asset_name, measures):
    functions.check_measure(asset_name, measures, {}, {}, {}, True)

@step('"([^"]*)" measures of asset "([^"]*)" are received by context broker')
def check_measures_cbroker(step, num_measures, asset_name):
    check_measures(step, num_measures, asset_name)

@step('"([^"]*)" measures of asset "([^"]*)" with timestamp "([^"]*)" are received by context broker')
def check_measures_cbroker_timestamp(step, num_measures, asset_name, timestamp):
    if not asset_name:
        asset_name=world.device_id
    if world.entity_type:
        world.device={}
        world.thing=world.entity_type
    if world.entity_name:
        world.device={}
    print asset_name
    check_measures(step, num_measures, asset_name, timestamp)

def check_measures(step, measures, asset_name, timestamp={}):
    time.sleep(2)
    if world.protocol == 'IoTModbus':
        cbroker_url = CBROKER_URL_TLG
    else:
        cbroker_url = CBROKER_URL
    measures_count =  requests.get(cbroker_url+"/countMeasure")
    num_measures = measures.split('/')[0]
    assert measures_count.text == str(num_measures), 'ERROR: ' + str(num_measures) + ' measures expected, ' + measures_count.text + ' received'
    req =  requests.get(cbroker_url+"/last")
    response = req.json()
    if len(measures.split('/'))>1:
        assert str(len(response['contextElements'])) == measures.split('/')[1], 'ERROR: ' + str(measures.split('/')[1]) + ' contexElements expected, ' + str(len(response['contextElements'])) + ' received'
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    for measures_dict in step.hashes:
        measures = measures_dict['generated_measures']
        count_measure=0
        for j in measures.split('@'):
            if j:
                for i in j.split('#'):
                    if i:
                        d = dict([i.split(':')])
                    else:
                        break 
                    for contextElement in response['contextElements']:
                        assetElement = contextElement['id']
                        typeElement = contextElement['type']
                        measure_name=str(d.items()[0][0])
                        measure_value=str(d.items()[0][1])
                        metadata_value=""
                        if  "/" in measure_value:
                            if not measure_name=='l':
                                d2 = dict([measure_value.split('/')])
                                measure_value=str(d2.items()[0][0])
                                metadata_value=str(d2.items()[0][1])
                        attr_matches=False
                        for attr in contextElement['attributes']:
                            if str(measure_name) == attr['name']:
                                if str(attr['value']) == str(measure_value):
                                    print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                                    attr_matches=True
                                    if metadata_value:
                                        assert attr['metadatas'][1]['name'] == "uom", 'ERROR: ' + str(attr['metadatas'][1])
                                        assert str(metadata_value) in attr['metadatas'][1]['value'], 'ERROR: metadata: ' + str(metadata_value) + " not found in: " + str(attr['metadatas'][1])
                                    assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(attr['metadatas'][0])
                                    if timestamp:
                                        if len(timestamp.split('#'))>1:
                                            print timestamp.split('#')[count_measure]
                                            timest=timestamp.split('#')[count_measure]
                                        else:
                                            timest=timestamp.split('#')[0]
#                                        assert str(timest) == attr['value'], 'ERROR: timestamp: ' + str(timest) + " not found in: " + str(attr)
                                        assert str(timest) == attr['metadatas'][0]['value'], 'ERROR: metadata: ' + str(timest) + " not found in: " + str(attr['metadatas'][0])
                                    else:
                                        assert functions.check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                                    break
                        if attr_matches:
                            break
                    if len(step.hashes)<=2:
                        assert attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " not found in: " + str(contextElement['attributes'])
                    is_timestamp=False
                if attr_matches:
                    for attr in contextElement['attributes']:
                        if attr ['name'] == "TimeInstant":
                            print 'Compruebo atributo TimeInstant y {} en {}'.format(attr['value'],str(attr))
                            if timestamp:
                                if len(timestamp.split('#'))>1:
                                    print timestamp.split('#')[count_measure]
                                    timest=timestamp.split('#')[count_measure]
                                else:
                                    timest=timestamp.split('#')[0]
                                assert str(timest) == attr['value'], 'ERROR: timestamp: ' + str(timest) + " not found in: " + str(attr)
                            else:
                                assert functions.check_timestamp(str(attr['value'])), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
                            is_timestamp=True
                            break
                    assert is_timestamp, 'ERROR: TimeInstant not found in' + str(contextElement['attributes'])
                    device_name=asset_name
                    if world.def_entity:
                        device_name = DEF_ENTITY_TYPE + ':' + asset_name
                        world.thing = DEF_ENTITY_TYPE
                    else:
                        if world.entity_type:
                            if not world.entity_name:
                                device_name = world.thing + ':' + asset_name
                    if world.device:
                        device_name = world.device + '.' + asset_name
                    assert assetElement == "{}".format(device_name), 'ERROR: id: ' + str(device_name) + " not found in: " + str(contextElement)
                    assert typeElement == "{}".format(world.thing), 'ERROR: type: ' + str(world.thing) + " not found in: " + str(contextElement)
                count_measure+=1
            else:
                break
        if (len(step.hashes)>2) & (attr_matches):
            assert attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " not found in: " + str(contextElement['attributes'])
            break

@step('the measure of asset "([^"]*)" with measures "([^"]*)" is received or NOT by context broker')
def check_NOT_measure_cbroker(step, asset_name, measures):
    time.sleep(1)
    if world.protocol == 'IoTModbus':
        cbroker_url = CBROKER_URL_TLG
    else:
        cbroker_url = CBROKER_URL
    req =  requests.get(cbroker_url+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    typeElement = contextElement['type']
    if (world.field == "timestamp") | (world.field == "sens_type") | (world.field == "payload"):
        device_name=asset_name
        if world.def_entity:
            device_name = DEF_ENTITY_TYPE + ':' + asset_name
            world.thing = DEF_ENTITY_TYPE
        if world.device:
            device_name = world.device + '.' + asset_name
        assert assetElement == "{}".format(device_name), 'ERROR: id: ' + str(device_name) + " not found in: " + str(contextElement)
        assert typeElement == "{}".format(world.thing), 'ERROR: type: ' + str(world.thing) + " not found in: " + str(contextElement)
        if measures:
            for i in measures.split('#'):
                d = dict([i.split(':')]) 
                measure_name=str(d.items()[0][0])
                measure_value=str(d.items()[0][1])
                metadata_value=""
                if  "/" in measure_value:
                    d2 = dict([measure_value.split('/')])
                    measure_value=str(d2.items()[0][0])
                    metadata_value=str(d2.items()[0][1])
                attr_matches=False
                for attr in contextElement['attributes']:
                    if str(measure_name) == attr['name']:
                        if str(measure_value) == "void":
                            measure_value = ' '
                        print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                        attr_matches=True
                        assert attr['value'] == str(measure_value), 'ERROR: value: ' + str(measure_value) + " not found in: " + str(attr)
                        if metadata_value:
                            assert attr['metadatas'][1]['name'] == "uom", 'ERROR: ' + str(attr['metadatas'][1])
                            assert str(metadata_value) in attr['metadatas'][1]['value'], 'ERROR: metadata: ' + str(metadata_value) + " not found in: " + str(attr['metadatas'][1])
                        assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(attr['metadatas'][0])
                        if (world.field == "timestamp") | (world.field == "payload"):
                            assert functions.check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])   
                        else:
                            assert str(world.st) == attr['metadatas'][0]['value'], 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                        break
                assert attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " not found in: " + str(contextElement['attributes'])
#        if world.field == "payload":
#            assert len(contextElement['attributes']) == 1, 'ERROR: Atribute ' + str(contextElement['attributes'][0]) + " found in: " + str(contextElement['attributes'])
        is_timestamp=False
        for attr in contextElement['attributes']:
            if attr ['name'] == "TimeInstant":
                print 'Compruebo atributo TimeInstant y {} en {}'.format(attr['value'],str(attr))
                if (world.field == "timestamp") | (world.field == "payload"):
                    assert functions.check_timestamp(str(attr['value'])), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
                else:
                    assert str(world.st) == attr['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
                is_timestamp=True
                break
        assert is_timestamp, 'ERROR: TimeInstant not found in' + str(contextElement['attributes'])
    else:
        assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
        print "Measure is NOT received"

@step('"([^"]*)" measures of asset "([^"]*)" are received or NOT by context broker')
def check_NOT_measures_cbroker(step, num_measures, asset_name):
    check_NOT_measures(step, num_measures, asset_name)
    
@step('([^"]*)" measures of asset "([^"]*)" with timestamp "([^"]*)" are received or NOT by context broker')
def check_NOT_measures_cbroker_timestamp(step, num_measures, asset_name, timestamp):
    check_NOT_measures(step, num_measures, asset_name, timestamp)

def check_NOT_measures(step, num_measures, asset_name, timestamp={}):
    time.sleep(1)
    if world.protocol == 'IoTModbus':
        cbroker_url = CBROKER_URL_TLG
    else:
        cbroker_url = CBROKER_URL
    measures_count =  requests.get(cbroker_url+"/countMeasure")
    assert measures_count.text == str(num_measures), 'ERROR: ' + str(num_measures) + ' measures expected, ' + measures_count.text + ' received'
    req =  requests.get(cbroker_url+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    typeElement = contextElement['type']
    if int(num_measures)>0:
        measures_dict=step.hashes[0]
        measures = measures_dict['generated_measures']
        for j in measures.split('@'):
            if j:
                for i in j.split('#'):
                    if i:
                        d = dict([i.split(':')])
                    else:
                        break 
                    for contextElement in response['contextElements']:
                        assetElement = contextElement['id']
                        typeElement = contextElement['type']
                        measure_name=str(d.items()[0][0])
                        measure_value=str(d.items()[0][1])
                        metadata_value=""
                        if  "/" in measure_value:
                            d2 = dict([measure_value.split('/')])
                            measure_value=str(d2.items()[0][0])
                            metadata_value=str(d2.items()[0][1])
                        attr_matches=False
                        for attr in contextElement['attributes']:
                            if str(measure_name) == attr['name']:
                                print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                                if str(attr['value']) == str(measure_value):
                                    attr_matches=True
                                    if metadata_value:
                                        assert attr['metadatas'][1]['name'] == "uom", 'ERROR: ' + str(attr['metadatas'][1])
                                        assert str(metadata_value) in attr['metadatas'][1]['value'], 'ERROR: metadata: ' + str(metadata_value) + " not found in: " + str(attr['metadatas'][1])
                                    assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(attr['metadatas'][0])
                                    if not timestamp:
                                        timestamp=world.st
#                                        assert functions.check_timestamp(str(attr['metadatas'][0]['value'])), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])   
#                                    else:
                                        assert str(timestamp) == attr['metadatas'][0]['value'], 'ERROR: metadata: ' + str(timestamp) + " not found in: " + str(attr['metadatas'][0])
                                    break
                        if attr_matches:
                            break
                    assert attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " not found in: " + str(contextElement['attributes'])
                    is_timestamp=False                   
                if attr_matches:
                    for attr in contextElement['attributes']:
                        if attr ['name'] == "TimeInstant":
                            print 'Compruebo atributo TimeInstant y {} en {}'.format(attr['value'],str(attr))
                            if not timestamp:
                                timestamp=world.st
#                                assert functions.check_timestamp(str(attr['value'])), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])   
#                            else:
                                assert str(timestamp) == attr['value'], 'ERROR: timestamp: ' + str(timestamp) + " not found in: " + str(attr)
                            is_timestamp=True
                            break
                    assert is_timestamp, 'ERROR: TimeInstant not found in' + str(contextElement['attributes'])
            device_name=asset_name
            if world.def_entity:
                device_name = DEF_ENTITY_TYPE + ':' + asset_name
                world.thing = DEF_ENTITY_TYPE
            if world.device:
                device_name = world.device + '.' + asset_name
            assert assetElement == "{}".format(device_name), 'ERROR: id: ' + str(device_name) + " not found in: " + str(contextElement)
            assert typeElement == "{}".format(world.thing), 'ERROR: type: ' + str(world.thing) + " not found in: " + str(contextElement)
        if len(step.hashes)==2:
            measures_dict=step.hashes[1]
            measures = measures_dict['generated_measures']
            print measures
            for i in measures.split('#'):
                d = dict([i.split(':')]) 
                for contextElement in response['contextElements']:
                    assetElement = contextElement['id']
                    typeElement = contextElement['type']
                    measure_name=str(d.items()[0][0])
                    measure_value=str(d.items()[0][1])
                    metadata_value=""
                    if  "/" in measure_value:
                        d2 = dict([measure_value.split('/')])
                        measure_value=str(d2.items()[0][0])
                        metadata_value=str(d2.items()[0][1])
                    attr_matches=True
                    attr_exists=False
                    for attr in contextElement['attributes']:
                        if str(measure_name) == attr['name']:
                            attr_exists=True
                            print 'Compruebo no existencia del atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                            if str(attr['value']) != str(measure_value):
                                attr_matches=False
                                break
                    if not attr_exists:
                        attr_matches=False
                assert not attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " found in: " + str(contextElement['attributes'])
    else:
        assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
        print "Measures are NOT received"
                
@step('the command of device "([^"]*)" with response "([^"]*)" and status "([^"]*)" is received by context broker')
def check_status_info(step, asset_name, response, status):
    world.response={}
    if status=='OK':
        functions.check_command_cbroker(asset_name, status, response)
    else:
        if response:
            world.response=status
        functions.check_command_cbroker(asset_name, status)
        functions.check_NOT_command_cbroker(asset_name, response, "Info")

@step('the command of device "([^"]*)" with response "([^"]*)" and status "([^"]*)" is received or NOT by context broker')
def check_wrong_status_info(step, asset_name, response, status):
    world.response={}
    if status != "fail":
        if ('not_send' in response) | ("length" in response):
            functions.check_command_cbroker(asset_name, status)
        else:
            functions.check_command_cbroker(asset_name, status, response)
    else:
        functions.check_NOT_command_cbroker(asset_name, response, "Status")
        functions.check_NOT_command_cbroker(asset_name, response, "Info")
    
@step('the command of device "([^"]*)" with response "([^"]*)", entity_type "([^"]*)" and status "([^"]*)" is received by context broker')
def check_status_entity_info(step, asset, response, entity_type, status):
    if asset:
        asset_name = asset
    else:
        if entity_type:
            asset_name = entity_type + ":" + world.device_id
        else:
            if world.thing:
                asset_name = world.thing + ":" + world.device_id
            else:
                asset_name = DEF_ENTITY_TYPE + ":" + world.device_id
    if entity_type:
        functions.check_command_cbroker(asset_name, status, response, entity_type)
    else:
        functions.check_command_cbroker(asset_name, status, response)
    
@step('a SMS to telephone "([^"]*)" is received by SMPP Host')
def check_sms(step, tel_number):
    time.sleep(1)
    req =  requests.get(SMPP_URL+"/simulaClient/smpp1")
    response = req.json()
    assert response['from'] == "tel:"+SMPP_FROM, 'ERROR: telephone number: ' + SMPP_FROM + " not found in: " + str(response['from'])
    assert response['to'][0] == "tel:"+tel_number, 'ERROR: telephone number: ' + tel_number + " not found in: " + str(response['to'][0])
    assert response['message'] == world.code+"AK"+world.msg_id, 'ERROR: message ' + world.code+"AK"+world.msg_id + " not found in: " + str(response['message'])

@step('a SMS to telephone "([^"]*)" is received or NOT by SMPP Host')
def check_NOT_sms(step, tel_number):
    time.sleep(1)
    req =  requests.get(SMPP_URL+"/simulaClient/smpp1")
    response = req.json()
    if ((world.field == "timestamp" ) | world.are_measures) & (not world.multidata):
        assert response['from'] == "tel:"+SMPP_FROM, 'ERROR: telephone number: ' + SMPP_FROM + " not found in: " + str(response['from'])
        assert response['to'][0] == "tel:"+tel_number, 'ERROR: telephone number: ' + tel_number + " not found in: " + str(response['to'][0])
        assert response['message'] == world.code+"AK"+world.msg_id, 'ERROR: message ' + world.code+"AK"+world.msg_id + " not found in: " + str(response['message'])
        print "SMS is received"
    else:
        assert response['to'] != "tel:"+tel_number, 'ERROR: telephone number: ' + tel_number + " found in: " + str(response['to'])
        print "SMS is NOT received"    