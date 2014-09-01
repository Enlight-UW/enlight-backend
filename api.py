from bottle import error, get, post, run, request
import fountain
import queries
import constants

# ######################################################################################################################
# Defaults and Errors
# ######################################################################################################################

@error(404)
def error404(error):
    return "404"


@post('/api')
def gDefaultResponse():
    """The default route for the API; returns the version number."""
    getAPIKeyPriority()
    return {'success': 'true', 'apiVersion': '1B'}


# ###
# Helper Functions
# ###
def log(msg):
    """Prints a message alongside the IP of the client that generated it."""
    print('[' + request.remote_addr + '] ' + msg)


def checkAPIKey():
    """Verifies the requesting API key. These need to be verified on POST requests."""
    if not 'apikey' in request.json.keys():
        log('No API key with request.')
        return False

    log("Checking API key: " + request.json['apikey'])

    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.QUERY_API_KEY_COUNT, {'key': request.json['apikey']})

    # This query is an aggregation (so don't need None check) - the number of matching keys. Should be > 0 (and probably 1).
    r = c.fetchone()

    isValid = True if r[0] == 1 else False
    fountain.db_close(con)

    if (isValid):
        log("API key is valid.")
    else:
        log("Invalid API key.")
    return isValid


def getAPIKeyPriority():
    """Gets the priority of the requesting key."""
    if not 'apikey' in request.json.keys():
        log('No API key with request')
        return 0

    log('Getting priority for API key: ' + request.json['apikey'])
    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.QUERY_API_KEY_PRIORITY, {'key': request.json['apikey']})

    # We'll just get a single number back from this one.
    r = c.fetchone()
    if r is None:
        log('No rows returned - invalid API key?')
        return 0

    return r[0]


def getAPIKeyFail():
    """Returns a dictionary containing a failure status and message indicating an API key authentication failure."""
    return {'success': 'false', 'message': 'Invalid API key.'}


def jsonRow(cursor):
    """Takes the next row from the cursor and formats it as a dictionary (JSON object as far as Bottle is concerned)."""
    fields = [f[0] for f in cursor.description]

    r = cursor.fetchone()
    if r is not None:
        i = 0
        newDict = {}
        for f in fields:
            newDict[f] = r[i]
            i += 1
        return newDict
    else:
        return None


def jsonRows(cursor):
    """Queries a cursor and returns its rows as a JSON response (a list of dictionaries contained in a JSON object)."""
    responseDict = {'success': 'true', 'items': []}

    if cursor is None:
        return responseDict

    jr = jsonRow(cursor)

    while jr is not None:
        responseDict['items'].append(jr)
        jr = jsonRow(cursor)

    return responseDict


def getTrueQueuePosition(controllerID):
    """Taking into account priority and acquisition time, determines the true queue position"""
    estimate = -1
    log("Estimating queue position for cID " + str(controllerID))

    # This will look very similar to the code in the background processing script, but here we'll be counting how many
    # controllerIDs are ahead of us.
    con = fountain.db_connect()
    c = con.cursor()

    # We need to check the queue of valid items at the maximum priority level, and all these below it. Group the
    # priority levels and descend them to estimate how long it will be.
    for r in c.execute(queries.GET_PRIORITY_LEVELS):
        for row in c.execute(queries.GET_QUEUE_AT_PRIORITY, {'priority': r[0]}):
            # These are ordered with the highest priority first. Check the times until we find a valid item.
            estimate += 1

            # Need to count everything at each descending priority until we find our controllerID.
            if row[3] == controllerID:
                # Done!
                fountain.db_close(con)
                log("Determined estimate of " + str(estimate))
                return estimate

    log("Fell out of loop with estimate of " + str(estimate))
    fountain.db_close(con)
    return estimate

# ######################################################################################################################
# Authentication and Control
# ######################################################################################################################

@get('/api/control/query')
def gQueryControl():
    """Returns the current control queue."""

    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.QUERY_CONTROL_QUEUE)

    res = jsonRows(c)

    fountain.db_close(con)
    return res


@post('/api/control/query')
def pQueryControl():
    """Returns info about the 'true' queue position of this controllerID. If this is 0, you are in control."""
    if not checkAPIKey():
        return getAPIKeyFail()
    if not 'controllerID' in request.json.keys():
        return {'success': 'false', 'message': 'Must specify controllerID to query.'}

    # TODO: Implement ETA too
    return {'success': 'true', 'trueQueuePosition': getTrueQueuePosition(request.json['controllerID']), 'eta': '15'}


@post('/api/control/request')
def pRequestControl():
    """
    Requests control for an API key and returns a controllerID to the user which they should watch to see when they gain
    control.
    """
    if not checkAPIKey():
        return getAPIKeyFail()

    if not 'requestedLength' in request.json.keys():
        return {'success': 'false', 'Message': 'Must specify requested length for control.'}

    con = fountain.db_connect()
    c = con.cursor()

    c.execute(queries.REQUEST_CONTROL, {'priority': getAPIKeyPriority(), 'ttl': request.json['requestedLength'],
                                        'apikey': request.json['apikey']})

    # No concurrency worries with lastrowid, I think, as it's per-connection.
    controllerID = c.lastrowid
    fountain.db_close(con)

    # TODO: Implement variable length TTL part and some additional sanity checks.
    return {'success': 'true', 'ttl': request.json['requestedLength'], 'controllerID': controllerID}


@post('/api/control/release')
def pReleaseControl():
    """Releases the control from a specific controllerID."""
    if not checkAPIKey():
        return getAPIKeyFail()

    if not 'controllerID' in request.json.keys():
        return {'success': 'false', 'message': 'Must specify controllerID to release.'}

    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.RELEASE_CONTROL, {'controllerID': request.json['controllerID']})
    fountain.db_close(con)

    return {'success': 'true', 'message': 'Control released.'}


# ######################################################################################################################
# Valve Interaction
# ######################################################################################################################

@get('/api/valves')
def gValves():
    """Queries the valve descriptions and current valve states."""
    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.QUERY_VALVES)

    res = jsonRows(c)

    fountain.db_close(con)
    return res


@post('/api/valves')
def pValves():
    """Updates the valves based on a bitmask."""
    if not checkAPIKey():
        return getAPIKeyFail()

    if not 'controllerID' in request.json.keys():
        return {'success': 'false', 'message': 'Must specify controllerID to set valves.'}

    if not 'bitmask' in request.json.keys():
        return {'success': 'false', 'message': 'Must specify bitmask to set valves.'}

    bm = request.json['bitmask']  # Store bitmask so we can shift it around while reading out the valve states
    con = fountain.db_connect()
    c = con.cursor()

    for i in range(1, constants.NUM_VALVES + 1):
        val = bm & 1
        c.execute(queries.SET_VALVE, {'spraying': val, 'id': i})
        bm >>= 1

    fountain.db_close(con)
    return {'success': 'true'}


@get('/api/valves/<id>')
def gValvesID(id):
    """Queries a specific valve."""
    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.QUERY_VALVE, {'id': id})

    res = jsonRow(c)

    fountain.db_close(con)

    if res is None or len(res) == 0:
        return {'success': 'false', 'message': 'Invalid valve ID.'}
    else:
        return res


@post('/api/valves/<id>')
def pValvesID(id):
    """Updates a single valve based on ID."""
    if not checkAPIKey():
        return getAPIKeyFail()

    if not 'controllerID' in request.json.keys():
        return {'success': 'false', 'message': 'Must specify controllerID to set valves.'}

    if not 'spraying' in request.json.keys():
        return {'success': 'false', 'message': 'Must specify spraying value to set.'}

    con = fountain.db_connect()
    c = con.cursor()

    # TODO: check control

    c.execute(queries.SET_VALVE, {'spraying': request.json['spraying'], 'id': id})
    fountain.db_close(con)

    return {'success': 'true'}


# ######################################################################################################################
# Pattern Management
# ######################################################################################################################

@get('/api/patterns')
def gPatterns():
    """Queries the known, enabled patterns."""
    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.QUERY_PATTERNS)

    res = jsonRows(c)

    fountain.db_close(con)
    return res


@post('/api/patterns/<id>')
def pPatternsID(id):
    """Sets a specific pattern to active."""
    if not checkAPIKey():
        return getAPIKeyFail()

    if not 'controllerID' in request.json.keys():
        return {'success': 'false', 'message': 'Need to provide controllerID.'}

    # TODO: check for control
    # TODO: also disengage other patterns from playing (probably a separate query to run before this one)

    con = fountain.db_connect()
    c = con.cursor()
    c.execute(queries.ENGAGE_PATTERN, {'id': id})
    fountain.db_close(con)

    return {'success': 'true'}


# ######################################################################################################################
# Database Debugging
# ######################################################################################################################

@get('/db/drop')
def gDBDrop():
    """Drop the known tables in the database."""
    fountain.db_dropTables()
    return "ok"


@get('/db/pop')
def gDBPop():
    """Populate the database with the default values."""
    fountain.db_createTables()
    fountain.db_loadDefaults()
    return "ok"


def startAPI():
    """Starts the server API."""
    print("API hook started...")
    run(host='enlight.club', port=8082)