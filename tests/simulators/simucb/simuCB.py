#!/usr/bin/python


__author__ = 'developer'


from flask import Flask, request, Response
from sys import argv
from datetime import datetime
from math import trunc
import os
import time
import thread
import threading
import json
import dateutil.parser
import socket
import binascii
from array import *

app = Flask(__name__)

# Default arguments
port = 7010
host='0.0.0.0'
register_context_url = '/NGSI9/registerContext'
update_context_url = '/NGSI10/updateContext'
query_context_url = '/NGSI10/queryContext'
client_ul20_command = '/simulaClient/ul20Command'
telegestion = '/telegestion'
auth_token = '/v3/auth/tokens'
auth_roles = '/v3/role_assignments'
auth_projects = '/v3/projects'
auth_access_control = '/'
auth_domains = {'TestService': '1'}
auth_prjs = {'/TestSubservice': '1'}
auth_users = dict()
auth_users['alice'] = {'id': '1', 'token': 'token-alice', 'domains': ['1'], 'projects': ['1'], 'roles': ['1']} 

roles = {'1': 'admin', '2': 'user'}
telegestion_devices = dict()
INITIAL_MODBUS = 0xFFFF
INITIAL_DF1 = 0x0000
 
table = (
0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 )
verbose = 0
#verbose =1

# Arguments from command line
if len(argv) > 1:
    port = int(argv[1])
    #server_url = argv[2]


def update(threadName, updatePayload):

    global lastM, lastStatus, lastInfo, contR, contM, contT, acuR, acuM, latenMaxR, latenMinR, latenMedR, latenMaxM, latenMinM, latenMedM, mut

    if verbose:
        print updatePayload

    data = json.loads(updatePayload)

    event_value = ''
    status_value = ''
    info_value = ''
    deltaTot = 0


    attr_list = data ["contextElements"] [0] ["attributes"]
    for i in range(len(attr_list)):
         if ( attr_list [i] ["name"] == 'Event'):
             event_value = attr_list [i] ["value"]
         elif ( attr_list [i] ["name"].endswith ('_status')):
             status_value = attr_list [i] ["value"]
             lastStatus = updatePayload
         elif ( attr_list [i] ["name"].endswith ('_info')):
             info_value = attr_list [i] ["value"]
             lastInfo = updatePayload

    if verbose:
        print event_value
        print status_value
        print info_value


    # Solo calculamos latencia, para measures (miliseg), para registros el valor de Event es 0


    if (event_value != str(0)):
        str0 = event_value[:-1]
        t0 = dateutil.parser.parse(str0)
        delta0 = time.mktime(t0.timetuple())*1000 + t0.microsecond/1000
        delta1 = time.mktime(datetime.now().timetuple())*1000 + datetime.now().microsecond/1000
        deltaTot = delta1 - delta0
        if (deltaTot < 0):
            deltaTot = 0
        if verbose:
            print t0
            print datetime.now()
            print delta0
            print delta1
            print deltaTot


    # Mutex

    mut.acquire()

    # Para registros con updateContext no calculamos acumulados ni latencias
    #if (event_value == str(0)):
    #    contR += 1

    if (event_value != str (0)):
        contM += 1
        acuM += deltaTot
        latenMedM = acuM / contM
        if (latenMinM == 0):
           latenMinM = deltaTot
        if (deltaTot < latenMinM):
           latenMinM = deltaTot
        if (latenMaxM < deltaTot):
           latenMaxM = deltaTot

    contT += 1

    mut.release()

    return


@app.route(register_context_url, methods=['POST'])
def treat_register():

    resp_aux = Response()

    for h in request.headers.keys():
        if (h == 'Fiware-Service'):
            resp_aux.headers.add('Fiware-Service', request.headers[h])


    regresp = '{ "duration" : "PT24H", "registrationId" : "123456789" } '

    return Response(response=regresp, status=200, content_type='application/json', headers=resp_aux.headers)



@app.route(client_ul20_command, methods=['GET', 'POST'])
def treat_client_ul20_cmd():

    mydata = ''

    if ((request.data is not None) and (len(request.data) != 0)):
        mydata = request.data

    for h in request.args.keys():
      if (h == 'delay'):
          saux =  request.args[h]
          t = float(saux)
          if verbose:
             print 'sleeping'
          time.sleep (t)

    command_fields = mydata.split("|");
    cmd_name = command_fields[0].split("@");

    if (cmd_name[0] == "ping"):
       cmdresp = command_fields[0] + "|ping OK"
    else :
       cmdresp = mydata

    if verbose:
       print mydata
       #print cmd_name
       #print command_fields[0]
       print cmdresp

    return Response(response=cmdresp, status=200, content_type='text/plain;charset=UTF-8')


@app.route(update_context_url, methods=['POST'])
def treat_update():

    global lastM, lastQC, lastHeaderSrv, last_id 
    
    myheaders = ''
    mydata = ''

    lastM = ''
    lastHeaderSrv = ''

    if ((request.data is not None) and (len(request.data) != 0)):
        mydata = request.data
        lastM = request.data
	data = json.loads(mydata) 
        last_id = data ["contextElements"] [0] ["id"]
        if not ("TTcurrent" in data ["contextElements"] [0] ["attributes"] [0] ["name"]):
        	lastQC = lastM

    resp_aux = Response()

    for h in request.headers.keys():
        if (h == 'Fiware-Service'):
            resp_aux.headers.add('Fiware-Service', request.headers[h])
            lastHeaderSrv = request.headers[h]

    thread.start_new_thread( update, ("Thread-update", mydata ))

    myresp = '{ "contextResponses": [ { "contextElement": '
    all = json.loads(mydata)
    ce = all ["contextElements"] [0]
    myresp +=  json.dumps(ce)
    myresp += ', "statusCode": { "code": "200", "reasonPhrase": "OK" } } ]  }'

    return Response(response=myresp, status=200, content_type='application/json', headers=resp_aux.headers)


@app.route(query_context_url, methods=['POST'])
def treat_query():
    global lastQC, lastHeaderSrv, last_id

    if ((request.data is not None) and (len(request.data) != 0)):
        mydata = request.data
	data = json.loads(mydata) 
        id = data ["entities"] [0] ["id"]

    resp_aux = Response()

    for h in request.headers.keys():
        if (h == 'Fiware-Service'):
            resp_aux.headers.add('Fiware-Service', request.headers[h])
            lastHeaderSrv = request.headers[h]

    if ( last_id == id ):
	myresp = '{ "contextResponses": [ { "contextElement": '
	all = json.loads(lastQC)
    	ce = all ["contextElements"] [0]
    	myresp +=  json.dumps(ce)
    	myresp += ', "statusCode": { "code": "200", "reasonPhrase": "OK" } } ]  }'
    else:
    	myresp = '{ "errorCode": { "code": "404", "reasonPhrase": "No context element found" } }'
   
    return Response(response=myresp, status=200, content_type='application/json', headers=resp_aux.headers)


@app.route('/last', methods=['GET'])
def treat_last():
    global lastM, lastHeaderSrv

    resp_last = Response()
    resp_last.headers.add('Fiware-Service', lastHeaderSrv)
    return Response(response=lastM, status=200, content_type='application/json', headers=resp_last.headers)


@app.route('/lastStatus', methods=['GET'])
def treat_last_status():
    global lastStatus, lastHeaderSrv

    resp_last = Response()
    resp_last.headers.add('Fiware-Service', lastHeaderSrv)
    return Response(response=lastStatus, status=200, content_type='application/json', headers=resp_last.headers)


@app.route('/lastInfo', methods=['GET'])
def treat_last_info():
    global lastInfo, lastHeaderSrv

    resp_last = Response()
    resp_last.headers.add('Fiware-Service', lastHeaderSrv)
    return Response(response=lastInfo, status=200, content_type='application/json', headers=resp_last.headers)


@app.route('/countRegister', methods=['GET'])
def count_R():
    global contR
    return Response(response=str(contR), status=200)


@app.route('/countMeasure', methods=['GET'])
def count_M():
    global contM
    return Response(response=str(contM), status=200)


@app.route('/count', methods=['GET'])
def count_T():
    global contT
    return Response(response=str(contT), status=200)


@app.route('/resetRegister', methods=['POST'])
def reset_R():

    global contR, acuR, latenMaxR, latenMinR, latenMedR
    contR = 0
    acuR = 0
    latenMaxR = 0
    latenMinR = 0
    latenMedR = 0
    return Response(status=200)


@app.route('/resetMeasure', methods=['POST'])
def reset_M():
    global contM, acuM, latenMaxM, latenMinM, latenMedM
    contM = 0
    acuM = 0
    latenMaxM = 0
    latenMinM = 0
    latenMedM = 0
    return Response(status=200)


@app.route('/reset', methods=['POST'])
def reset_T():
    global contT, contR, acuR, latenMaxR, latenMinR, latenMedR, contM, acuM, latenMaxM, latenMinM, latenMedM
    contR = 0
    acuR = 0
    latenMaxR = 0
    latenMinR = 0
    latenMedR = 0
    contM = 0
    acuM = 0
    latenMaxM = 0
    latenMinM = 0
    latenMedM = 0
    contT = 0
    lastM = ''
    lastQC = ''
    lastHeaderSrv = ''
    lastStatus = ''
    lastInfo = ''
    last_id = ''
    return Response(status=200)

@app.route('/stats', methods=['GET'])
def treat_stats():
    global latenMaxR, latenMinR, latenMedR, latenMaxM, latenMinM, latenMedM
    statistics = ''
    statistics += 'MaxR: '
    statistics += str(latenMaxR)
    statistics += ', MinR: '
    statistics += str(latenMinR)
    statistics += ', MedR: '
    statistics += str(latenMedR)
    statistics += ', MaxM: '
    statistics += str(latenMaxM)
    statistics += ', MinM: '
    statistics += str(latenMinM)
    statistics += ', MedM: '
    statistics += str(latenMedM)
    return Response(response=statistics, status=200)

@app.route(auth_token, methods=['GET', 'POST'])
def treat_auth_token():

    mydata = ''
    token_from_trust = 'token-from-trust-request'
    token_from_pep = 'token-from-pep'
    auth_response = ''
    status_ = 200
    fiware_service = 'TestService'
    fiware_servicepath = '/TestSubservice'
    resp_aux = Response()
    if ((status_ == 200) and (request.method == 'POST') and (request.data is not None) and (len(request.data) != 0)):
        mydata = request.data
        json_data = json.loads(request.data)
        who_is = json_data['auth']['identity']['password']['user']['name']
        if (who_is == 'iotagent'):
            token = token_from_trust
        elif (who_is == 'pep'):
            token = token_from_pep
        else:
            status_ = 401
            auth_token = 'No pep no trust'
        if (status_ == 200):
            resp_aux.headers.add('X-Subject-Token', token)
    elif ((status_ == 200) and (request.method == 'GET')):
        if ('X-Auth-Token' in request.headers):
            if (request.headers['X-Auth-Token'] == 'expired-token'):
                status_ = 401
                auth_response = 'expired-token'
        else:
            status_ = 401 
            auth_response = 'No X-Auth-Token header'
        # Get user info
        if ('X-Subject-Token' in request.headers):
            user_token = request.headers['X-Subject-Token']
            user_from_token = user_token.split('-')
            user = '{ "token": { "issued_at": "2014-10-06T08:20:13.484880Z", "extras": {}, "methods": [ "password" ], "expires_at": "2014-10-06T09:20:13.484827Z", "user": { "domain": { "id": "' + auth_domains[fiware_service]+'", "name": "'+fiware_service+'" }, "id": "'+auth_users[user_from_token[1]]['id']+'", "name": "'+user_from_token[1]+'" } } }'
            print user
            if (status_ == 200):
                auth_response = user
        else:
            status_ = 401 
            auth_response = 'No X-Subject-Token header'
    else:
			print 'Me cago en to'


    if verbose:
       print mydata
       #print cmd_name
       #print command_fields[0]
       print auth_response 
    print auth_response
    return Response(response=auth_response, status=status_, content_type='text/plain;charset=UTF-8', headers=resp_aux.headers)

@app.route(auth_roles, methods=['GET', 'POST'])
def treat_auth_roles():

    mydata = ''
    token_from_trust = 'token-from-trust-request'
    token_from_pep = 'token-from-pep'
    auth_response = ''
    status_ = 200
    fiware_service = ''
    fiware_servicepath = ''
    resp_aux = Response()
    if ('X-Auth-Token' in request.headers):
        if (request.headers['X-Auth-Token'] == 'expired-token'):
            status_ = 401
            auth_response = 'expired-token'
    else:
        status_ = 401 
        auth_response = 'No X-Auth-Token header'

    if ((status_ == 200) and (request.method == 'POST') and (request.data is not None) and (len(request.data) != 0)):
        mydata = request.data
        json_data = json.loads(request.data)
        who_is = json_data['auth']['identity']['password']['user']['name']
        print who_is
        if (who_is == 'iotagent'):
            token = token_from_trust
        elif (who_is == 'pep'):
            token = token_from_pep
        else:
            status_ = 401
            auth_token = 'No pep no trust'
        if (status_ == 200):
            resp_aux.headers.add('X-Subject-Token', token)
    elif ((status_ == 200) and (request.method == 'GET')):
        if ('user.id' in request.args):
            user_id = request.args['user.id']
            # Get domain and project and role
            project = ''
            domain = ''
            role = ''
            for user in auth_users:
                if (auth_users[user]['id'] == user_id):
                    project = auth_users[user]['projects'][0]
                    role = auth_users[user]['roles'][0]
            user_role = '{"role_assignments": [{"scope": {"project": {"id": "' + project +'"}},"role": {"id": "' + role + '"},"user": {"id": "' + user_id +'"},"links": {"assignment": "puthere"}}],"links": {"self": "http://${KEYSTONE_HOST}/v3/role_assignments","previous": null,"next": null}}'
            print user_role 
            if (status_ == 200):
                auth_response = user_role
        else:
            status_ = 401 
            auth_response = 'No user.id parameter'


    if verbose:
       print mydata
       #print cmd_name
       #print command_fields[0]
       print auth_response 

    return Response(response=auth_response, status=status_, content_type='text/plain;charset=UTF-8')

@app.route(auth_projects, methods=['GET', 'POST'])
def treat_auth_projects():

    mydata = ''
    token_from_trust = 'token-from-trust-request'
    token_from_pep = 'token-from-pep'
    auth_response = ''
    status_ = 200
    fiware_service = ''
    fiware_servicepath = ''
    resp_aux = Response()
    if ('X-Auth-Token' in request.headers):
        if (request.headers['X-Auth-Token'] == 'expired-token'):
            status_ = 401
            auth_response = 'expired-token'
    else:
        status_ = 401 
        auth_response = 'No X-Auth-Token header'

    if ((status_ == 200) and (request.method == 'POST') and (request.data is not None) and (len(request.data) != 0)):
        mydata = request.data
        json_data = json.loads(request.data)
        who_is = json_data['auth']['identity']['password']['user']['name']
        print who_is
        if (who_is == 'iotagent'):
            token = token_from_trust
        elif (who_is == 'pep'):
            token = token_from_pep
        else:
            status_ = 401
            auth_token = 'No pep no trust'
        if (status_ == 200):
            resp_aux.headers.add('X-Subject-Token', token)
    elif ((status_ == 200) and (request.method == 'GET')):
        if ('domain_id' in request.args):
            domain_id = request.args['domain_id']
            # Get domain and project and role
            project = '1'
            project_name = '/TestSubservice'
            projects = '{"projects": [{"description": "Subservices","links": {"self": "http://${KEYSTONE_HOST}/v3/projects/1"},"enabled": true,"id": "' + project + '","domain_id": "' + domain_id + '","name": "'+ project_name +'"}]}'
            if (status_ == 200):
                auth_response = projects 
        else:
            status_ = 401 
            auth_response = 'No domain_id parameter'


    if verbose:
       print mydata
       #print cmd_name
       #print command_fields[0]
       print auth_response 

    return Response(response=auth_response, status=status_, content_type='text/plain;charset=UTF-8')

@app.route(auth_access_control, methods=['GET', 'POST'])
def treat_access_control():
    status_ = 200
    fiware_service = 'other'
    fiware_servicepath = ''
    resp_aux = Response()
    decision = 'Permit'
    if ((status_ == 200) and (request.method == 'POST') and (request.data is not None) and (len(request.data) != 0)):
       mydata = request.data
       print request.data
       if (not 'TestService' in request.data):
          status_ = 403
          decision = 'Deny'
    else:
        status_ = 403
        decision = 'Deny'
    auth_response = '<?xml version="1.0" encoding="UTF-8"><Response xmlns="urn:oasis:names:tc:xacml:3.0:core:schema:wd-17" xmlns:xsi="http://www.w3.."><Result><Decision>'+ decision +'</Decision></Result></Response>'


    if verbose:
       print mydata
       #print cmd_name
       #print command_fields[0]
       print auth_response 

    return Response(response=auth_response, status=status_, content_type='text/plain;charset=UTF-8')

def write_pids():
    mypid = os.getpid()
    spid = str(mypid)
    #f = open('/var/run/iot/simuCB.pid', 'w')
    f = open('simcb.pid', 'w')
    f.write(spid)
    f.close()

class FlowRepsolModbus:
    def __init__(self, obj):
        try:
            self.thread = threading.Thread(target=self.start, args=(obj['master'],))
            self.device = obj['device']
            self.num = obj['measures']  
            self.operation = 0
            self.operations = [
                '',
                '01 06 02 DE 00 01',                
                '01 03 02 00 05',
                '',
                '01 03 02 00 01'
                ]
            self.calcDevice()
            self.calcTime()
            self.thread.start()
        except Exception, e:
            print e

    def calcTime(self):
        now = time.localtime(time.time() + 60*5*self.num)
        ddmm = str(now.tm_mday).zfill(2)
        ddmm += str(now.tm_mon).zfill(2)
        aahh = str(now.tm_year)[-2:]
        aahh += str(now.tm_hour).zfill(2)
        mmss = str(now.tm_min).zfill(2)
        mmss += str(now.tm_sec).zfill(2)
        time_hex = format(int(ddmm), '04x')
        time_hex += format(int(aahh), '04x')
        time_hex += format(int(mmss), '04x')
        modbus = "010306" + time_hex
        self.operations[3] = modbus
        for i in range (0, self.num):
            self.operations.append('01 03 2C 00 01 00 AE 00 D5 00 DA 00 DD 00 E4 00 00 00 49 00 4C 00 4C 00 42 00 50 00 38 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 06')
        self.operations.append('01 06 02 DE 00 00')   
    
    def calcDevice(self):
        """8 digits to 4 positions"""
        self.operations[0] = '010308' + self.device.encode("hex")


    def calcByte(self, ch, crc):
        """Given a new Byte and previous CRC, Calc a new CRC-16"""
        if type(ch) == type("c"):
            by = ord( ch)
        else:
            by = ch
        crc = (crc >> 8) ^ table[(crc ^ by) & 0xFF]
        return (crc & 0xFFFF)
 
    def calcString(self, st, crc):
        """Given a bunary string and starting CRC, Calc a final CRC-16 """
        for ch in st:
            crc = (crc >> 8) ^ table[(crc ^ ord(ch)) & 0xFF]
        high, low = divmod(crc, 0x100)
        a = binascii.b2a_hex(st)
        a += format(low, '02x')
        a += format(high, '02x')
        return binascii.unhexlify(a)

    def start(self, endpoint):
        server_address = endpoint.split(':')
        ip = server_address[0]
        port = server_address[1]
         
        try:            
            self.sock = socket.create_connection((ip, int(port)), 3) 
            while (self.operation < len(self.operations)):
                self.sock.recv(512)
                op = self.operations[self.operation]
                op_hex_data = binascii.unhexlify(op.replace(" ", ""))
                self.sock.sendall(self.calcString(op_hex_data, INITIAL_MODBUS))
                self.operation += 1
        except Exception, e:
            print e
            self.sock.shutdown()
            
        
@app.route(telegestion, methods=['GET', 'POST'])
def treat_telegestion():

    mydata = ''
    
    if ((request.data is not None) and (len(request.data) != 0)):
        mydata = request.data
    try:
        obj = json.loads(mydata)
        telegestion_devices[obj['device']] = FlowRepsolModbus(obj)
    except Exception, e:
        print e
    return Response(response="", status=200, content_type='text/plain;charset=UTF-8')

# Globals
contR = 0
contM = 0
contT = 0
acuR = 0
acuM = 0
latenMaxR = 0
latenMinR = 0
latenMedR = 0
latenMaxM = 0
latenMinM = 0
latenMedM = 0
lastM = ''
lastQC = ''
lastHeaderSrv = ''
lastStatus = ''
lastInfo = ''
last_id = ''
mut = threading.Lock()

if __name__ == '__main__':
    write_pids()
    #app.run(host=host, port=port, debug=True)
    app.run(host=host, port=port)
