import constants
from multiprocessing import Process
from api import startAPI

# The backend is threaded - one thread (which we will start and spin off) takes care of the API hook
# and associated interaction, while the main thread then proceeds to run periodic tasks (like updating the running
# pattern, clearing old control queues, sending events to the cRIO).

# TODO: mutex database access

if __name__ == '__main__':
    print("Enlight backend version " + constants.VERSION)
    print("Spawning API hook process...")
    p = Process(target=startAPI)
    p.start()
else:
    print('Starting background processing...')
    # TODO: Here is where we'd check if a pattern is running and increment that pattern, and then either way send the
    # new states to the cRIO. Also performs other database tasks like scheduling jobs in the control request queue.