import requests
import time
import sys

import spotipy
import spotipy.util as util

import credentials

input("ENTER to start")
for i in range(10, 0, -1):
    print(i)
    time.sleep(1)


# State of Phone: 0->Not lifted, 1->Lifted
state = 1

# Set up for Spotipy
token = util.prompt_for_user_token('moaromnoms', 
            scope='user-modify-playback-state user-read-playback-state', 
            client_id=credentials.c_id, 
            client_secret=credentials.c_secret, 
            redirect_uri='http://localhost/')
sp = spotipy.Spotify(auth = token)

# Spotipy select device
d_list = sp.devices()['devices']
d_id = d_list[0]['id']


# Main Loop
while True:
    try:
        # Get Arduino Data
        info = requests.post("http://127.0.0.1:8080/serial.dat").content
        if not info:
            continue

        if b"LIFTED" in info:
            if state == 0:
                print("PAUSE")
                sp.pause_playback(device_id = d_id)
            state = 1
        
        if b"DOCKED" in info:
            if state == 1:
                print("PLAY")
                sp.start_playback(device_id = d_id)
            state = 0
    
    except KeyboardInterrupt:
        print("Stopping")
        sys.exit(0)
    except:
        pass