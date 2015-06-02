import sys, mosquitto, re, time
from gw_configuration import MQTT_BROKER_HOSTNAME, MQTT_BROKER_PORT

# Client configuration
config = {
    'host': '',
    'port': '',
    'client_id': '',
    'apikey': '',
    'device_id': '',
    'num': 0,
    'num_commands': 0
}



def mqtt_command(apikey, device_id, cmd_type, num):
    config['host'] = MQTT_BROKER_HOSTNAME;
    config['port'] = MQTT_BROKER_PORT;
    config['client_id'] = '3333';
    config['apikey'] = apikey
    config['device_id'] = device_id
    config['num'] = num
    config['num_commands'] = 0
    config['type'] = cmd_type
    global mqttc;
    global num_commands;
    mqttc = mosquitto.Mosquitto(config['client_id'])
    start(apikey, cmd_type);


def on_connect(mosq, obj, rc):
    print("rc: "+str(rc))
    topic = '+/+/cmd/+';
    subscribe(topic);

def on_message(mosq, obj, msg):
    message = str(msg.payload);
    print(msg.topic+" "+str(msg.qos)+" "+str(msg.payload))
    m = re.match( r'(.*)/(.*)/(.*)/(.*)', msg.topic);
    if m:
        apikey = m.group(1);
        device = m.group(2)
        cmd_name = m.group(4);
        my_topic =  apikey + '/' + device + '/cmdexe/' + cmd_name;
        #time.sleep(1)       
        publish(my_topic, message+"Result|Command_OK");
    config['num_commands']+= 1
    if config['num_commands'] == config['num']:
        disconnect()

def on_publish(mosq, obj, mid):
    print("mid: "+str(mid))

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed: "+str(mid)+" "+str(granted_qos))
    cmd = re.match( r'pull(.*)', config['type'])
    if cmd:
        topic = config['apikey'] + '/' + config['device_id'] + '/cmdget'
        payload = '11'
        publish(topic, payload)
    cmd2 = re.match( r'(.*)_not_resp', config['type'])
    if cmd2:
        time.sleep(2)
        disconnect()    

def on_log(mosq, obj, level, string):
    print(string)


def on_disconnect(mosq, obj, rc):
    print("Disconnected successfully.")


def start(user,cmd_type):
    cmd = re.match( r'(.*)_not_resp', cmd_type)
    if not cmd:
        mqttc.on_message = on_message
    mqttc.username_pw_set(user, "")
    mqttc.on_connect = on_connect
    mqttc.on_publish = on_publish
    mqttc.on_subscribe = on_subscribe
    # Uncomment to enable debug messages
    mqttc.on_log = on_log
    mqttc.on_disconnect = on_disconnect
    mqttc.connect(config['host'], config['port'], 60)
    it=0
    while it<10:
        mqttc.loop()
        it+=1

def subscribe(topic):
    print 'Subscrito a topic: ' +topic
    mqttc.subscribe(topic)

def publish(topic, content):
    mqttc.publish(topic, content, 1);

def disconnect():
    mqttc.disconnect()

#if __name__ == "__main__":
#    mqtt_command()
