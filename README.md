# F1 BLARE 🏎️⏰

## Inspiration

I wanted to make a BLARE alarm clock, but instead of making a normal alarm clock, I wanted to make something that actually looked cool on my desk.

I decided to make mine in the shape of an F1 car. The idea is to have the F1 car sitting on top of the clock, with the display built into the base and four buttons on the front.

The four buttons are used to control the clock and alarms:
- `+` 
- `Set Alarm / Stop Alarm`
- `-`
- `Change Screen`

There are four screens: the normal clock and three different alarms.

### Challenges

This was my first time making a PCB and working with KiCad, so there were quite a few things I had to figure out.

I had to learn how to make the schematic, assign footprints, route the PCB, check it with DRC and add 3D models.

The hardest part was probably getting all the PCB connections and footprints working correctly. I also wanted the final PCB to fit inside the custom F1-shaped enclosure.

### Specifications

BOM:

- 1x Seeed XIAO ESP32-C3
- 4x Cherry MX-style switches
- 1x 2.25" TFT display
- 1x 3.3V piezo buzzer
- 1x 8-pin connector for the display
- 8x female-female jumper wires
- 4x M3x8 screws
- 4x M3x16 screws
- 8x M3 heatset inserts

Others:

- Arduino firmware
- Custom PCB
- Custom F1-style enclosure
- 3D printed F1 car body

### How it works

The main screen shows the current time.

Pressing the fourth button changes between the clock and the three alarm screens.

On an alarm screen:
- `+` increases the alarm time
- `-` decreases the alarm time
- `Set Alarm` enables/disables the alarm

When an alarm goes off, the buzzer plays until the user presses the alarm button.

### PCB

| Schematic | PCB | 3D Model |
| --- | --- | --- |
| ![Schematic](images/schematic.png) | ![PCB](images/pcb.png) | ![3D PCB](images/pcb-3d.png) |

### Case

The PCB will be mounted inside the base of the clock, with the TFT display on the front and the four buttons underneath it.

The F1 car body sits on top of the base to make the whole thing look like an F1 car rather than a normal alarm clock.

![F1 BLARE](images/f1-blare.png)

### Firmware

The firmware is written using Arduino IDE and runs on the XIAO ESP32-C3.

The TFT display is controlled using the Adafruit GFX and ST7789 libraries.