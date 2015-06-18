from iotqautils.gtwRest import Rest_Utils_SBC
from iotqautils.iota_utils import Rest_Utils_IoTA
from common.gw_configuration import CBROKER_URL,CBROKER_HEADER,CBROKER_PATH_HEADER,IOT_SERVER_ROOT,DEF_ENTITY_TYPE,MANAGER_SERVER_ROOT
from lettuce import world

iotagent = Rest_Utils_IoTA(server_root=IOT_SERVER_ROOT+'/iot', cbroker=CBROKER_URL)
iota_manager = Rest_Utils_IoTA(server_root=MANAGER_SERVER_ROOT+'/iot', cbroker=CBROKER_URL)

URLTypes = {
    "IoTUL2": "/iot/d",
    "IoTRepsol": "/iot/repsol",
    "IoTEvadts": "/iot/evadts",
    "IoTTT": "/iot/tt",
    "IoTMqtt": "/iot/mqtt"
}

ProtocolTypes = {
    "IoTUL2": "PDI-IoTA-UltraLight",
    "IoTTT": "PDI-IoTA-ThinkingThings",
    "IoTMqtt": "PDI-IoTA-MQTT-UltraLight"
}

class Functions(object):
    world.service_exists = False
    world.service_path_exists = False
    world.device_exists = False

    def service_created(self, service_name, service_path={}, resource={}):
        headers = {}
        params = {}
        headers[CBROKER_HEADER] = str(service_name)
        if service_path:
            if not service_path == 'void':
                headers[CBROKER_PATH_HEADER] = str(service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        if resource:
            params['resource']= resource
        service =  iotagent.get_service('', headers, params)
        if service.status_code == 200:
            serv = service.json()
            if serv['count'] == 1:
                world.service_exists = True
                if resource:
                    world.service_path_exists = True
                return True
            else:
                return False
        else:
            return False
        
    def device_created(self, service_name, device_name, service_path={}):
        headers = {}
        headers[CBROKER_HEADER] = str(service_name)
        if service_path:
            if not service_path=='void':
                headers[CBROKER_PATH_HEADER] = str(service_path)
        else:
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        device = iotagent.get_device(device_name, headers)
        if device.status_code == 200:
            world.device_exists=True
            return True
        else:
            return False
            
    def create_device(self, service_name, device_name, service_path={}, endpoint={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}, protocol={}, manager={}):
        headers = {}
        if not service_name=='void':
            headers[CBROKER_HEADER] = str(service_name)
        if service_path:
            if not service_path=='void':
                headers[CBROKER_PATH_HEADER] = str(service_path)
        else:
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        device={
            "devices":[
                {
#                    "device_id": device_name
                }
                ]
               }
        if device_name:
            if device_name=='void':
                device_name=""
            device['devices'][0]['device_id'] = device_name
        if commands:
            device['devices'][0]['commands'] = commands
        if endpoint:
            device['devices'][0]['endpoint'] = endpoint
        if entity_type:
            device['devices'][0]['entity_type'] = entity_type
#        else:
#            device['devices'][0]['entity_type'] = DEF_ENTITY_TYPE
        if entity_name:
            device['devices'][0]['entity_name'] = entity_name
        if attributes:
            device['devices'][0]['attributes'] = attributes
        if static_attributes:
            device['devices'][0]['static_attributes'] = static_attributes
        if protocol:
            if protocol=="void":
                protocol=""
            device['devices'][0]['protocol'] = protocol
        if manager:
            req = iota_manager.post_device(device,headers)
        else:
            req = iotagent.post_device(device,headers)
#        assert req.status_code == 201, 'ERROR: ' + req.text + "El device {} no se ha creado correctamente".format(device_name)
        return req
        
    def create_service(self, service_name, protocol, attributes={}, static_attributes={}):
        headers = {}
        headers[CBROKER_HEADER] = service_name
        headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        resource = URLTypes.get(protocol)
        if (protocol == 'IotTT') | (protocol == 'IoTRepsol'):
            apikey=''
        else:
            apikey='apikey_' + str(service_name)
        service={
            "services":[
                {
                    "apikey": apikey,
                    "entity_type": DEF_ENTITY_TYPE,
                    "cbroker": CBROKER_URL,
                    "resource": resource
                }
                ]
                }
        if attributes:
            service['services'][0]['attributes'] = attributes
        if static_attributes:
            service['services'][0]['static_attributes'] = static_attributes
        req = iotagent.post_service(service, headers)
        assert req.status_code == 201, 'ERROR: ' + req.text + "El servicio {} no se ha creado correctamente".format(service_name)
        world.service_exists = True            
        return req

    def create_service_with_params(self, service_name, service_path, resource={}, apikey={}, cbroker={}, entity_type={}, token={}, attributes={}, static_attributes={}, protocol={}):
        world.protocol={}
        headers = {}
        if not service_name == 'void':
            headers[CBROKER_HEADER] = service_name
        if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
        service={
            "services":[
                {
#                    "resource": resource
                }
                ]
                }
        if resource:
            if not resource == 'void':
                if not resource == 'null':
                    service['services'][0]['resource'] = resource
            else:
                service['services'][0]['resource'] = ""
#        if not apikey == 'void':
        if apikey:
            if not apikey == 'null':
                service['services'][0]['apikey'] = apikey
        else:
            service['services'][0]['apikey'] = ""
        if cbroker:
            if not cbroker == 'null':
                service['services'][0]['cbroker'] = cbroker
        else:
            service['services'][0]['cbroker'] = ""
        if entity_type:
            service['services'][0]['entity_type'] = entity_type
        if token:
            service['services'][0]['token'] = token
        if attributes:
            service['services'][0]['attributes'] = attributes
        if static_attributes:
            service['services'][0]['static_attributes'] = static_attributes
        if protocol:
            if not protocol == 'void':
                if not protocol == 'null':
                    resource = URLTypes.get(protocol)
                    prot = ProtocolTypes.get(protocol)
                    if not prot:
                        prot = protocol
                    service['services'][0]['protocol']= [prot]
            else:
                resource = protocol
                service['services'][0]['protocol'] = []
            req = iota_manager.post_service(service, headers)
        else:
            req = iotagent.post_service(service, headers)
        if req.status_code == 201 or req.status_code == 409:
            world.remember.setdefault(service_name, {})
            if service_path == 'void':
                service_path='/'
#            world.remember[service_name].setdefault('path', set())
#            world.remember[service_name]['path'].add(service_path)
#            world.remember[service_name]['path'][service_path].setdefault('resource', set())
#            world.remember[service_name]['path'][service_path]['resource'].add(service_path)
            world.remember[service_name].setdefault(service_path, {})
            world.remember[service_name][service_path].setdefault('resource', {})
            world.remember[service_name][service_path]['resource'].setdefault(resource, {})
            if not apikey:
                apikey = ""
            world.remember[service_name][service_path]['resource'][resource].setdefault(apikey)
#            print world.remember
            world.service_exists = True
            world.service_path_exists = True
        return req

    def delete_device(self, device_name, service_name, service_path={}):
        headers = {}
        headers[CBROKER_HEADER] = service_name
        if service_path:
            headers[CBROKER_PATH_HEADER] = str(service_path)
        else:
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        req = iotagent.delete_device(device_name,headers)
        assert req.status_code == 204, 'ERROR: ' + req.text + "El device {} no se ha borrado correctamente".format(device_name)
        return req

    def delete_service(self, service_name, service_path={}, resource={}, apikey={}):
        params={}
        headers = {}
        headers[CBROKER_HEADER] = service_name
        if world.protocol:
            resource2 = URLTypes.get(world.protocol)
            if (world.protocol == 'IotTT') | (world.protocol == 'IoTRepsol'):
                apikey=''
            else:
                apikey='apikey_' + str(service_name)
            params = {"resource": resource2,
                      "apikey": apikey}
        if resource:
            if apikey:
                params = {"resource": resource,
                          "apikey": apikey
                          }
            else:
                params = {"resource": resource}
        if service_path:
            headers[CBROKER_PATH_HEADER] = str(service_path)
        else:
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        print params
        req = iotagent.delete_service('', headers, params)
        assert req.status_code == 204, 'ERROR: ' + req.text + "El servicio {} no se ha borrado correctamente".format(service_name)
        return req

    def service_precond(self, service_name, protocol, attributes={}, static_attributes={}):
        world.service_name = service_name
        if not iotagent.service_created(service_name):
            service = iotagent.create_service(service_name, protocol, attributes, static_attributes)
            assert service.status_code == 201, 'Error al crear el servcio {} '.format(service_name)
            print 'Servicio {} creado '.format(service_name)
        else:
            print 'El servicio {} existe '.format(service_name)
        world.protocol=protocol
        world.remember.setdefault(service_name, {})
        world.service_exists = True

    def service_with_params_precond(self, service_name, service_path, resource, apikey, cbroker={}, entity_type={}, token={}, attributes={}, static_attributes={}):
        world.protocol={}
        world.service_name = service_name
        if not self.service_created(service_name, service_path, resource):
            service = self.create_service_with_params(service_name, service_path, resource, apikey, cbroker, entity_type, token, attributes, static_attributes)
            assert service.status_code == 201, 'Error al crear el servcio {} '.format(service_name)
            print 'Servicio {} creado '.format(service_name)
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

    def device_precond(self, device_id, endpoint={}, protocol={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}):
        world.device_id = device_id
        if not iotagent.device_created(world.service_name, device_id):
            prot = ProtocolTypes.get(protocol)
            device = iotagent.create_device(world.service_name, device_id, {}, endpoint, commands, entity_name, entity_type, attributes, static_attributes, prot)
            assert device.status_code == 201, 'Error al crear el device {} '.format(device_id)
            print 'Device {} creado '.format(device_id)
        else:
            print 'El device {} existe '.format(device_id)
        world.remember[world.service_name].setdefault('device', set())
        world.remember[world.service_name]['device'].add(device_id)
        world.device_exists = True
       
    def device_of_service_precond(self, service_name, service_path, device_id, endpoint={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}, protocol={}, manager={}):
        world.device_id = device_id
        if not self.device_created(service_name, device_id, service_path):
            prot = ProtocolTypes.get(protocol)
            device = self.create_device(world.service_name, device_id, service_path, endpoint, commands, entity_name, entity_type, attributes, static_attributes, prot, manager)
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
                            apikey='apikey_' + str(world.service_name)
                    req_service = iotagent.delete_service_with_params(srv, {}, resource2, apikey)
                    if req_service.status_code == 204:
                        print 'Se ha borrado el servicio ' + srv
                    else:
                        print 'No se ha podido borrar el servicio ' + srv