# CSGORadar.
For a school project I had to do something with an arduino.
I decided to make a radar for csgo with a led matrix. 


## Supplies.
1) Arduino Uno.
2) 2 Breadboards.
3) 8x8 led matrix.


## Connections.
The connections between the arduino and the led matrix looks something like this.

![matrix_connections_s9OJ43nrTc](https://user-images.githubusercontent.com/72727982/125671026-eb9f9444-7319-46c8-aeb2-b57ba0ce3d87.jpg)

## Stuff you'll have to change.
You will have to update the addresses and offsets in the main.cpp file for it to work.
The addresses can be found here: https://github.com/frk1/hazedumper/blob/master/csgo.cs.
You will probably also have to change the COM port for it to work which is also in the main.cpp file.
I did tape off one row and one column in order to have a center point. 


When it's working it should look something like this.

https://user-images.githubusercontent.com/72727982/125674555-b5efd369-8aa0-4cc1-867f-0f8729b7ae03.mp4

