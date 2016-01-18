from iotqatools.iota_utils import Rest_Utils_IoTA
from common.gw_configuration import CBROKER_URL,CBROKER_URL_TLG,CBROKER_HEADER,CBROKER_PATH_HEADER,IOT_SERVER_ROOT,DEF_ENTITY_TYPE,MANAGER_SERVER_ROOT,PATH_UL20_SIMULATOR,DEF_TYPE,TIMESTAMP,KS_USER,KS_DOMAIN,KS_PASSWD,KS_IP,KS_PORT
from lettuce import world
import time, datetime, requests
from iotqatools.ks_utils import KeystoneCrud

ks = KeystoneCrud(username=KS_USER,password=KS_PASSWD,domain=KS_DOMAIN,ip=KS_IP,port=KS_PORT)
token = ks.get_token()
iotagent = Rest_Utils_IoTA(server_root=IOT_SERVER_ROOT+'/iot', server_root_secure=IOT_SERVER_ROOT+'/iot', cbroker=CBROKER_URL, token=token)
iota_manager = Rest_Utils_IoTA(server_root=MANAGER_SERVER_ROOT+'/iot', server_root_secure=IOT_SERVER_ROOT+'/iot', cbroker=CBROKER_URL, token=token)

URLTypes = {
    "IoTUL2": "/iot/d",
    "IoTRepsol": "/iot/repsol",
    "IoTModbus": "/iot/tgrepsol",
    "IoTEvadts": "/iot/evadts",
    "IoTTT": "/iot/tt",
    "IoTMqtt": "/iot/mqtt"
}

ProtocolTypes = {
    "IoTUL2": "PDI-IoTA-UltraLight",
    "IoTTT": "PDI-IoTA-ThinkingThings",
    "IoTRepsol": "PDI-SMS-REPSOL",
    "IoTModbus": "PDI-MODBUS-REPSOL",
    "IoTMqtt": "PDI-IoTA-MQTT-UltraLight"
}

class Functions(object):
    world.service_exists = False
    world.service_path_exists = False
    world.device_exists = False
    world.check_manager = False

    def service_precond(self, service_name, protocol, attributes={}, static_attributes={}, cbroker={}):
        world.service_name = service_name
        world.srv_path = '/'
        if protocol:
            resource = URLTypes.get(protocol)
        else:
            resource= {}
        if not iotagent.service_created(service_name, {}, resource):
            service = iotagent.create_service(service_name, protocol, attributes, static_attributes, cbroker)
            assert service.status_code == 201, 'Error al crear el servicio {} '.format(service_name)
            print 'Servicio {} creado '.format(service_name)
        else:
            print 'El servicio {} existe '.format(service_name)
        world.protocol=protocol
        world.remember.setdefault(service_name, {})
        world.service_exists = True

    def service_with_params_precond(self, service_name, service_path, resource, apikey, cbroker={}, entity_type={}, token={}, attributes={}, static_attributes={}):
        world.service_name = service_name
        if not iotagent.service_created(service_name, service_path, resource):
            service = iotagent.create_service_with_params(service_name, service_path, resource, apikey, cbroker, entity_type, token, attributes, static_attributes)
            assert service.status_code == 201, 'Error al crear el servicio {} '.format(service_name)
            print 'Servicio {} creado '.format(service_name)
            if service.status_code == 201 or service.status_code == 409:
                world.remember.setdefault(service_name, {})
                if service_path == 'void':
                    service_path='/'
                world.remember[service_name].setdefault(service_path, {})
                world.remember[service_name][service_path].setdefault('resource', {})
                world.remember[service_name][service_path]['resource'].setdefault(resource, {})
                if not apikey:
                    apikey = ""
                world.remember[service_name][service_path]['resource'][resource].setdefault(apikey)
                world.service_exists = True
                world.service_path_exists = True
        else:
            print 'El servicio {} existe '.format(service_name)
            world.remember.setdefault(service_name, {})
            if service_path == 'void':
                service_path='/'
            world.remember[service_name].setdefault(service_path, {})
            world.remember[service_name][service_path].setdefault('resource', {})
            world.remember[service_name][service_path]['resource'].setdefault(resource, {})
            if not apikey:
                apikey = ""
            world.remember[service_name][service_path]['resource'][resource].setdefault(apikey)
            world.service_exists = True
            world.service_path_exists = True

    def not_service_precond(self, service_name, service_path, resource, apikey):
        if iotagent.service_created(service_name, service_path, resource):
            service=iotagent.delete_service_with_params(service_name, service_path, resource, apikey)
            assert service.status_code == 204, 'ERROR: ' + service.text + "El servicio {} no se ha borrado correctamente".format(service_name)
            print 'El servicio {} se ha tenido que borrar'.format(service_name)
            if iotagent.service_created(service_name, service_path, resource):
                print 'ERROR: El servicio {} sigue existiendo'.format(service_name)
                world.remember.setdefault(service_name, {})
                if service_path == 'void':
                    service_path='/'
                world.remember[service_name].setdefault(service_path, {})
                world.remember[service_name][service_path].setdefault('resource', {})
                world.remember[service_name][service_path]['resource'].setdefault(resource, {})
                if not apikey:
                    apikey = ""
                world.remember[service_name][service_path]['resource'][resource].setdefault(apikey)
                world.service_exists = True
                world.service_path_exists = True

    def create_service_with_params(self, service_name, service_path, resource={}, apikey={}, cbroker={}, entity_type={}, token={}, attributes={}, static_attributes={}, protocol={}):
#        world.protocol={}
        world.service_name = service_name
        if protocol:
            service = iota_manager.create_service_with_params(service_name, service_path, {}, apikey, cbroker, entity_type, token, attributes, static_attributes, protocol)
            resource=world.resource
        else:
            service = iotagent.create_service_with_params(service_name, service_path, resource, apikey, cbroker, entity_type, token, attributes, static_attributes)
        if service.status_code == 201 or service.status_code == 200 or service.status_code == 409:
            world.remember.setdefault(service_name, {})
            if service_path == 'void':
                service_path='/'
            world.remember[service_name].setdefault(service_path, {})
            world.remember[service_name][service_path].setdefault('resource', {})
            world.remember[service_name][service_path]['resource'].setdefault(resource, {})
            if not apikey:
                apikey = ""
            world.remember[service_name][service_path]['resource'][resource].setdefault(apikey)
            world.service_exists = True
            world.service_path_exists = True
        return service

    def update_service_with_params(self, attribute, service_name, value, service_path={}, resource={}, apikey={}, fail=False, manager=False, protocol={}):
        json = self.fill_update_json(attribute, value, manager)
        if manager:
            if protocol:
                prot = ProtocolTypes.get(protocol)
                if not prot:
                    prot=protocol
                if prot == 'void':
                    json['services'][0]['protocol']= []
                else:
                    json['services'][0]['protocol']= [prot]
            req =  iota_manager.update_service_with_params(json, service_name, service_path, {}, apikey)
        else:
            req =  iotagent.update_service_with_params(json, service_name, service_path, resource, apikey)            
        if service_path == 'void':
            service_path='/'
        if attribute == 'apikey':
            print world.remember[service_name][service_path]['resource']
            del world.remember[service_name][service_path]['resource'][world.resource][world.apikey]
            if not value:
                value = ""
            world.remember[service_name][service_path]['resource'][world.resource].setdefault(value)
            print world.remember[service_name][service_path]['resource']
        if ((attribute == 'resource') or (attribute == 'protocol')) and (not fail):
            print world.remember[service_name][service_path]['resource']
            del world.remember[service_name][service_path]['resource'][world.resource]
            world.remember[service_name][service_path]['resource'].setdefault(value, {})
            world.remember[service_name][service_path]['resource'][value].setdefault(world.apikey)
            print world.remember[service_name][service_path]['resource']
        return req           

    def get_service_created(self, service_name, service_path, resource={}, limit={}, offset={}, protocol={}, manager=False):
        if manager:
            req =  iota_manager.get_service_with_params(service_name, service_path, {}, limit, offset, protocol)
            world.check_manager=True            
        else:
            req =  iotagent.get_service_with_params(service_name, service_path, resource, limit, offset)
            world.check_manager=False
        world.req = req
        return req

    def check_service_data(self, num_services, attribute={}, value={}):
        res = world.req.json()
        assert len(res['services']) == int(num_services), 'Error: ' + str(num_services) + ' services expected, ' + str(len(res['services'])) + ' received'
        if len(res['services'])==1:
            response = res['services'][0]
            if world.check_manager:
                assert response['protocol'] == world.prot, 'Expected Result: ' + world.prot + '\nObtained Result: ' + response['protocol']
            else:
                if (attribute == 'resource') or (attribute == 'protocol'):
                    if attribute == 'protocol':
                        resource = URLTypes.get(value)
                    else:
                        resource = value                    
                    assert response['resource'] != world.resource, 'NOT Expected Result: ' + world.resource + '\nObtained Result: ' + response['resource']
                    assert response['resource'] == resource, 'Expected Result: ' + resource + '\nObtained Result: ' + response['resource']
                else:
                    assert response['resource'] == world.resource, 'Expected Result: ' + world.resource + '\nObtained Result: ' + response['resource']
            if attribute == 'apikey':
                if value:
                    assert response['apikey'] != world.apikey, 'NOT Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
                    assert response['apikey'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['apikey']
                else:
                    assert response['apikey'] != world.apikey, 'NOT Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
                    assert response['apikey'] == "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
            else:
                if world.apikey:
                    assert response['apikey'] == world.apikey, 'Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
                else:
                    assert response['apikey'] == "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
            if attribute == 'cbroker':
                assert response['cbroker'] != world.cbroker, 'NOT Expected Result: ' + world.cbroker + '\nObtained Result: ' + response['cbroker']
                assert response['cbroker'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['cbroker']
            else:
                if world.cbroker:
                    assert response['cbroker'] == world.cbroker, 'Expected Result: ' + world.cbroker + '\nObtained Result: ' + response['cbroker']
                else:
                    assert response['cbroker'] == "", 'Expected Result: NULL \nObtained Result: ' + response['cbroker']
            if attribute == 'token':
                assert response['token'] != world.token, 'NOT Expected Result: ' + world.token + '\nObtained Result: ' + response['token']
                assert response['token'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['token']
            else:
                if world.token:
                    assert response['token'] == world.token, 'Expected Result: ' + world.token + '\nObtained Result: ' + response['token']
            self.check_attributes(attribute, value, response)

    def check_NOT_service_data(self, attribute, value):
        res = world.req.json()
        response = res['services'][0]
        assert response['service'] == world.service_name, 'Expected Result: ' + world.service_name + '\nObtained Result: ' + response['service']
        if world.srv_path == 'void':
            world.srv_path = '/'
        assert response['service_path'] == world.srv_path, 'Expected Result: ' + world.srv_path + '\nObtained Result: ' + response['service_path']
        if (attribute == 'resource') or (attribute == 'protocol'):
            if attribute == 'protocol':
                resource = URLTypes.get(value)
            else:
                resource = value                    
            assert response['resource'] != resource, 'NOT Expected Result: ' + resource + '\nObtained Result: ' + response['resource']
        assert response['resource'] == world.resource, 'Expected Result: ' + world.resource + '\nObtained Result: ' + response['resource']
        if attribute == 'apikey':
            if value:
                assert response['apikey'] != value, 'NOT Expected Result: ' + value + '\nObtained Result: ' + response['apikey']
            else:
                assert response['apikey'] != "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
            assert response['apikey'] == world.apikey, 'Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
        else:
            if world.apikey:
                assert response['apikey'] == world.apikey, 'Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
            else:
                assert response['apikey'] == "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
        if attribute == 'cbroker':
            assert response['cbroker'] != value, 'NOT Expected Result: ' + value + '\nObtained Result: ' + response['cbroker']
        assert response['cbroker'] == world.cbroker, 'Expected Result: ' + world.cbroker + '\nObtained Result: ' + response['cbroker']

    def delete_service_data(self, service_name, service_path, resource={}, apikey={}, device={}):
        req =  iotagent.delete_service_with_params(service_name, service_path, resource, apikey, device)
        return req

    def check_service_created(self, service_name, service_path, resource={}, delete=False):
        req = iotagent.service_created(service_name, service_path, resource)
        if  (delete) and not req:
            if service_path == 'void':
                del world.remember[service_name]['/']['resource'][resource]
            else:
                del world.remember[service_name][service_path]['resource'][resource]
        return req

    def device_precond(self, device_id, endpoint={}, protocol={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}):
        world.device_id = device_id
        world.device_type={}
        if not iotagent.device_created(world.service_name, device_id):
            prot = ProtocolTypes.get(protocol)
            device = iotagent.create_device(world.service_name, device_id, {}, endpoint, commands, entity_name, entity_type, attributes, static_attributes, prot)
            assert device.status_code == 201, 'Error al crear el device {} '.format(device_id)
            print 'Device {} creado '.format(device_id)
            world.device_created=True
        else:
            print 'El device {} existe '.format(device_id)
            world.device_created=False
        if entity_name:
            world.device_name=entity_name
        else:
            if entity_type:
                world.device_name = entity_type + ":" + device_id
            else:
                if world.thing:
                    world.device_name = world.thing + ":" + device_id
                else:
                    world.device_name = DEF_ENTITY_TYPE + ":" + device_id
        if entity_type:
            world.device_type=entity_type
        world.remember[world.service_name].setdefault('device', set())
        world.remember[world.service_name]['device'].add(device_id)
        world.device_exists = True

    def device_with_commands_precond(self, device_id, device_name, protocol, cmd_name, cmd_value, endpoint={}, ent_type={}):
        if endpoint:
            if endpoint=='void':
                endpoint={}
            else:
                if not "http://" in endpoint:
                    endpoint = CBROKER_URL + endpoint
        else:
            endpoint =  CBROKER_URL+PATH_UL20_SIMULATOR      
        if cmd_value:
            replaces = {
                "#": "|"
            }
            for kreplace in replaces:
                cmd_value = cmd_value.replace(kreplace,replaces[kreplace])
        if not cmd_value=='fail':
            command=[
                     {
                      "name": cmd_name,
                      "type": 'command',
                      "value": cmd_value
                      }
                     ]
            self.device_precond(device_id, endpoint, protocol, command, device_name, ent_type)
        else:
            world.device_id=device_id
    def not_device_precond(self, device_id, endpoint={}, protocol={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}):
        world.device_id = device_id
        if iotagent.device_created(world.service_name, device_id, world.srv_path):
            device=iotagent.delete_device_with_params(device_id, world.service_name, world.srv_path)
            assert device.status_code == 204, 'ERROR: ' + device.text + "El device {} no se ha borrado correctamente".format(device_id)
            print 'El device {} se ha tenido que borrar'.format(device_id)
            if iotagent.device_created(world.service_name, device_id, world.srv_path):
                print 'ERROR: El device {} sigue existiendo'.format(device_id)
                if world.srv_path=='void':
                    service_path='/'
                else:
                    service_path=world.srv_path
                world.remember[world.service_name][service_path].setdefault('device', set())
                world.remember[world.service_name][service_path]['device'].add(device_id)
                world.device_exists = True

    def device_of_service_precond(self, service_name, service_path, device_id, endpoint={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}, protocol={}):
        world.device_id = device_id
        if not iotagent.device_created(service_name, device_id, service_path):
            prot = ProtocolTypes.get(protocol)
            device = iotagent.create_device(world.service_name, device_id, service_path, endpoint, commands, entity_name, entity_type, attributes, static_attributes, prot)
            assert device.status_code == 201, 'Error al crear el device {} '.format(device_id)
            print 'Device {} con path {} creado '.format(device_id, service_path)
        else:
            print 'El device {} existe '.format(device_id)
        if service_path=='void':
            service_path2='/'
        else:
            service_path2=service_path
        world.remember[service_name][service_path2].setdefault('device', set())
        world.remember[service_name][service_path2]['device'].add(device_id)
        world.device_exists = True

    def create_device(self, service_name, service_path, device_id, endpoint={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}, protocol={}, manager=False):
        world.device_id = device_id
        if protocol:
            prot = ProtocolTypes.get(protocol)
            if not prot:
                prot = protocol
        else:
            prot = {}    
        world.prot=prot
        if manager:
            device = iota_manager.create_device(world.service_name, device_id, service_path, endpoint, commands, entity_name, entity_type, attributes, static_attributes, prot)
        else:
            device = iotagent.create_device(world.service_name, device_id, service_path, endpoint, commands, entity_name, entity_type, attributes, static_attributes, prot)            
        if device.status_code == 201 or device.status_code == 409:
            if service_path=='void':
                service_path2='/'
            else:
                service_path2=service_path
            if world.service_path_exists:
                world.remember[service_name][service_path2].setdefault('device', set())
                world.remember[service_name][service_path2]['device'].add(device_id)
            else:
                world.remember[service_name].setdefault('device', set())
                world.remember[service_name]['device'].add(device_id)                
            world.device_exists = True
        return device

    def update_device_with_params(self, attribute, device_name, value, service_name, service_path={}, fail=False, manager=False, protocol={}):
        json = self.fill_update_json(attribute, value, False)
        if manager:
            req =  iota_manager.update_device_with_params(json, device_name, service_name, service_path, protocol)
        else:
            req =  iotagent.update_device_with_params(json, device_name, service_name, service_path)            
        if service_path == 'void':
            service_path='/'
        if (attribute == 'device_id') and (not fail):
            world.device_id=value
            world.remember[world.service_name][world.srv_path]['device'].remove(device_name)
        return req           

    def get_device_created(self, service_name, service_path, device_name, protocol={}, manager=False):
        if manager:
            req =  iota_manager.get_device_with_params(service_name, device_name, service_path, protocol)
            world.check_manager=True            
        else:
            req =  iotagent.get_device_with_params(service_name, device_name, service_path)
            world.check_manager=False
        world.req = req
        return req
    
    def get_devices_created(self, service_name, service_path, entity={}, limit={}, offset={}, detailed={}, protocol={}, manager=False):
        if manager:
            req =  iota_manager.get_devices_with_params(service_name, service_path, protocol, entity, detailed, limit, offset)
            world.check_manager=True            
        else:
            req =  iotagent.get_devices_with_params(service_name, service_path, protocol, entity, detailed, limit, offset)
            world.check_manager=False
        world.req = req
        return req

    def check_device_created(self, service_name, device_name, service_path, delete=False):
        req = iotagent.device_created(service_name, device_name, service_path)
        if  (delete) and not req:
            if service_path == 'void':
                world.remember[service_name]['/']['device'].remove(device_name)
            else:
                world.remember[service_name][service_path]['device'].remove(device_name)
        return req

    def check_device_data(self, dev_name, manager=False, detailed='on', attribute={}, value={}, commands={}):
        res = world.req.json()
        if manager:
            assert len(res['devices']) == 1, 'Error: 1 devices expected, ' + str(len(res['devices'])) + ' received'
            response = res['devices'][0]
        else:
            response = res
        assert response['device_id'] == dev_name, 'Expected Result: ' + dev_name + '\nObtained Result: ' + response['device_id']
        if not detailed=='on':
            return
        if attribute == 'entity_name':
            assert response['entity_name'] != world.entity_name, 'NOT Expected Result: ' + world.entity_name + '\nObtained Result: ' + response['entity_name']
            assert response['entity_name'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['entity_name']
        else:
            if world.entity_name:
                assert response['entity_name'] == world.entity_name, 'Expected Result: ' + world.entity_name + '\nObtained Result: ' + response['entity_name']
        if attribute == 'endpoint':
            assert response['endpoint'] != world.endpoint, 'NOT Expected Result: ' + world.endpoint + '\nObtained Result: ' + response['endpoint']
            assert response['endpoint'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['endpoint']
        else:
            if world.endpoint:
                assert response['endpoint'] == world.endpoint, 'Expected Result: ' + world.endpoint + '\nObtained Result: ' + response['endpoint']
        if world.prot:
            assert response['protocol'] == world.prot, 'Expected Result: ' + world.prot + '\nObtained Result: ' + response['protocol']
        if attribute:
            self.check_attributes(attribute, value, response)
        if commands:
            for i in commands.split('/'):
                self.check_command(response, i)
    
    def check_devices_data(self, num_devices, data):
        entity_name={}
        entity_type={}
        res = world.req.json()
        assert len(res['devices']) == int(num_devices), 'Error: ' + str(num_devices) + ' devices expected, ' + str(len(res['devices'])) + ' received'
        if len(res['devices']) == 1:
            self.check_device_data(world.device_id, True, world.detailed)
        if len(res['devices']) > 1:
            for i in data.split('/'):
                if '#' in i:
                    d = dict([i.split('#')]) 
                    device_name=str(d.items()[0][0])
                    attrs=str(d.items()[0][1])
                    if ':' in attrs:
                        d2 = dict([attrs.split(':')])
                        entity_name=str(d2.items()[0][0])
                        entity_type=str(d2.items()[0][1])
                else:
                    device_name=i
                dev_matches=False
                for dev in res['devices']:
                    if str(device_name) == dev['device_id']:
                        print 'Compruebo device {} en {}'.format(device_name,dev)
                        assert dev['device_id'] == str(device_name), 'ERROR: device_name: ' + str(device_name) + " not found in: " + str(dev)
                        if entity_name:
                            assert dev['entity_name'] == str(entity_name), 'ERROR: entity_name: ' + str(entity_name) + " not found in: " + str(dev)
                        if entity_type:
                            assert dev['entity_type'] == str(entity_type), 'ERROR: entity_type: ' + str(entity_type) + " not found in: " + str(dev)
                        dev_matches=True
                        break
                assert dev_matches, 'ERROR: device: ' + str(device_name) + " not found in: " + str(res['devices'])

    def delete_device_data(self, device_name, service_name, service_path, manager=False, protocol={}):
        if manager:
            req =  iota_manager.delete_device_with_params(device_name, service_name, service_path, protocol)
        else:
            req =  iotagent.delete_device_with_params(device_name, service_name, service_path)
        return req

    def check_measure(self, device, measures, timestamp={}, entity_type={}, entity_name={}, are_attrs=False):
        time.sleep(1)
        print measures
        req =  requests.get(CBROKER_URL+"/last")
        response = req.json()
        assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
        print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
        #print 'Ultima medida recibida {}'.format(response)
        contextElement = response['contextElements'][0]
        assetElement = contextElement['id']
        #print 'Dispositivo {}'.format(assetElement)
        typeElement = contextElement['type']
        #print 'Dispositivo {}'.format(typeElement)
        for i in measures.split('#'):
                d = dict([i.split(':')]) 
                measure_name=str(d.items()[0][0])
                measure_value=str(d.items()[0][1])
                metadata_value=""
                if  "/" in measure_value:
                    if not measure_name=='l':
                        d2 = dict([measure_value.split('/')])
                        measure_value=str(d2.items()[0][0])
                        metadata_value=str(d2.items()[0][1])
                if are_attrs:
                    attrs=0
                    if ('attr' in world.typ1) & (measure_name==world.value1):
                        self.check_attribute(contextElement, world.name1, world.type1, measure_value)
                    elif ('attr' in world.typ2) & (measure_name==world.value2):
                        self.check_attribute(contextElement, world.name2, world.type2, measure_value)
                    else:
                        self.check_attribute(contextElement, measure_name, DEF_TYPE, measure_value)
                    attrs+=1
                    if 'st_att' in world.typ1:
                        self.check_attribute(contextElement, world.name1, world.type1, world.value1)
                        attrs+=1
                    if 'st_att' in world.typ2:
                        self.check_attribute(contextElement, world.name2, world.type2, world.value2)
                else:
                    attr_matches=False
                    for attr in contextElement['attributes']:
                        if str(measure_name) == attr['name']:
                            print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                            assert attr['value'] == str(measure_value), 'ERROR: value: ' + str(measure_value) + " not found in: " + str(attr)
                            attr_matches=True
                            if metadata_value:
                                assert attr['metadatas'][1]['name'] == "uom", 'ERROR: ' + str(attr['metadatas'][1])
                                assert str(metadata_value) in attr['metadatas'][1]['value'], 'ERROR: metadata: ' + str(metadata_value) + " not found in: " + str(attr['metadatas'][1])
                            assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(attr['metadatas'][0])
                            if timestamp:
                                assert str(timestamp) == attr['metadatas'][0]['value'], 'ERROR: metadata: ' + str(timestamp) + " not found in: " + str(attr['metadatas'][0])
                            else:
                                assert self.check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                            break
                    assert attr_matches, 'ERROR: attribute: ' + str(measure_name) + " not found in: " + str(contextElement['attributes'])
        is_timestamp=False
        for attr in contextElement['attributes']:
            if attr ['name'] == "TimeInstant":
                print 'Compruebo atributo TimeInstant y {} en {}'.format(attr['value'],str(attr))
                if timestamp:
                    assert str(timestamp) == attr['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
                else:
                    assert self.check_timestamp(str(attr['value'])), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
                is_timestamp=True
                break
        assert is_timestamp, 'ERROR: TimeInstant not found in' + str(contextElement['attributes'])
        if world.def_entity:
                device_name = DEF_ENTITY_TYPE + ':' + device
                ent_type = DEF_ENTITY_TYPE
        else:
            device_name=device
            ent_type=world.thing
        if entity_name:
            device_name = entity_name
        else:
            if entity_type:
                device_name = entity_type + ":" + device
        if entity_type:
            ent_type = entity_type
        assert assetElement == "{}".format(device_name), 'ERROR: id: ' + str(device_name) + " not found in: " + str(contextElement)
        print 'ID: ' + str(assetElement)
        assert typeElement == ent_type, 'ERROR: ' + ent_type + ' type expected, ' + typeElement + ' received'
        print 'TYPE: ' + str(typeElement)

    
    def check_command_cbroker(self, asset_name, status, response={}, entity_type={}):
        time.sleep(1)
        timeinstant=1
        if response:
            replaces = {
                    "#": "|"
            }
            for kreplace in replaces:
                response = response.replace(kreplace,replaces[kreplace])
        if world.protocol == 'IoTModbus':
            cbroker_url = CBROKER_URL_TLG
        else:
            cbroker_url = CBROKER_URL
        req =  requests.get(cbroker_url+"/lastStatus")
        cmd_name=str(world.cmd_name)+"_status"
        print "Voy a comprobar el STATUS del Comando: " + str(cmd_name)
        resp = req.json()
        assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
        print 'Compruebo la cabecera {} con valor {} en lastStatus'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
        contextElement = resp['contextElements'][0]
        assetElement = contextElement['id']
        valueElement = contextElement['attributes'][0]['value']
        nameElement = contextElement['attributes'][0]['name']
        assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
        print 'ID: ' + str(assetElement)
        typeElement = contextElement['type']
        if entity_type:
            ent_type = entity_type
        else:
            if world.thing:
                ent_type = world.thing
            else:
                ent_type = DEF_ENTITY_TYPE
        assert typeElement == ent_type, 'ERROR: ' + ent_type + ' type expected, ' + typeElement + ' received'
        print 'TYPE: ' + str(typeElement)
        assert nameElement == cmd_name, 'ERROR: ' + cmd_name + ' name expected, ' + nameElement + ' received'
        assert status in valueElement, 'ERROR: ' + status + ' value expected, ' + valueElement + ' received'
        assert contextElement['attributes'][0]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][0]['metadatas'][0])
        assert self.check_timestamp(contextElement['attributes'][0]['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0]['metadatas'][0])
        if response:
            cmd_name=str(world.cmd_name)+"_info"
            print "Voy a comprobar el INFO del Comando: " + str(cmd_name)
            valueElement = contextElement['attributes'][1]['value']
            nameElement = contextElement['attributes'][1]['name']
            assert nameElement == cmd_name, 'ERROR: ' + cmd_name + ' name expected, ' + nameElement + ' received'
            assert response in valueElement, 'ERROR: ' + response + ' value expected, ' + valueElement + ' received'
            assert contextElement['attributes'][1]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][1]['metadatas'][0])
            assert self.check_timestamp(contextElement['attributes'][1]['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][1]['metadatas'][0])
            timeinstant+=1
        nameTime = contextElement['attributes'][timeinstant]['name']
        assert nameTime == "TimeInstant", 'ERROR: ' + "TimeInstant" + ' name expected, ' + nameTime + ' received'
        assert self.check_timestamp(contextElement['attributes'][timeinstant]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][timeinstant])
    
    def check_NOT_command_cbroker(self, asset_name, response, cmd_type):
        time.sleep(1)
        if world.protocol == 'IoTModbus':
            cbroker_url = CBROKER_URL_TLG
        else:
            cbroker_url = CBROKER_URL
        if cmd_type == "Status":
            req =  requests.get(cbroker_url+"/lastStatus")
        else:
            req =  requests.get(cbroker_url+"/lastInfo")
        resp = req.json()
        assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
        print 'Compruebo la cabecera {} con valor {} en last{}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER],cmd_type)
        contextElement = resp['contextElements'][0]
        assetElement = contextElement['id']
        assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
        print "Command is NOT received"
        if world.code==4000:
            resp = world.req_text
            assert response in resp
            return
        if world.code:
            resp = world.req_text['contextResponses'][0]
            assert resp['statusCode']['code'] == str(world.code), 'ERROR: code error expected ' + str(world.code) + " received " + str(resp['statusCode']['code'])
            if world.response:
                assert resp['statusCode']['reasonPhrase'] == str(world.response), 'ERROR: text error expected ' + str(world.response) + " received " + resp['statusCode']['reasonPhrase']
            else:
                assert resp['statusCode']['reasonPhrase'] == response, 'ERROR: text error expected ' + response + " received " + resp['statusCode']['reasonPhrase']
    
    def check_timestamp (self, timestamp):
        threshold=-TIMESTAMP
        while (threshold<=TIMESTAMP):
            st = datetime.datetime.utcfromtimestamp(world.ts+threshold).strftime('%Y-%m-%dT%H:%M:%S')
            if st in timestamp:
                return True
            threshold+=1
        return False 
    
    def check_attribute (self, contextElement, name, typ, value):
        attr_matches=False
        for attr in contextElement['attributes']:
            if str(name) == attr['name']:
                print 'Compruebo atributo {} y {} en {}'.format(name,value,attr)
                assert attr['type'] == str(typ), 'ERROR: type: ' + str(typ) + " not found in: " + str(attr)
                assert attr['value'] == str(value), 'ERROR: value: ' + str(value) + " not found in: " + str(attr)
                assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: TimeInstant metadata not found in: ' + str(attr)
                assert self.check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                attr_matches=True
                break
        assert attr_matches, 'ERROR: attribute: ' + str(name) + " not found in: " + str(contextElement['attributes'])

    def check_command (self, contextElement, name):
        cmd_matches=False
        for cmd in contextElement['commands']:
            if str(name) == cmd['name']:
                print 'Compruebo comando {} en {}'.format(name,cmd)
                assert cmd['type'] == "command", 'ERROR: type command not found in: ' + str(cmd)
                assert cmd['value'] == "", 'ERROR: empty value not found in: ' + str(cmd)
                cmd_matches=True
                break
        assert cmd_matches, 'ERROR: command: ' + str(name) + " not found in: " + str(contextElement['commands'])

    def fill_attributes(self, typ1, name1, type1, value1, typ2={}, name2={}, type2={}, value2={}, service=True):
        world.attributes=[]
        world.st_attributes=[]
        world.commands=[]
        world.typ1 = typ1
        world.typ2 = typ2
        world.name1 = name1
        world.name2 = name2
        world.type1 = type1
        world.type2 = type2
        world.value1 = value1
        world.value2 = value2
        if (service and (typ1=='srv_attr')) | (not service and (typ1=='dev_attr')):
            attribute={}
            if name1:
                if name1=='void':
                    name1=""
                attribute['name']=name1
            if type1:
                if type1=='void':
                    type1=""
                attribute['type']=type1
            if value1:
                if value1=='void':
                    value1=""
                attribute['object_id']=value1
            world.attributes.append(attribute)
        if (service and (typ2=='srv_attr')) | (not service and (typ2=='dev_attr')):
            attribute={
                  "name": name2,
                  "type": type2,
                  "object_id": value2
                  }
            world.attributes.append(attribute)
        if (service and (typ1=='srv_st_att')) | (not service and (typ1=='dev_st_att')):
            st_attribute={}
            if name1:
                if name1=='void':
                    name1=""
                st_attribute['name']=name1
            if type1:
                if type1=='void':
                    type1=""
                st_attribute['type']=type1
            if value1:
                if value1=='void':
                    value1=""
                st_attribute['value']=value1       
            world.st_attributes.append(st_attribute)
        if (service and (typ2=='srv_st_att')) | (not service and (typ2=='dev_st_att')):
            st_attribute={
                  "name": name2,
                  "type": type2,
                  "value": value2
                  }
            world.st_attributes.append(st_attribute)
        if (not service and (typ1=='cmd')):
            command={}
            if name1:
                if name1=='void':
                    name1=""
                command['name']=name1
            if type1:
                if not type1=='null':
                    if type1=='void':
                        type1=""
                    command['type']=type1
            else:
                type1='command'
                command['type']=type1
                world.type1 = type1
            if value1:
                if value1=='void':
                    value1=""
                command['value']=value1
            print command
            world.commands.append(command)
        if (not service and (typ2=='cmd')):
            command={
                  "name": name2,
                  "type": 'command',
                  "value": value2
                  }
            world.type2 = 'command'
            world.commands.append(command)

    def fill_update_json(self, attribute, value, manager=False):
        if (not 'att' in attribute ) & (attribute!='cmd'):
            if attribute=='protocol':
                resource=URLTypes.get(value)
                if resource:
                    self.update_service_with_params("resource", world.service_name, value, world.srv_path, resource, world.apikey)      
                    value = ProtocolTypes.get(value)
                    world.prot=value
            if attribute=='empty_json':
                if manager:
                    json={
                            "services":[
                                {
                                }
                                ]
                                }                                
                else:
                    json = {}
            else:
                if manager:
                    json={
                        "services":[
                            {
                             attribute: value
                            }
                            ]
                            }
                else:
                    json={
                        attribute: value
                        }
        else:
            d = dict([value.split('#')]) 
            name=str(d.items()[0][0])
            attrs=str(d.items()[0][1])
            if ':' in attrs:
                d2 = dict([attrs.split(':')])
                type1=str(d2.items()[0][0])
                value=str(d2.items()[0][1])       
            if (attribute=='srv_attr') | (attribute=='dev_attr'):
                attributes=[
                    {
                     "name": name,
                     "type": type1,
                     "object_id": value
                     }
                    ]
                attrs_type="attributes"
            if (attribute=='srv_st_att') | (attribute=='dev_st_att'):
                attributes=[
                    {
                     "name": name,
                     "type": type1,
                     "value": value
                     }
                    ]
                attrs_type="static_attributes"
            if attribute=='cmd':
                attributes=[
                    {
                     "name": name,
                     "type": 'command',
                     "value": value
                     }
                    ]
                attrs_type="commands"
            if manager:
                json={
                    "services":[
                        {
                         attrs_type: attributes
                        }
                        ]
                        }
            else:
                json={
                      attrs_type: attributes
                  }
        return json

    def check_attributes(self, attribute, value, response):
        attributes=0
        st_attributes=0
        commands=0
        assert response['service'] == world.service_name, 'Expected Result: ' + world.service_name + '\nObtained Result: ' + response['service']
        if world.srv_path:
            if world.srv_path == 'void':
                assert response['service_path'] == '/', 'Expected Result: ' + '/' + '\nObtained Result: ' + response['service_path']
            else:
                assert response['service_path'] == world.srv_path, 'Expected Result: ' + world.srv_path + '\nObtained Result: ' + response['service_path']
        if attribute == 'entity_type':
            assert response['entity_type'] != world.entity_type, 'NOT Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
            assert response['entity_type'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['entity_type']
        else:
            if world.entity_type:
                assert response['entity_type'] == world.entity_type, 'Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
        if ('att' in attribute ) | (attribute=='cmd'):
            d = dict([value.split('#')]) 
            name=str(d.items()[0][0])
            attrs=str(d.items()[0][1])
            if ':' in attrs:
                d2 = dict([attrs.split(':')])
                type1=str(d2.items()[0][0])
                value=str(d2.items()[0][1])       
        if 'attr' in attribute:
            print 'Compruebo atributo {} con valor {} y tipo {}'.format(name,value,type1)
            if attribute == world.typ1:
                print response['attributes'][0]
                assert response['attributes'][0]['name'] != world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['attributes'][0]['name']
                assert response['attributes'][0]['type'] != world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['attributes'][0]['type']
                assert response['attributes'][0]['object_id'] != world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['attributes'][0]['object_id']
            if 'st_att' in world.typ1:
                print response['static_attributes'][0]
                assert response['static_attributes'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['static_attributes'][0]['name']
                assert response['static_attributes'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['static_attributes'][0]['type']
                assert response['static_attributes'][0]['value'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['static_attributes'][0]['value']
            if world.typ1 == 'cmd':
                print response['commands'][0]
                assert response['commands'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['commands'][0]['name']
                assert response['commands'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['commands'][0]['type']
                assert response['commands'][0]['value'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['commands'][0]['value']
            print response['attributes'][0]
            assert response['attributes'][0]['name'] == name, 'Expected Result: ' + name + '\nObtained Result: ' + response['attributes'][0]['name']
            assert response['attributes'][0]['type'] == type1, 'Expected Result: ' + type1 + '\nObtained Result: ' + response['attributes'][0]['type']
            assert response['attributes'][0]['object_id'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['attributes'][0]['object_id']
        if 'st_att' in attribute:
            print 'Compruebo atributo estatico {} con valor {} y tipo {}'.format(name,value,type1)
            if attribute == world.typ1:
                print response['static_attributes'][0]
                assert response['static_attributes'][0]['name'] != world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['static_attributes'][0]['name']
                assert response['static_attributes'][0]['type'] != world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['static_attributes'][0]['type']
                assert response['static_attributes'][0]['value'] != world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['static_attributes'][0]['value']
            if 'attr' in world.typ1:
                print response['attributes'][0]
                assert response['attributes'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['attributes'][0]['name']
                assert response['attributes'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['attributes'][0]['type']
                assert response['attributes'][0]['object_id'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['attributes'][0]['object_id']
            if world.typ1 == 'cmd':
                print response['commands'][0]
                assert response['commands'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['commands'][0]['name']
                assert response['commands'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['commands'][0]['type']
                assert response['commands'][0]['value'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['commands'][0]['value']
            print response['static_attributes'][0]
            assert response['static_attributes'][0]['name'] == name, 'Expected Result: ' + name + '\nObtained Result: ' + response['static_attributes'][0]['name']
            assert response['static_attributes'][0]['type'] == type1, 'Expected Result: ' + type1 + '\nObtained Result: ' + response['static_attributes'][0]['type']
            assert response['static_attributes'][0]['value'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['static_attributes'][0]['value']
        if attribute == 'cmd':
            print 'Compruebo comando {} con valor {} y tipo {}'.format(name,value,type1)
            if attribute == world.typ1:
                print response['commands'][0]
                assert response['commands'][0]['name'] != world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['commands'][0]['name']
                assert response['commands'][0]['value'] != world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['commands'][0]['value']
            if 'attr' in world.typ1:
                print response['attributes'][0]
                assert response['attributes'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['attributes'][0]['name']
                assert response['attributes'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['attributes'][0]['type']
                assert response['attributes'][0]['object_id'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['attributes'][0]['object_id']
            if 'st_att' in world.typ1:
                print response['static_attributes'][0]
                assert response['static_attributes'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['static_attributes'][0]['name']
                assert response['static_attributes'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['static_attributes'][0]['type']
                assert response['static_attributes'][0]['value'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['static_attributes'][0]['value']
            print response['commands'][0]
            assert response['commands'][0]['name'] == name, 'Expected Result: ' + name + '\nObtained Result: ' + response['commands'][0]['name']
            assert response['commands'][0]['type'] == "command", 'Expected Result: command \nObtained Result: ' + response['commands'][0]['type']
            assert response['commands'][0]['value'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['commands'][0]['value']
        if ((not attribute) or ((not 'att' in attribute) and (not attribute=='cmd'))) and (world.typ1):
            print 'Compruebo campo {} con valor {} y tipo {}'.format(world.name1,world.value1,world.type1)           
            if 'attr' in world.typ1:
                assert response['attributes'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['attributes'][0]['name']
                assert response['attributes'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['attributes'][0]['type']
                assert response['attributes'][0]['object_id'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['attributes'][0]['object_id']
                attributes+=1
            if 'st_att' in world.typ1:
                assert response['static_attributes'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['static_attributes'][0]['name']
                assert response['static_attributes'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['static_attributes'][0]['type']
                assert response['static_attributes'][0]['value'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['static_attributes'][0]['value']
                st_attributes+=1
            if world.typ1 == 'cmd':
                assert response['commands'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['commands'][0]['name']
                assert response['commands'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['commands'][0]['type']
                assert response['commands'][0]['value'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['commands'][0]['value']
                commands+=1
        if world.typ2:
            print 'Compruebo campo {} con valor {} y tipo {}'.format(world.name2,world.value2,world.type2)           
            if 'attr' in world.typ2:
                assert response['attributes'][attributes]['name'] == world.name2, 'Expected Result: ' + world.name2 + '\nObtained Result: ' + response['attributes'][attributes]['name']
                assert response['attributes'][attributes]['type'] == world.type2, 'Expected Result: ' + world.type2 + '\nObtained Result: ' + response['attributes'][attributes]['type']
                assert response['attributes'][attributes]['object_id'] == world.value2, 'Expected Result: ' + world.value2 + '\nObtained Result: ' + response['attributes'][attributes]['object_id']
            if 'st_att' in world.typ2:
                assert response['static_attributes'][st_attributes]['name'] == world.name2, 'Expected Result: ' + world.name2 + '\nObtained Result: ' + response['static_attributes'][st_attributes]['name']
                assert response['static_attributes'][st_attributes]['type'] == world.type2, 'Expected Result: ' + world.type2 + '\nObtained Result: ' + response['static_attributes'][st_attributes]['type']
                assert response['static_attributes'][st_attributes]['value'] == world.value2, 'Expected Result: ' + world.value2 + '\nObtained Result: ' + response['static_attributes'][st_attributes]['value']
            if world.typ2 == 'cmd':
                assert response['commands'][commands]['name'] == world.name2, 'Expected Result: ' + world.name2 + '\nObtained Result: ' + response['commands'][commands]['name']
                assert response['commands'][commands]['type'] == world.type2, 'Expected Result: ' + world.type2 + '\nObtained Result: ' + response['commands'][commands]['type']
                assert response['commands'][commands]['value'] == world.value2, 'Expected Result: ' + world.value2 + '\nObtained Result: ' + response['commands'][commands]['value']

    def clean(self,dirty):
        if world.service_exists:
            for srv in dirty.keys():
                if world.device_exists:
                    if world.service_path_exists:
                        for path in dirty[srv]:
                            if dirty[srv][path].__contains__('device'):
                                for device in dirty[srv][path]['device']:
                                    req_device = iotagent.delete_device_with_params(device,srv,path)
                                    if req_device.status_code == 204:
                                        print 'Se ha borrado el device:{} del servicio:{} y path:{}'.format(device,srv,path) 
                                    else:
                                        print 'No se ha podido borrar el device:{} del servicio:{} y path:{}'.format(device,srv,path)
                    else:
                        if dirty[srv].__contains__('device'):
                            for device in dirty[srv]['device']:
                                req_device = iotagent.delete_device_with_params(device,srv)
                                if req_device.status_code == 204:
                                    print 'Se ha borrado el device ' + str(device) + ' del servicio ' + str(srv)
                                else:
                                    print 'No se ha podido borrar el device ' + str(device) + ' del servicio ' + str(srv)
                if world.service_path_exists:
                    for path in dirty[srv]:
                        if dirty[srv][path].__contains__('resource'):
                            for resource in dirty[srv][path]['resource']:
                                for apikey in dirty[srv][path]['resource'][resource]:
                                    if world.protocol:
                                        if (world.protocol == 'IoTRepsol') | (world.protocol == 'IoTModbus'):
                                            break                                    
                                    req_service = iotagent.delete_service_with_params(srv, path, resource, apikey)
                                    if req_service.status_code == 204:
                                        print 'Se ha borrado el servicio:{} path:{} resource:{} y apikey:{}'.format(srv,path,resource,apikey)
                                    else:
                                        print 'No se ha podido borrar el servicio:{} path:{} resource:{} y apikey:{}'.format(srv,path,resource,apikey)
                else:
                    if world.protocol:
                        resource2 = URLTypes.get(world.protocol)
                        if (world.protocol == 'IotTT') | (world.protocol == 'IoTRepsol'):
                            apikey=''
                        else:
                            apikey='apikey_' + str(srv)
                        if (world.protocol == 'IoTRepsol') | (world.protocol == 'IoTModbus'):
                            return                                    
                    req_service = iotagent.delete_service_with_params(srv, {}, resource2, apikey)
                    if req_service.status_code == 204:
                        print 'Se ha borrado el servicio ' + srv
                    else:
                        print 'No se ha podido borrar el servicio ' + srv
