from bottle import error, get, post, run

# ######################################################################################################################
# Defaults and Errors
# ######################################################################################################################

@error(404)
def error404(error):
    return "404"


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


run(host='localhost', port=8080)