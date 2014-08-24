# #####
# Internal functions to the fountain. Most of these get called by the API in some way.
# ########
import sqlite3
import queries
import constants

def db_connect():
    """Opens a connection to the database and returns the connection object."""
    return sqlite3.connect(constants.DB_FILENAME)

def db_close(con):
    """Closes any open connection to the database."""
    # I guess every connection with the DB-API 2.0 bindings is treated like a transaction, so commit it.
    con.commit()
    con.close()

def db_createTables():
    """Creates the default tables in the database."""
    con = db_connect()
    c = con.cursor()
    c.execute(queries.CREATE_TABLE_APIKEYS)
    c.execute(queries.CREATE_TABLE_CONTROLQUEUE)
    c.execute(queries.CREATE_TABLE_PATTERNDATA)
    c.execute(queries.CREATE_TABLE_PATTERNS)
    c.execute(queries.CREATE_TABLE_VALVES)
    db_close(con)
    
def db_dropTables():
    """Drops all the known tables in the database, useful for debugging."""
    con = db_connect()
    c = con.cursor()
    c.execute(queries.DROP_TABLE_APIKEYS)
    c.execute(queries.DROP_TABLE_CONTROLQUEUE)
    c.execute(queries.DROP_TABLE_PATTERNDATA)
    c.execute(queries.DROP_TABLE_PATTERNS)
    c.execute(queries.DROP_TABLE_VALVES)
    db_close(con)

def db_loadDefaults():
    """Load the default valves and API test keys into the database."""
    con = db_connect()
    c = con.cursor()
    c.executescript(queries.INSERT_DEFAULT_TEST_KEYS)
    c.executescript(queries.INSERT_DEFAULT_VALVES)
    db_close(con)

