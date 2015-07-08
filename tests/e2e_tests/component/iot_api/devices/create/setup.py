from lettuce import step, world
from common.functions import Functions
from common.steps import service_with_path_created_precond, device_not_created_precond, create_device, create_device_with_attrs_cmds, device_created


functions = Functions()

@step('I try to create a Device with name "([^"]*)", path "([^"]*)", protocol "([^"]*)" and atribute or command "([^"]*)", with name "([^"]*)", type "([^"]*)" and value "([^"]*)"')
def create_device_failed(step, dev_name, service_path, protocol, typ, name, type1, value):
    world.srv_path=service_path
    world.commands=[]
    world.attributes=[]
    world.st_attributes=[]
    functions.fill_attributes(typ, name, type1, value, {}, {}, {}, {}, False)
    if typ=='ent_name':
        world.entity_name=name
        world.req=functions.create_device(world.service_name, world.srv_path, dev_name, {}, world.commands, name, {}, world.attributes, world.st_attributes, protocol)
    elif typ=='protocol':
        if not name=='null':
            world.req=functions.create_device(world.service_name, world.srv_path, dev_name, {}, world.commands, {}, {}, world.attributes, world.st_attributes, name)
        else:
            world.req=functions.create_device(world.service_name, world.srv_path, dev_name, {}, world.commands, {}, {}, world.attributes, world.st_attributes)
    else:        
        world.req=functions.create_device(world.service_name, world.srv_path, dev_name, {}, world.commands, {}, {}, world.attributes, world.st_attributes, protocol)
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El device {} se ha podido crear".format(dev_name)
    print 'No se ha creado el device {}'.format(dev_name)
    
@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_device_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    assert str(error_text.format(world.service_name)) in world.req.text, 'ERROR: ' + world.req.text
    if http_status=="409":
        if 'duplicate' in error_text:
            assert world.device_id in world.req.text, 'ERROR: ' + world.req.text        
            assert world.service_name in world.req.text, 'ERROR: ' + world.req.text
            if world.srv_path=='void':
                world.srv_path='/'        
            assert world.srv_path in world.req.text, 'ERROR: ' + world.req.text
        else:
            assert world.entity_name in world.req.text, 'ERROR: ' + world.req.text        
                    
    
    
