# ###
# This file contains all of the query statements for the Enlight API. Some of these queries have fields named - these
# are meant to be filled via prepared statements.
# ###

# #############
# Table schemas
# #############

# A table of API keys. They have a name (like the name of the program it's
# registered to), the actual API key (alphanumeric string), a priority (higher
# priorities will be able to take over), a creation date (epoch timestamp), and
# a boolean enabled value (0 = disabled or revoked).
CREATE_TABLE_APIKEYS = """
    CREATE TABLE IF NOT EXISTS apikeys (
         apikey TEXT PRIMARY KEY,
         name TEXT NOT NULL,
         priority INTEGER NOT NULL,
         date INTEGER NOT NULL,
         enabled INTEGER DEFAULT 1 NOT NULL
     )
"""

# A table of api keys which are currently controlling or in queue for
# controlling the fountain. Each has a controllerID associated with it which
# represents this unique instance of fountain control (unique for a while, at
# least). Aquired and expires timestamps determine a period of control, and a
# numeric priority field determines who should be in control. Queue positions
# are tracked here as well.
CREATE_TABLE_CONTROLQUEUE = """
    CREATE TABLE IF NOT EXISTS controlQueue (
        controllerID INTEGER PRIMARY KEY AUTOINCREMENT,
        acquire INTEGER NOT NULL,
        ttl INTEGER NOT NULL,
        priority INTEGER NOT NULL,
        queuePosition INTEGER NOT NULL,
        apikey REFERENCES apikeys (apikey)
    )
"""

# A table of valves which describe the interactive elements of the fountain.
# Each valve has a numeric ID, a name, a description, and boolean enabled and
# spraying states.
CREATE_TABLE_VALVES = """
   CREATE TABLE IF NOT EXISTS valves (
        ID INTEGER PRIMARY KEY,
        name TEXT UNIQUE NOT NULL,
        description TEXT,
        spraying INTEGER NOT NULL,
        enabled INTEGER DEFAULT 1 NOT NULL
    )
"""

# A table of patterns which are sequences of on/off triggers for valves. Each
# pattern consists of an ID, a name, a description, an active field, and an
# enabled field.
CREATE_TABLE_PATTERNS = """
    CREATE TABLE IF NOT EXISTS patterns (
        ID INTEGER PRIMARY KEY,
        name TEXT UNIQUE NOT NULL,
        description TEXT,
        active INTEGER NOT NULL,
        enabled INTEGER DEFAULT 1 NOT NULL
    )
"""

# A table of pattern data. Each pattern has associated with it a series of
# actions, represented here. A single entry represents one action. An action
# has a pattern ID reference, an activation time (since the start of pattern),
# a valve to action, and an action to take.
CREATE_TABLE_PATTERNDATA = """
    CREATE TABLE IF NOT EXISTS patternData (
        patternID REFERENCES patterns(ID),
        time INTEGER NOT NULL,
        valve REFERENCES valves(ID),
        action INTEGER NOT NULL
    )
"""

# ###############
# Default values
#
# Eventually we'll be able to replace all the insert statements with a compound insert, but the version of SQLite3 lib
# that comes with python doesn't support that syntax yet.
# ###############

# The default entries in the valves table. If a new valve is added to the fountain,
# it should be added to this list.
INSERT_DEFAULT_VALVES = """
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (1, 'V1', 'Vertical caliper jet 1', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (2, 'V2', 'Vertical caliper jet 2', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (3, 'V3', 'Vertical caliper jet 3', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (4, 'V4', 'Vertical caliper jet 4', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (5, 'V5', 'Vertical caliper jet 5', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (6, 'V6', 'Vertical caliper jet 6', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (7, 'V7', 'Vertical caliper jet 7', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (8, 'V8', 'Vertical caliper jet 8', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (9, 'V9', 'Vertical caliper jet 9', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (10, 'V10', 'Vertical caliper jet 10', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (11, 'VC', 'Vertical caliper jet center', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (12, 'VR', 'Vertical caliper jet ring', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (13, 'H1', 'Horizontal caliper jet 1 (pointed up)', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (14, 'H2', 'Horizontal caliper jet 2', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (15, 'H3', 'Horizontal caliper jet 3', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (16, 'H4', 'Horizontal caliper jet 4', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (17, 'H5', 'Horizontal caliper jet 5', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (18, 'H6', 'Horizontal caliper jet 6', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (19, 'H7', 'Horizontal caliper jet 7', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (20, 'H8', 'Horizontal caliper jet 8', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (21, 'H9', 'Horizontal caliper jet 9', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (22, 'H10', 'Horizontal caliper jet 10', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (23, 'HC', 'Horizontal caliper jet center', 0, 1);
    INSERT OR IGNORE INTO valves (ID, name, description, spraying, enabled) VALUES
        (24, 'HR', 'Horizontal caliper jet ring', 0, 1);
"""

# Just some testing API keys. This will be removed in the future.
INSERT_DEFAULT_TEST_KEYS = """
    INSERT INTO apikeys (apikey, name, priority, date) VALUES
        ('abc123', 'Test API Key', 10, strftime('%s', 'now'));
    INSERT INTO apikeys (apikey, name, priority, date) VALUES
        ('pri20a', '20 Priority Test API Key 1', 20, strftime('%s', 'now'));
    INSERT INTO apikeys (apikey, name, priority, date) VALUES
        ('pri20b', '20 Priority Test API Key 2', 20, strftime('%s', 'now'));
    INSERT INTO apikeys (apikey, name, priority, date) VALUES
        ('pri30a', '30 Priority Test API Key 1', 30, strftime('%s', 'now'));
    INSERT INTO apikeys (apikey, name, priority, date) VALUES
        ('pri30b', '30 Priority Test API Key 2', 30, strftime('%s', 'now'));
"""

# ###
# Drop queries
#
# These straightforward drop queries should just be used for testing.
# ###

DROP_TABLE_APIKEYS = """
    DROP TABLE IF EXISTS apikeys
"""

DROP_TABLE_CONTROLQUEUE = """
    DROP TABLE IF EXISTS controlQueue
"""

DROP_TABLE_VALVES = """
    DROP TABLE IF EXISTS valves
"""

DROP_TABLE_PATTERNS = """
    DROP TABLE IF EXISTS patterns
"""

DROP_TABLE_PATTERNDATA = """
    DROP TABLE IF EXISTS patternData
"""

# ####
# API queries
# ####

# Used to verify an API key exists.
QUERY_API_KEY_COUNT = """
    SELECT COUNT(*) FROM apikeys
    WHERE apikey=:key
"""

QUERY_API_KEY_PRIORITY = """
    SELECT priority FROM apikeys
    WHERE apikey=:key
"""

# Gets the current control queue.
QUERY_CONTROL_QUEUE = """
    SELECT controllerID, acquire, ttl, priority, queuePosition, apikey
    FROM controlQueue
    -- WHERE (acquire + ttl) > strftime('%s', 'now')
"""

# Requests control by adding an entry to the queue. All entries will initially have position -1 until scheduled by the
# background job - this will make the SQL query simpler. The acquire time will also be set to -1 until it is scheduled.
# This way, it is easy to determine if a controller has finished by adding acquire + ttl and checking against current
# time.
REQUEST_CONTROL = """
    INSERT INTO controlQueue (priority, acquire, ttl, queuePosition, apikey)
    VALUES (
        :priority,
        -1,
        :ttl,
        -1,
        :apikey
    )
"""

# Releases the control of a specific user.
RELEASE_CONTROL = """
    UPDATE controlQueue
    SET ttl = 0
    WHERE controllerID=:controllerID
"""


# Get a list of the valves and their states.
QUERY_VALVES = """
    SELECT ID, name, description, spraying, enabled
    FROM valves
"""

# Query a singular valve.
QUERY_VALVE = """
    SELECT ID, name, description, spraying, enabled
    FROM valves
    WHERE ID=:id
"""

# Query the known, enabled patterns.
QUERY_PATTERNS = """
    SELECT ID, name, active, description
    FROM patterns
    WHERE enabled<>0
"""

# Engages a pattern by setting it to active.
ENGAGE_PATTERN = """
    UPDATE patterns
    SET active=1
    WHERE ID=:id and enabled<>0
"""

# Updates the spraying status of a particular valve.
SET_VALVE = """
    UPDATE valves
    SET spraying=:spraying
    WHERE ID=:id AND enabled<>0
"""

# ######################################################################################################################
# Background processing queries.
# ######################################################################################################################

# The background processing should not start until the controlQueue actually exists, so wait until it does.
CHECK_IF_CONTROL_QUEUE_EXISTS = """
    SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='controlQueue'
"""

# Discover requests that have been made, but not yet sorted into the control queue.
FIND_PENDING_CONTROL_REQUESTS = """
    SELECT controllerID, priority
    FROM controlQueue
    WHERE queuePosition = -1
"""

# Discover the current maximum position among active records in the controlQueue of a certain priority. Needs to be
# where queuePosition > -2 because we don't want to look at expired records.
FIND_MAX_QUEUE_POSITION_FOR_PRIORITY = """
    SELECT MAX(queuePosition)
    FROM controlQueue
    WHERE priority=:priority AND queuePosition > -2
"""

# Updates a control request's queue position. (Like scheduling it from -1 to an actual position.)
SET_QUEUE_POSITION = """
    UPDATE controlQueue
    SET queuePosition=:queuePosition
    WHERE controllerID=:controllerID
"""

# Updates a controllerID by setting its acquired time.
SET_CONTROL_ACQUIRED_TIME_TO_NOW = """
    UPDATE controlQueue
    SET acquire=strftime('%s','now')
    WHERE controllerID=:controllerID
"""

# Used for finding the top queue which should be served.
FIND_MAX_PRIORITY_IN_QUEUE = """
    SELECT MAX(priority)
    FROM controlQueue
    WHERE queuePosition > -2
"""

# Get a list of controllerIDs waiting in a certain priority queue, in the order that they should be served.
GET_QUEUE_AT_PRIORITY = """
    SELECT acquire, ttl, queuePosition, controllerID
    FROM controlQueue
    WHERE queuePosition > -2 AND priority=:priority
    ORDER BY queuePosition ASC
"""


GET_PRIORITY_LEVELS = """
    SELECT priority
    FROM controlQueue
    WHERE queuePosition > -1
    GROUP BY priority
    ORDER BY priority DESC
"""


CHECK_IF_IM_IN_THE_QUEUE = """
    SELECT COUNT(*)
    FROM controlQueue
    WHERE controllerID=:controllerID AND queuePosition > -1
"""