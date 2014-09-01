import constants
import fountain

import queries
import sys
from multiprocessing import Process
from api import startAPI
from time import sleep, time


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
    controlledBy = -1  # The current controllerID which should be allowed to control the fountain. -1 if in patterns.

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

        rows = c.fetchall()

        # Only bother if there are actually rows..
        if len(rows) > 0:

            # Find maximum queue position for each priority. The queue position is the 1th entry in the row.
            nextQueuePositionForPriority = {}
            for row in rows:
                if not row[1] in nextQueuePositionForPriority.keys():
                    c.execute(queries.FIND_MAX_QUEUE_POSITION_FOR_PRIORITY, {'priority': row[1]})
                    max = c.fetchone()[0]

                    if max is None or max < 0:
                        max = -1

                    nextQueuePositionForPriority[row[1]] = max + 1

            # Now, while incrementing the next queue positions, add these things to the database.
            for row in rows:
                print('Queueing cID ' + str(row[0]) + ' as position ' + str(
                    nextQueuePositionForPriority[row[1]]) + ' in priority ' + str(row[1]) + ' queue.')

                c.execute(queries.SET_QUEUE_POSITION,
                          {'controllerID': row[0], 'queuePosition': nextQueuePositionForPriority[row[1]]})

                # If a controller becomes in control, set its acquired time to now.
                if nextQueuePositionForPriority[row[1]] == 0:
                    c.execute(queries.SET_CONTROL_ACQUIRED_TIME_TO_NOW, {'controllerID': row[0]})

                nextQueuePositionForPriority[row[1]] += 1

        # Check if currently running request needs to be booted out due to either expiring or a higher priority request,
        # and if so, update queuePosition and acquire times on all affected requests.
        # If a request becomes invalid, set its TTL to 0 and queuePosition to -2.

        # First, find the controller who is currently in control. Find highest priority, and then check that queue.
        c.execute(queries.FIND_MAX_PRIORITY_IN_QUEUE)
        max = c.fetchone()[0]

        if max is None:
            # Nothing's in the queue!
            controlledBy = -1

        # See if there's a 0th-position item in this queue. If so, check the time to make sure it's still valid. If it's
        # not, or there is no such item, go through the rest of this queue until we find a valid record to set as the
        # new controller. If we can't find one (because we run out or a bunch of TTL = 0 records exist), drop down to the
        # next priority level and start from there (i.e. re-call the max priority finding query). If we have run out of
        # control requests completely, set controlledBy to -1 so the defailt patterns can engage.
        discoveringInvalidItems = True  # At first glance seems unnecessary, but there could be junk records (TTL = 0)
        while discoveringInvalidItems and controlledBy != -1:
            # During this loop, we might have to drop down a priority level. We might even run out of valid items
            # entirely - in which case we'll need to set controlledBy to -1.

            # We need to check the queue of valid items at the maximum priority level.
            for row in c.execute(queries.GET_QUEUE_AT_PRIORITY, {'priority': max}):
                # These are ordered with the highest priority first. Check the times until we find a valid item. If we
                # find an invalid item, set its queuePosition to -2 and set the next valid item in the queue to position
                # 0, with an acquire time of now. Astute readers will note that this will result in non-contiguous
                # queuePositions for the requests, but this does not matter as we treat the request list as a queue,
                # only adding items to the max + 1 and only taking items off the front.

                # First, check if it's queuePosition is 0. If it's not, it likely has an acquire time of -1 and needs to
                # be scheduled anyway (providing its TTL > 0, otherwise kill that record). We could run into records
                # waiting in the queue whose owners have released control (given up on) and their TTLs will be 0.
                if row[2] == 0:
                    # Currently in control, check its validity.
                    if row[0] + row[1] > time():
                        # Still valid, set this to be the controller
                        print("controllerID " + str(row[3]) + " remains in control.")
                        controlledBy = row[3]
                        discoveringInvalidItems = False
                        break
                    else:
                        # Invalid, need to clear it and continue.

                        # XXX: So, this is a query modifying the controlQueue while we're currently iterating through it
                        # I don't know if Python/SQLite bindings are smart enough to figure this one out, it might be a
                        # problem...
                        c.execute(queries.SET_QUEUE_POSITION, {'controllerID': row[3], 'queuePosition': -2})

                        # Drop out to the for loop and we'll now find some non-0 position items which will be promoted.
                        continue #  TODO: If it does work, this is really easy. If not, that's a lot more work.
                else:
                    # Check that this item at least has a TTL > 0
                    if not row[1] > 0:
                        # Done with that request...
                        c.execute(queries.SET_QUEUE_POSITION, {'controllerID': row[3], 'queuePosition': -2})
                        continue

                    # This item needs to be promoted to the front of the queue.
                    c.execute(queries.SET_QUEUE_POSITION, {'controllerID': row[3], 'queuePosition': 0})
                    print("New controllerID in control: " + str(row[3]))
                    discoveringInvalidItems = False
                    break

            # Now, since we got here, we know that we haven't found a valid item at this priority, having gone through
            # all of them. We need to drop down to the next priority level, or decide that we're completely done if we
            # have exhausted all priority levels in the queue.
            c.execute(queries.FIND_MAX_PRIORITY_IN_QUEUE)
            max = c.fetchone()[0]

            if max is None:
                # Nothing's in the queue!
                print("Queue is empty, returning control to patterns.")
                controlledBy = -1

        # Advance patterns if nothing else is in control.
        if controlledBy == -1:
            # Default patterns should be able to run here
            print('Resume or start pattern due to lack of other control')
            # TODO: pattern stuff

        # TODO: Send valve data to cRIO

        # Will need state tracking here as well as raw udp stuff

        fountain.db_close(con)


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