import constants
import fountain
import queries
import sys
from multiprocessing import Process
from api import startAPI
from time import sleep


def backgroundProcessing():
    """
    This method is called to initiate the background processing of the fountain, which includes advancing the control
    queue, running the patterns, and sending the current state to the cRIO.
    """

    # Now supposedly, SQLite3 Python bindings allow for this kind of multithreading without any special locking/mutex
    # effort on our part, as long as we don't re-use the same connection object across different threads. That is, the
    # following should be entirely thread-safe ("process" safe), and any modifying queries will automagically wait on
    # existing queries to finish!

    tablesChecked = False

    # Main background processing loop.
    while True:
        sleep(1)

        # First, check if a control queue exists. If it doesn't, create it.
        if not tablesChecked:
            print("Checking if controlQueue exists...")

            con = fountain.db_connect()
            c = con.cursor()
            c.execute(queries.CHECK_IF_CONTROL_QUEUE_EXISTS)

            r = c.fetchone()
            fountain.db_close(con)

            if r[0] == 0:
                print("... it doesn't. Creating and populating default tables, if they don't exist...")
                fountain.db_createTables()
                fountain.db_loadDefaults()
                print("... OK.")
            else:
                print("... it does.")

            tablesChecked = True
            print("Background processing started...")

        # The first task is to check our database to see if any items in the queue are pending assignment (with position
        # as -1). Take these items and add them to their corresponding priority queues. When an item reaches the front of
        # the queue, update its acquire time from -1 to the current time.
        con = fountain.db_connect()
        c = con.cursor()
        c.execute(queries.FIND_PENDING_CONTROL_REQUESTS)

        for row in c.fetchall():
            print(row)
            # TODO: add to appropriate queue and set queue position

        fountain.db_close(con)


        # TODO: Check if currently running request needs to be booted out due to either expiring or a higher priority
        # request, and if so, update queuePosition and acquire times on all affected requests.
        # If a request becomes invalid, set its TTL to 0 and queuePosition to -2.

        # TODO: Here is where we'd check if a pattern is running and increment that pattern, and then either way send the
        # new states to the cRIO. Also performs other database tasks like scheduling jobs in the control request queue.


# The backend is threaded - one thread (which we will start and spin off) takes care of the API hook
# and associated interaction, while the main thread then proceeds to run periodic tasks (like updating the running
# pattern, clearing old control queues, sending events to the cRIO).

if __name__ == '__main__':
    print("Enlight backend version " + constants.VERSION)

    print("Spawning API hook process...")
    p = Process(target=startAPI)
    p.start()

    print('Starting background processing...')
    p = Process(target=backgroundProcessing)
    p.start()