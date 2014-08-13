from bottle import error, get, post, run, request
import fountain
import queries

# ######################################################################################################################
# Defaults and Errors
# ######################################################################################################################

@error(404)
def error404(error):
    return "404"

@post('/api')
def gDefaultResponse():
    "The default route for the API; returns the version number."
    getAPIKeyPriority()
    return {'success': 'true', 'apiVersion': '1B'}


# ###
# Helper Functions
# ###
def log(msg):
    "Prints a message alongside the IP of the client that generated it."
    print('[' + request.remote_addr + '] ' + msg)

def checkAPIKey():
    "Verifies the requesting API key. These need to be verified on POST requests."
    if not 'apikey' in request.json.keys():
        log('No API key with request.')
        return False

    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.QUERY_API_KEY_COUNT, {'key': request.json['apikey']})
    # This query is an aggregation (so don't need None check) - the number of matching keys. Should be > 0 (and probably 1).
    r = c.fetchone()

    isValid = True if r[0] == 1 else False
    fountain.db_close(con)
    return isValid

def getAPIKeyPriority():
    "Gets the priority of the requesting key."
    if not 'apikey' in request.json.keys():
        log('No API key with request')
        return 0

    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.QUERY_API_KEY_PRIORITY, {'key': request.json['apikey']})

    # We'll just get a single number back from this one.
    r = c.fetchone()
    if r is None:
        log('No rows returned - invalid API key?')
        return 0

    return r[0]

def getTrueQueuePosition():
    "Taking into account priority and acquisition time, determines the true queue position"



# ######################################################################################################################
# Authentication and Control
# ######################################################################################################################

@get('/api/control/query')
def gQueryControl():
    return {'success': 'false', 'message': 'Not yet implemented.'}


@post('/api/control/query')
def pQueryControl():
    return {'success': 'false', 'message': 'Not yet implemented.'}


@post('/api/control/request')
def pRequestControl():
    return {'success': 'false', 'message': 'Not yet implemented.'}


@post('/api/control/release')
def pReleaseControl():
    return {'success': 'false', 'message': 'Not yet implemented.'}


# ######################################################################################################################
# Valve Interaction
# ######################################################################################################################

@get('/api/valves')
def gValves():
    return {'success': 'false', 'message': 'Not yet implemented.'}


@post('/api/valves')
def pValves():
    return {'success': 'false', 'message': 'Not yet implemented.'}


@get('/api/valves/<id>')
def gValvesID(id):
    return {'success': 'false', 'message': 'Not yet implemented.'}


@post('/api/valves/<id>')
def pValvesID(id):
    return {'success': 'false', 'message': 'Not yet implemented.'}


# ######################################################################################################################
# Pattern Management
# ######################################################################################################################

@get('/api/patterns')
def gPatterns():
    return {'success': 'false', 'message': 'Not yet implemented.'}


@post('/api/patterns/<id>')
def pPatternsID(id):
    return {'success': 'false', 'message': 'Not yet implemented.'}


# ######################################################################################################################
# Database Debugging
# ######################################################################################################################

@get('/db/drop')
def gDBDrop():
    fountain.db_dropTables()
    return "ok"


@get('/db/pop')
def gDBPop():
    fountain.db_createTables()
    fountain.db_loadDefaults()
    return "ok"


run(host='localhost', port=8080)