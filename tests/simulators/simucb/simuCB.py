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
from array import *

app = Flask(__name__)

# Default arguments
port = 7010
host='0.0.0.0'
register_context_url = '/NGSI9/registerContext'
update_context_url = '/NGSI10/updateContext'
query_context_url = '/NGSI10/queryContext'
client_ul20_command = '/simulaClient/ul20Command'
auth_token = '/v3/auth/tokens'
auth_roles = '/v3/role_assignments'
auth_projects = '/v3/projects'
auth_access_control = '/'
auth_domains = {'TestService': '1'}
auth_prjs = {'/TestSubservice': '1'}
auth_users = dict()
auth_users['alice'] = {'id': '1', 'token': 'token-alice', 'domains': ['1'], 'projects': ['1'], 'roles': ['1']} 

roles = {'1': 'admin', '2': 'user'}
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
             #lastStatus = status_value
             lastStatus = lastM
         elif ( attr_list [i] ["name"].endswith ('_info')):
             info_value = attr_list [i] ["value"]
             #lastInfo = info_value
             lastInfo = lastM

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
