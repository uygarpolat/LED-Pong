https://github.com/user-attachments/assets/fd56c479-0efc-44e0-8df5-ec8a253fdd49

# LED Pong!

LED Pong brings the classic arcade game to a 64x32 RGB LED matrix controlled over WiFi by an ESP8266 and the Blynk mobile app.

You move each paddle in real-time with a slider in the app while scores update in value display widgets. The code also handles realistic ball physics with angled bounces and a win at ten points, all of which are editable.

To set up create a Blynk project, add two sliders on virtual pins V13 and V14 with a value range from 25 down to 1 and leave “send on release only” off. Then add two value displays on V15 and V16 to show each player’s score. Hook up your LED matrix following [Brian Lough’s LED Matrix guide](https://www.youtube.com/watch?v=YvU_ZfF7vs4), fill in the necessary credentials, upload the sketch and launch the Blynk app to start playing.

Happy Ponging!  
