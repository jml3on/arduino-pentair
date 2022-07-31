# Protocol Analysis

## Sending

```
        20:49:39.032 INFO Msg# 153   Wireless asking Main to change pool heat mode to Solar Only (@ 95 degrees) & spa heat mode to Heater (at 102 degrees): 
        165 16  16  34  136 4   95  102 7   0   2   63
        m   tgt src cmd 
        a5  10  10  22  88  04  5f  66  07  00  02  3f
```

## Observed commands and frequency

This summarizes the frequencies & values seen over a 129 minutes period.

### Command IDs

```
   4129 command: 01
  18384 command: 02
   4864 command: 04
    278 command: 05
   4736 command: 06
   4815 command: 07
    275 command: 08
      2 command: 18
      1 command: 1d
      1 command: 1e
   2184 command: 50
  16739 command: 92
      2 command: fa
     72 command: ff
```

### Target IDs

```
   3976 destination: 0f broadcast
   2074 destination: 10 control center
   2103 destination: 60 pump1
   3658 destination: 90 intellichlor
```

### Source IDs

```
   9737 source:      10
   2074 source:      60
```

## 0x01 - ???

control center to pump request/response

### Request

. 01: always 0x02
. 02: 0xbf when pool is OFF; switches to 0xc4 when pool turns on. periodically goes back to 0xbf 4 times

```
$ grep 601001: dump.txt | cut -f 3 | uniq -c
      8 bf
    111 c4
      4 bf
    125 c4
      4 bf
    127 c4
      4 bf
    125 c4
      4 bf
    127 c4
      4 bf
    127 c4
      4 bf
    124 c4
      4 bf
    125 c4
      4 bf
    125 c4
      4 bf
    126 c4
      4 bf
    126 c4
      4 bf
    122 c4
      4 bf
    125 c4
      4 bf
    124 c4
      4 bf
    125 c4
      4 bf
    123 c4
      4 bf
    126 c4
      4 bf
    126 c4
      4 bf
    125 c4
      4 bf
    113 c4
     16 bf
```

```
- pump on:    02 c4 07 d0 ->
              07 d0
- solar on:   02 c4 09 c4 ->
              09 c4
- cleaner on: 02 c4 08 98 ->
              08 98
- pump off:   02 bf 00 53 ->
              no response
```

the response's bytes echo the request's 3rd & 4th.
only get a response when pool is ON.

when pool is OFF, request's 4th byte is water temp.

```
version: 00
target:  60
source:  10
command: 01
type:    Unknown
- Packet Data
length:      4
bufferPos:   15
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
0x601001:    02 c4 09 c4 
data(10):    2 196 9 196
checkdat:    02 ad
checksum:    02 ad
buffer:      ff 00 ff a5 00 60 10 01 04 02 c4 09 c4 02 ad
---------------------------------
version: 00
target:  10
source:  60
command: 01
type:    Unknown
- Packet Data
length:      2
bufferPos:   14
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
0x106001:    09 c4 
data(10):    9 196
checkdat:    01 e5
checksum:    01 e5
buffer:      ff ff 00 ff a5 00 10 60 01 02 09 c4 01 e5

```

## 0x02 - System Status Broadcast

- header: 25 0f 10 02
- length: 1d
- 00: hour (0-23)
- 01: minute (0-59)
- 02: circuits
- 03:
- 04:
- 05:
- 06:
- 07:
- 08:
- 09: mode(auto, service, timeout) and temperature unit (Celcius, fahrenheit)
- 10: 0x03; 0x33 when solar is on
- 11:
- 12:
- 13:
- 14: water temp
- 15: spa temp
- 16:
- 17:
- 18: air temp
- 19: solar temp
- 20:
- 21:
- 22: heat settings
- 23:
- 24:
- 25:
- 26:
- 27:
- 28:

```
target:  0f
source:  10
command: 02
type:    Status Broadcast
time:    7:36
mode:    Auto
air T:   60F
water T: 68F
solar T: 54F
circuits:VACCUUM=OFF ??=OFF ??=OFF ??=OFF POOL=OFF ??=OFF ??=OFF ??=OFF 
pool heat: Solar
spa heat : Heater
- Packet Data
length:      29
bufferPos:   48
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data(0x):    07 24 00 00 00 00 00 00 00 00 03 00 20 04 44 44 20 00 3c 36 00 00 07 00 00 57 80 00 0d 
data(10):    7 36 0 0 0 0 0 0 0 0 3 0 32 4 68 68 32 0 60 54 0 0 7 0 0 87 128 0 13
checksum:    03 5f
buffer:      ff ff ff ff ff ff ff ff ff 00 ff a5 25 0f 10 02 1d 07 24 00 00 00 00 00 00 00 00 03 00 20 04 44 44 20 00 3c 36 00 00 07 00 00 57 80 00 0d 03 5f
```

## 0x04 - Pump ???

single byte.
always `0xff` for both request/response.
request pump to turn

### Request

#### System Off

```
version:     00
destination: 60
source:      10
command:     04
- Unkown
- Packet Contents
length:      1
bufferPos:   22
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data:        ff 
data:        255
checksum:    02 19
buffer:      ff 10 02 50 11 00 73 10 03 ff ff 00 ff a5 00 60 10 04 01 ff 02 19
```

### Response

#### System Off

```
version:     00
destination: 10
source:      60
command:     04
- Unkown
- Packet Contents
length:      1
bufferPos:   13
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data:        ff 
data:        255
checksum:    02 19
buffer:      ff ff 00 ff a5 00 10 60 04 01 ff 02 19
```

## 0x05 - Time Settings Broadcast

Sent about every other minute, or when the config is modified.

- 0: hour
- 1: minute
- 2: day of the week sunday=1, monday=2, tuesday=4, ...
- 3: day
- 4: month
- 5: year
- 6: clk adjust
- 7: auto adjust DST

```
version: 25
target:  0f
source:  10
command: 05
type:    Unknown
- Packet Data
length:      8
bufferPos:   27
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
0x0f1005:    0b 0a 02 14 08 12 00 00 
data(10):    11 10 2 20 8 18 0 0
checkdat:    01 3b
checksum:    01 3b
buffer:      ff ff ff ff ff ff ff ff ff 00 ff a5 25 0f 10 05 08 0b 0a 02 14 08 12 00 00 01 3b
```

## 0x06 - Pump Control

Pump Request/Response

single byte.

- 00: 0x04 when system off, 0x10 when system/pump running.
- byte is echoed in reply.

### Request

#### System Off

```
version:     00
destination: 60
source:      10
command:     06
- Unkown
- Packet Contents
length:      1
bufferPos:   12
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data:        04 
data:        4
checksum:    01 20
buffer:      ff 00 ff a5 00 60 10 06 01 04 01 20
```

### Response

#### System Off

```
version:     00
destination: 10
source:      60
command:     06
- Unkown
- Packet Contents
length:      1
bufferPos:   13
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data:        04 
data:        4
checksum:    01 20
buffer:      ff ff 00 ff a5 00 10 60 06 01 04 01 20
```

## 0x07 - Pump Status

### Pump Status Request

```
version: 00
target:  60
source:  10
command: 07
type:    Pump Status Request
- Packet Data
length:      0
bufferPos:   12
checkdat:    01 1c
checksum:    01 1c
buffer:      ff ff 00 ff a5 00 60 10 07 00 01 1c
```

### Pump Status Response

- header: 00 10 60 07
- length: 0f
- 00: on(0x0a, off = 0x04)
- 01:
- 02:
- 03: watts hi
- 04: watts lo
- 05: rpms hi
- 06: rpms lo
- 07: 0x00
- 08: 0x00
- 09: 0x00
- 10: 0x00
- 11: 0x00
- 12: priming=0x0b, running=0x01, off=0x00
- 13: hour
- 14: minutes

#### Pump Priming ?

```
version:     00
destination: 10
source:      60
command:     07
- Pump Status Response
     time:    10:15
  running:    No
    state:    Off
    watts:    2112
     rpms:    3450
- Packet Contents
length:      15
bufferPos:   27
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data:        0a 00 00 08 40 0d 7a 00 00 00 00 00 0b 0a 0f 
data:        10 0 0 8 64 13 122 0 0 0 0 0 11 10 15
checksum:    02 28
buffer:      ff ff 00 ff a5 00 10 60 07 0f 0a 00 00 08 40 0d 7a 00 00 00 00 00 0b 0a 0f 02 28
```

#### Pump running

```
version:     00
destination: 10
source:      60
command:     07
- Pump Status Response
    time:     19:06
    watts:    643
    rpms:     2200
length:      15
bufferPos:   27
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data:        0a 00 00 02 83 08 98 00 00 00 00 00 01 13 06 
data:        10 0 0 2 131 8 152 0 0 0 0 0 1 19 6
checksum:    02 74
buffer:      ff ff 00 ff a5 00 10 60 07 0f 0a 00 00 02 83 08 98 00 00 00 00 00 01 13 06 02 74
```

#### Pump Stopped

```
version: 00
target:  10
source:  60
command: 07
type:    Pump Status Response
time: 8:11
running: No
watts: 0
rpms: 0
- Packet Data
length:      15
bufferPos:   27
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data(0x):    04 00 00 00 00 00 00 00 00 00 00 00 00 08 0b 
data(10):    4 0 0 0 0 0 0 0 0 0 0 0 0 8 11
checkdat:    01 42
checksum:    01 42
buffer:      ff ff 00 ff a5 00 10 60 07 0f 04 00 00 00 00 00 00 00 00 00 00 00 00 08 0b 01 42
```

## 0x08 - Heat Status

Broadcast every other minute.

- 0: pool temp
- 1: spa temp
- 2: air temp
- 3: target pool temp
- 4: target spa temp
- 5: heat source
- 6:
- 7:
- 8: solar temp
- 9:
- 10:
- 11:
- 12:

```
version: 25
target:  0f
source:  10
command: 08
type:    Heat Status
pool T:   70
spa T:    70
air T:    66
solar T:  81
pool heat:Off
spa heat: Heater
- Packet Data [13]
			0	1	2	3	4	5	6	7	8	9	10	11	12
0x0f1008:	46	46	42	57	5f	04	00	00	51	64	00	00	00	
data(10):	70	70	66	87	95	4	0	0	81	100	0	0	0
checkdat:	03	3b
checksum:	03	3b
bufferPos:	32
buffer:		ff	ff	ff	ff	ff	ff	ff	ff	ff	00	ff	a5	25	0f	10	08	0d	46	46	42	57	5f	04	00	00	51	64	00	00	00	03	3b
```

## 0x11 - Schedule Configuration

All commands starting with 0x1 seem to be broadcasting config changes from the control panel.

These are sent when navigating the schedule config menu on the control center

first byte is the device/circuit it seems: 01 is pool, 02 is cleaner, 03 could be spa?

last byte (127) are the days the schedule is active.

```
version: 25
target:  0f
source:  10
command: 11
type:    Unknown
- Packet Data
length:      7
bufferPos:   26
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
0x0f1011:    01 06 0a 0f 14 0f 7f 
data(10):    1 6 10 15 20 15 127
checkdat:    01 c3
checksum:    01 c3
buffer:      ff ff ff ff ff ff ff ff ff 00 ff a5 25 0f 10 11 07 01 06 0a 0f 14 0f 7f 01 c3
---------------------------------
version: 25
target:  0f
source:  10
command: 11
type:    Unknown
- Packet Data
length:      7
bufferPos:   26
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
0x0f1011:    02 02 12 1e 14 00 7f 
data(10):    2 2 18 30 20 0 127
checkdat:    01 c8
checksum:    01 c8
buffer:      ff ff ff ff ff ff ff ff ff 00 ff a5 25 0f 10 11 07 02 02 12 1e 14 00 7f 01 c8
---------------------------------
version: 25
target:  0f
source:  10
command: 11
type:    Unknown
- Packet Data
length:      7
bufferPos:   26
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
0x0f1011:    03 00 00 00 00 00 00 
data(10):    3 0 0 0 0 0 0
checkdat:    01 04
checksum:    01 04
buffer:      ff ff ff ff ff ff ff ff ff 00 ff a5 25 0f 10 11 07 03 00 00 00 00 00 00 01 04
```

## 0x20 - ??

## 0x44 - ??

## 0x92 - ??

Seen this when the spa is ON. First time is just before the status shows the pool=off.

```
version:     00
destination: 90
source:      10
command:     92
- Unkown
- Packet Contents
length:      21
bufferPos:   33
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data(0x):    00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
data(10):    0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
checksum:    01 ec
buffer:      ff ff 00 ff a5 00 90 10 92 15 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 ec
```

The status data before:
```
data(0x):    12 00 20 00 00 00 00 00 00 00 03 00 20 04 55 55 20 00 4f 53 00 00 07 00 00 93 be 00 0d 
```
and after:
```
data(0x):    12 00 01 00 00 00 00 00 00 00 0f 00 20 04 55 55 20 00 4f 53 00 00 07 00 00 93 be 00 0d 
```

here the only different byte (other than circuits[2]) is [10]

## 0xd2 - ??

Sent by easytouch controller, always destination 90; always single byte = 0xd2

### Request

#### System Off

```
version:     00
destination: 90
source:      10
command:     d2
- Unkown
- Packet Contents
length:      1
bufferPos:   12
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data:        d2 
data:        210
checksum:    02 ea
buffer:      ff 00 ff a5 00 90 10 d2 01 d2 02 ea
```

## Intellichlor (0x50)

Intellichlor related packets have a different format.

`<header><data><checksum><footer>`, where:

- header is `10 02`
- data seen at 3 or 4 bytes
- checksum is one byte long, computed as a unit8_t and includes the header.
- footer is `10 03`

These packets appears as pairs, which suggest a request/response.

### Chlorinator Request

3 bytes long
- [0] always 0x50. maybe as command.
- [1] ??, 0x00 when chlorination is disabled. 0x11 when not.
- [2] % target of chlorination, between 0% and 100%. 0 when chlorination is disabled.

Changing the spa% doesn't seem to affect the request. Maybe it gets sent when spa mode is on?

```
version:     00
destination: 00
source:      00
command:     ff
- Unkown
- Packet Contents
length:      3
bufferPos:   9
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data(0x):    50 11 5a 
data(10):    80 17 90
checksum:    00 cd
buffer:      ff 10 02 50 11 5a cd 10 03
```

### Chlorinator Response

4 bytes long
- [0] always 0x50. maybe as command.
- [1] ??, 0x00 when chlorination is disabled. 0x11 when not.
- [2] appears to be the salinity ppm: 66 * 50 = 3300ppm
- [3] bit 3 (mask 0x10) is set to 1 when the status light flashes green (inspect cell)

```
version:     00
destination: 00
source:      00
command:     ff
- Unkown
- Packet Contents
length:      4
bufferPos:   10
             0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28
data(0x):    00 12 42 80 
data(10):    0 18 66 128
checksum:    00 e6
buffer:      ff 10 02 00 12 42 80 e6 10 03
-------------------------------------------------------------------------------------------
```

## 0x85

I have an Autelis on my Inteltouch system. If I change the time on it to send to the controller, I get this message sent and its ACK..

Time to be set was: 8/12/2016, 6:02:00 PM (I can’t set seconds, it always zero)
```
NEW FRAMES RECEIVED
Payload bytes to get… 8
Checksum is……. GOOD
ID: 0x0B
Des: 0x10
Src: 0x24
Que: 0x85
Msg: 0x12 0x02 0x20 0x0C 0x08 0x10 0x00 0x01
18 2 32? 12 8 16 00? 01?
```
so the time is: 18:02 on 8/12/16
```
NEW FRAMES RECEIVED
Payload bytes to get… 1
Checksum is……. GOOD
ID: 0x0B
Des: 0x24
Src: 0x10
Que: 0x01
Msg: 0x85
```
This is an ack to the above message, notice what I call Que on the first message is 0x85 and is the Msg on the ack… so the Que field might be a frame counter???…