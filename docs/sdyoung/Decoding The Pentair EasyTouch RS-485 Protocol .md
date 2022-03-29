# Decoding The Pentair EasyTouch RS-485 Protocol
## Posted by Jason on Sep 5, 2015 in Geek Stuff, Rants and Such

> **If you already have an RS-485 line connected to your EasyTouch then you are possibly already seeing the messages on the wire.  If not, this isn’t the best article to begin with.  You should probably start here.**

So we have a new pool.  Which means I have new things to learn (or remember) and because I can’t help myself, I also have new things to geek out on.  Our pool builder used Pentair equipment on nearly every job and from what I read, it’s pretty good stuff.  The pump has a main control panel which besides acting as a power sub-panel also is the brain of the system.  There are three peripherals hanging off it: the pump, salt cell chlorinator and the wireless remote.  These each have a control wire that turns out to be a RS-485 bus.  So including the control panel that makes four “nodes”.  How will I figure out how to get access to this bus?

# What I Know

Pentair sells a protocol adapter called ScreenLogic2 to convert the RS-485 to Ethernet, but at over $400 that seems too “luxury” for me.  Autelis also makes a protocol adapter that is almost half the cost, and they have a write-up for integrating it with my Vera home automation controller using the Variable Container plug-in.  That seems like the better choice, but I couldn’t shake the thought that I can get RS-485 Arduino shields for like $5.  And even if I had to buy another Arduino, I’m out $35 from Amazon or $10 if I wait for the slow boat from China.  So let’s see, $425 – $15 = big bucks saved, and $270 – $15 = bucks saved… I’m doing this myself.  Besides, plug-and-play is for sissies.

# Where Do I Start?

Google, of course.  The first realization I had was that Pentair hasn’t released an API.  So there goes easy street.  I guess it serves to protect the cost of their ScreenLogic2.  So it looks like this is old school packet decoding.  And that’s what lots of others taking my approach have done.  Most used a raspberry Pi, and I have a spare one on hand.  But I have two issues with them: the SD cards seem to die at some point no matter how high-end I buy and I’m not into Python, yet (I think there are better options now that have onboard flash vs SD flash).  Regardless of the platform the same thing needs to happen: decode the protocol.  With a handful of useful Google hits I was on my way.

#The Pentair Protocol

Most people noticed there are different messages but depending on their needs only decoded some of them, primarily the broadcast updates because they contain multiple datapoints.  The messages are all in HEX, so be prepared to convert to decimal.  This was the first thing that helped me see what was happening.  I could see the Pentair remote display a water temperature of 85ºF, convert that to HEX and look for 0x55 in the messages.  The protocol follows a structured format like the next table shows.  (HINT:  the pump is telling the control panel it’s using 230 (E6) watts and running at 1400 (578) RPM)
```
1
2
3
4
5
FF 00 FF A5 - Preamble
00 10 60 07 - Header (in this case message is pump to control panel (60 to 10)
0F - length if data in bytes (15 in this case)
0A 00 00 00 E6 05 78 00 00 00 00 00 01 12 17 - the data payload
02 C2 - the checksum
```

Here’s the broadcast packet as far as I have decoded it.  It contains padding, a preamble, destination, source, length, hour, minute, mode, water temp, air temp and a two byte checksum.  For ease just strip off the padding of 0xFF on the lead in of the packet — it’s just noise.

So for this line you would have the following:  A5+7+F+10+2+1D+12+17+22+20+4+56+56+53+81+83+3+D = 0x036C = 876.  The checksum is calculated by multiplying the high order byte by 256 and adding that to the low order byte.  So ((0x3 * 256) + 0x6C) which is 768 + 108 which is 876.  Isn’t HEX fun!  If you can’t convert HEX to decimal in your head go here.  OK, so know you can calculate checksums.  This will work for all the control panel, wireless remote and pump messages.  The salt chorinator messages are different.  More on this later.
```
FF 00 FF A5 07 0F 10 02 1D 12 17 22 00 00 00 00 00 00 20 00 00 00 04 56 56 00 00 53 00 00 00 00 00 00 81 83 03 0D 03 6C    CHK=876
         <-------------------------------------------------add these up-----------------------------------------><chksum>
1
2
3
4
5
6
7
8
9
10
11
12
13
14
                                     S   L
                                     O   E
                                  D  U   N  H     M                                   T  T        T                                C  C
                                  E  R   G  O  M  O                                   M  M        M                                H  H
                                  S  C   T  U  I  D                                   P  P        P                                K  K
<------padding-----> <preamble>   T  E   H  R  N  E                                   1  2        3                                H  L
FF FF FF FF FF FF FF FF 0 FF A5 7 F 10 2 1D 14 1E 00 00 00 00 00 00 00 20 00 00 00 04 5A 5A 00 00 60 00 00 00 00 00 00 81 83 03 0D 03 68 pool off, light off
FF FF FF FF FF FF FF FF 0 FF A5 7 F 10 2 1D 15 04 02 00 00 00 00 00 00 20 00 00 00 04 58 58 00 00 5F 00 00 00 00 00 00 81 83 03 0D 03 4C pool off, light on
FF FF FF FF FF FF FF FF 0 FF A5 7 F 10 2 1D 14 2D 22 00 00 00 00 00 00 20 00 00 00 04 58 58 00 00 5F 00 00 00 00 00 00 81 83 03 0D 03 94 pool on, light on
FF FF FF FF FF FF FF FF 0 FF A5 7 F 10 2 1D 14 2D 20 00 00 00 00 00 00 20 00 00 00 04 58 58 00 00 5F 00 00 00 00 00 00 81 83 03 0D 03 92 pool on, light off
FF FF FF FF FF FF FF FF 0 FF A5 7 F 10 2 1D 14 20 04 00 00 00 00 00 00 20 00 00 00 04 5A 5A 00 00 60 00 00 00 00 00 00 81 83 03 0D 03 6E waterfall on, light off
FF FF FF FF FF FF FF FF 0 FF A5 7 F 10 2 1D 14 20 06 00 00 00 00 00 00 20 00 00 00 04 59 59 00 00 60 00 00 00 00 00 00 81 83 03 0D 03 6E waterfall on, light on
FF FF FF FF FF FF FF FF 0 FF A5 7 F 10 2 1D 15 05 01 00 00 00 00 00 00 20 00 00 00 04 58 58 00 00 5F 00 00 00 00 00 00 81 83 03 0D 03 4C cleaner on, light off
FF FF FF FF FF FF FF FF 0 FF A5 7 F 10 2 1D 15 06 03 00 00 00 00 00 00 20 00 00 00 04 58 58 00 00 5F 00 00 00 00 00 00 81 83 03 0D 03 4F cleaner on, light on
```

You can see my brief notes above the columns.  This packet broadcasts forever.  The top packet includes the header and preamble (padding + 0xFF, 0x0, 0xFF, 0xA5, 0x?), source (0x10), destination (0xF aka broadcast), the system hour and minute (0x14, 0x1E), mode (0x0), temps (0x5A, 0x5A, 0x60) and the checksum bytes (0x3, 0x68).  Whew.  So we know the control panel sent a broadcast, 29 bytes long, at 8:30PM, the pool is idle, the water is 90ºF, the air is 96ºF.  This most interesting byte in this message for me, besides the temps is the mode byte.  It tells us exactly what is on, or off.

## So What Mode Am I Running In Now?

That’s pretty easy once you know where to look.  I assume your system could be assigned different addresses, but in mine it works out like the previous table.  Scroll to the right and you can see my notes appended to the end of each line.  All I did was run the controller through every combination to see what mode bytes were generated.  There is a pattern here.  You can see I found the following:
00 = system idle
01 = in floor cleaner mode.  This is tied to SPA mode, since I don't have a spa on my pool.
02 = pool light on
03 = cleaner and light on (you caught that we just added 1 + 2 to get 3, right?)
04 = waterfall on
05 = cleaner and waterfall (missing from sample messages above)
06 = waterfall and light on (again, 4 + 2 = 6, get it?)
07 = cleaner, waterfall and light on (missing from sample messages above)
20 = pool pump circulating (low speed)
22 = pool pump and light on

## So Where Are We At So Far?

You know the Pentair packet format.  You know where to determine the source and destination of a message.  You know how to calculate how long a message is.  You can determine which mode the system is running and how warm (or cold) the water is.  You know how warm the air is.  That’s the foundation for the pump messages.  All we need to do is dissect the messages that include the pump as a source and we can get the power in watts consumed real-time and the pump RPM.

## So Let’s Break The Pump Message Down

Same format as the broadcast.  In fact, I already revealed it in the Pentair Protocol section above.  Here it is again because there’s a little more to it.  The pump will periodically send it’s stats to the controller after it receives a query packet formatted like FF 00 FF A5 00 60 10 07 00 01 1C.  This is the EasyTouch requesting stats from the pump.  The reply from the pump is always in the format FF 0 FF A5 0 10 60 7 F 0 0 0 E6 5 78 0 0 0 0 0 1 C 38 2 DD.  Interestingly, that query packet has a zero byte payload, so the magic must be in byte 0x7.  The only thing I can’t be certain of in the response packet is byte 8.  Mine is always a 0x7 but others show differently.  Doesn’t matter because I don’t know the bytes significance anyway.  So here we have a bit of a loaded breakdown because the watts and RPM are represented in two bytes each.  Here it is dissected.  In short, the pump informed the controller it is using 230 watts at 1400 RPM and it’s 12:56PM.  Not sure what the 0x0’s and the 0x1 represent, never seen them change.  Again, the checksum is the sum of 0xA5 through 0x38.
```
1                            W W
2                            A A  R R              H    C  C
3              D  S    L     T T  P P              O M  H  H
4              S  R    E     T T  M M              U I  K  K
5 <PREAMBLE>   T  C    N     H L  H L              R N  H  L
6 FF 0 FF A5 0 10 60 7 F 0 0 0 E6 5 78 0 0 0 0 0 1 C 38 2 DD
```

Looking at the watts first we have a high order byte of 5 and low order byte of 78.  Drawing from the previous HEX talk, you need to calculate this as ((0x5 * 256) + 0x78).  This is 1280 + 120.  So the RPM in decimal is 1400.  This is the lowest speed I have set on my controller so I always need to do the calculation this way.  But with watts it’s different.  When I run in low speed (1400RPM) I only use ~230 watts.  Since one byte can be up to 255, the watts fit in a one byte representation.  However, when I run in waterfall or in-floor cleaner mode the watts exceed 255 and roll into the high order byte field.  In that case I need to calculate watts using the RPM formula.  This isn’t a big deal, really, you just need to test for a value in the watts high order byte before you know how to calculate the value.  Oh, I suppose you could just run it through the same long formula since if the watts high order byte is a zero when the watts value is <255, you would just have ((0x0 * 256) + <low order byte>.  I don’t know, your choice I guess.

Just for clarity, here it is on cleaner mode.  This is 1412 watts at 2800 RPM and it’s 7:02PM.  Again, watts are arrived at with ((0x5 * 256) + 0x84, which is ((5 * 256) + 132).  RPM is ((0xA * 256) + 0xF0), which is ((10 * 256) + 240.

```
1                              W W
2                              A A  R R              H    C  C
3              D  S    L       T T  P P              O  M H  H
4              S  R    E       T T  M M              U  I K  K
5 <PREAMBLE>   T  C    N       H L  H L              R  N H  L
6 FF 0 FF A5 0 10 60 7 F A 0 0 5 84 A F0 0 0 0 0 0 1 13 2 2 CE
```

## And About That IntelliChlor Salt Chlorinator Cell

I have an IntelliChlor IC-40.  In my opinion, it’s probably private labeled.  The messaging standard states messages should begin with a 0x10 0x02, contain some data and terminate with 0x10 0x03.  The 0x2 is “Start of text”, or STX and 0x3 is “End of text, or ETX.  Well, as you have read Pentair hosed that up by making their own wacky protocol.  However, the IntelliChlor DOES follow the 10 02 / 10 03 standard.  And on the same bus.  That means you have to account for BOTH the Pentair garbage on the bus AND the IntelliChlor.  It’s just something else you have to code for.

The IntelliChlor sends two messages periodically.  The first is eight bytes long, the second is nine bytes.  When the pump is off only the first is sent.  There is a third message that is infrequently broadcast, by the control panel.  The only thing so far I’ve decoded from it is when it’s actively generating chlorine it will output the percent that the EasyTouch panel instructed it to.  For example, right now I have mine set to 50%.  The way it works to achieve 50% is through on-off-on-off toggling at regular intervals.  So whenever the unit toggles from inactive to active or vice-versa it sends the current output level.  In HEX.  So for this message the format is `<header><data1><data2><data3><checksum><terminator>`.  The checksum calculation is different from the Pentair messages, too.  Another reason I question who really developed this unit.  It’s only one byte and is derived from the sum of the three data bytes + 18.  Why not?  I always likes the number 18 anyway.  Anyway, so below you have, 0x50+0x11+0x32, which is 147 in decimal, then adding 18 brings you to 165, which is magically 0xA5 in HEX.  Wow!  It works.  And no, that 0xA5 has nothing to do with the 0XA5 you see in every Pentair message.  `<shaking my head some more>`

```
1 10 02 - Header
2 50 11 32 - Data (where 32 is HEX for 50, as in 50%)
3 A5 - Checksum
4 10 03 - Terminator
```

The second message is the same formula except it has a fourth data byte.  Otherwise it’s the same format.  I have yet to figure out what the second message means.  Here it is for your viewing pleasure.  I have noticed the third data byte was 0x4B and lately it’s 0x4C.  I wonder if maybe it’s a usage counter?  Watching this one…

```
1 10 02 - Header
2 00 12 4B 80 - Data
3 EF - Checksum
4 10 03 - Terminator
```

The third is a confusing mess.  Every once in a while I catch this message fly by.  It’s a Pentair broadcast message from the EasyTouch panel so it does not follow the IntelliChlor format.

```
1                  L
2                  E
3                  N
4   A5 07 0F 10 19 16 0D 3C 81 49 80 00 49 6E 74 65 6C 6C 69 63 68 6C 6F 72 2D 2D 34 30 08 34 note: 3750ppm no errors
5   A5 07 0F 10 19 16 01 3C 81 4B 80 00 49 6E 74 65 6C 6C 69 63 68 6C 6F 72 2D 2D 34 30 08 2A note: 3750ppm no errors
6   A5 07 0F 10 19 16 01 3C 80 4C 81 00 49 6E 74 65 6C 6C 69 63 68 6C 6F 72 2D 2D 34 30 08 2B note: 3800ppm comm error
7   A5 07 0F 10 19 16 01 3C 81 4C 81 00 49 6E 74 65 6C 6C 69 63 68 6C 6F 72 2D 2D 34 30 08 2C note: low flow
8   A5 07 0F 10 19 16 01 3C 81 4C 80 00 49 6E 74 65 6C 6C 69 63 68 6C 6F 72 2D 2D 34 30 08 2B note: 3800ppm no errors
9   A5 07 0F 10 19 16 01 3C 81 4C 80 00 49 6E 74 65 6C 6C 69 63 68 6C 6F 72 2D 2D 34 30 08 2B note: 3800ppm no errors
10  A5 07 0F 10 19 16 01 3C 81 4C 80 00 49 6E 74 65 6C 6C 69 63 68 6C 6F 72 2D 2D 34 30 08 2B note: 3800ppm no errors
11  A5 07 0F 10 19 16 01 3C 81 4C 80 00 49 6E 74 65 6C 6C 69 63 68 6C 6F 72 2D 2D 34 30 08 2B note: 3800ppm no errors
12                       60             I  n  t  e  l  l  i  c  h  l  o  r  -  -  4  0
```

There is one thing I noticed in bytes three and five (in line one above these are 0x81 and 0x80.  Remember the data bytes begin after the length byte.)  At the end of each line I put the note about what the display showed.  For the lines with no errors, byte three was 0x81, five was 0x80.  But when it displayed “COMM ERROR” these were 0x80 and 0x81.  And then when the display showed “LOW FLOW” these were 0x81 and 0x81.  So there is something going on here but I don’t know how much value these offer, or if there may be other values that I’ve not seen yet.

Regardless, for some reason the EasyTouch sends this message out.  Maybe it’s a “hello” message because it sometimes follows the first IC-40 message.  The only useful item I’ve decoded from this is the second data byte 0x3C (out of the 22) , which corresponds to the IC-40 setpoint of 50%.  This is not the current IntelliChlor output, rather what the IntelliChlor setpoint is.  How is this different?  Remember I said that the IIntelliChlor toggles on, off, on, off?  The first message in this section explains how the IntelliChlor represents it’s activity.  This byte tells you what the IntelliChlor is working towards as it’s setting.  Interestingly, from the the remote if you enter the the menu path DIAGNOSTICS >> CHLORINATOR the display will present the detected salinity level in parts per million, say 3500ppm.  At the same time the remote sends this command on the RS-485 bus:  FF 00 FF A5 07 10 20 D9 01 00 01 B6.  Notice the source is 0x20 and the destination is 0x10?  That means the message came from the remote to the EasyTouch panel.  I don’t know what the 0xD9 means, but the data length is one byte long, and it’s 0x0, so you know the 0xD9 is where the goods are.  At this point the display is showing the salinity in PPM.  I was hoping the response to this remote query would include the salinity, but I just don’t see it.  Instead it’s `A5 07 0F 10 19 16 01 32 81 49 80 00 49 6E 74 65 6C 6C 69 63 68 6C 6F 72 2D 2D 34 30 08 1E`.  Another “Intellichlor–40” message with the setpoint just like the ones above.

## Things To Consider

You should know that because there isn’t a standard 0x10 0x3 terminating sequence for the Pentair messages that how you code to read this will require you to read the length byte and count that many additional bytes to get the payload, and then two more for the checksum.  This kind of sucks because it means each frame can be a different length.  Some come with FF FF FF FF noise before the real FF 0 FF A5 string starts, so you have handle that, too.  Then there’s the Intellichlor 10 02 / 10 03 messages all mixed in there.  What’s worse is there are 10 02 sequences in the Pentair messages.  WTF Pentair?  Look at line one below.  That 10 02 IS NOT the beginning of the Intellichlor message.  Nope, the first one of those is on line two.  The second one begins at the end of line two and rolls into line three.  There’s another pair on lines 25 and 26, and the second in the last set is the goofy “Intellichlor — 40” message.  Plus, it’s half-duplex RS-485 with no collision detection.  That’s where the checksum comes in, so you better calculate that if you expect to trust what you see on the wire.  When I first coded my Arduino to get the messages here’s the hot mess I got back:

```
1   FF FF FF FF FF FF FF FF 00 FF A5 07 0F 10 02 1D 09 08 20 00 00 00 00 00 00 20 00 00
2   00 04 56 56 00 00 5B 00 00 00 00 00 00 81 83 03 0D 03 5A 10 02 50 11 34 A7 10 03 10
3   02 00 12 4B 80 EF 10 03 FF FF FF FF FF FF FF FF 00 FF A5 07 0F 10 02 1D 09 08 20 00
4   00 00 00 00 00 20 00 00 00 04 55 55 00 00 5B 00 00 00 00 00 00 81 83 03 0D 03 58 FF
5   FF FF FF FF FF FF FF 00 FF A5 07 0F 10 02 1D 09 08 20 00 00 00 00 00 00 20 00 00 00
6   04 55 55 00 00 5B 00 00 00 00 00 00 81 83 03 0D 03 58 FF FF FF FF FF FF FF FF 00 FF
7   A5 07 0F 10 02 1D 09 08 20 00 00 00 00 00 00 20 00 00 00 04 55 55 00 00 5B 00 00 00
8   00 00 00 81 83 03 0D 03 58 FF 00 FF A5 00 60 10 07 00 01 1C FF 00 FF A5 00 10 60 07
9   0F 0A 00 00 00 E3 05 78 00 00 00 00 00 01 09 08 02 A7 FF FF FF FF FF FF FF FF 00 FF
10  A5 07 0F 10 08 0D 55 55 5B 2A 2B 00 00 00 00 64 00 00 00 02 9E FF FF FF FF FF FF FF
11  FF 00 FF A5 07 0F 10 02 1D 09 09 20 00 00 00 00 00 00 20 00 00 00 04 55 55 00 00 5B
12  00 00 00 00 00 00 81 83 03 0D 03 59 FF FF FF FF FF FF FF FF 00 FF A5 07 0F 10 02 1D
13  09 09 20 00 00 00 00 00 00 20 00 00 00 04 55 55 00 00 5B 00 00 00 00 00 00 81 83 03
14  0D 03 59 FF FF FF FF FF FF FF FF 00 FF A5 07 0F 10 02 1D 09 09 20 00 00 00 00 00 00
15  20 00 00 00 04 55 55 00 00 5B 00 00 00 00 00 00 81 83 03 0D 03 59 FF 00 FF A5 00 60
16  10 04 01 FF 02 19 FF 00 FF A5 00 10 60 04 01 FF 02 19 FF 00 FF A5 00 60 10 06 01 0A
17  01 26 FF 00 FF A5 00 10 60 06 01 0A 01 26 FF 00 FF A5 00 60 10 01 04 02 C4 05 78 02
18  5D FF 00 FF A5 00 10 60 01 02 05 78 01 95 FF FF FF FF FF FF FF FF 00 FF A5 07 0F 10
19  02 1D 09 09 20 00 00 00 00 00 00 20 00 00 00 04 55 55 00 00 5B 00 00 00 00 00 00 81
20  83 03 0D 03 59 FF FF FF FF FF FF FF FF 00 FF A5 07 0F 10 02 1D 09 09 20 00 00 00 00
21  00 00 20 00 00 00 04 55 55 00 00 5B 00 00 00 00 00 00 81 83 03 0D 03 59 FF FF FF FF
22  FF FF FF FF 00 FF A5 07 10 20 C5 01 00 01 A2 00 FF FF FF FF FF FF FF FF 00 FF A5 07
23  0F 10 05 08 09 09 10 1B 08 0F 00 00 01 2C FF FF FF FF FF FF FF FF 00 FF A5 07 0F 10
24  02 1D 09 09 20 00 00 00 00 00 00 20 00 00 00 04 55 55 00 00 5B 00 00 00 00 00 00 81
25  83 03 0D 03 59 10 02 50 14 00 76 10 03 10 02 00 03 00 49 6E 74 65 6C 6C 69 63 68 6C
26  6F 72 2D 2D 34 30 BC 10 03
```

Yep.  One big ass long stream of nonsense.  Oh all the data is in there, trust me.  You just have to code for it.

## So What Do You Do Next?

If you’re planning on using an Arduino to do this then you might find my write-up on that handy.  It ain’t the prettiest coding, but it’s working, and updating Xively, and updating Vera, soon to be now updating this website, and it accepts controls via HTTP GET calls, and I’m out a whopping $5 for the RS-485 shield.

# 31 Comments

## Mike | September 14, 2015

Great post! I’ve been trying to piece together the Pentair RS485 protocol and really wish I’d found this before having done it already. It would have saved me several hours 🙂

##     Jason | September 14, 2015

Thanks, Mike. If you found any additional gems buried in the messages please, please, please… share with me. I’ve posted my Arduino code and almost as quickly began fine-tuning it. It was rather messy. When I have a stable load I will update it accordingly.

I’d really like to extract the salinity PPM from the IntelliChor programmatically but I’ve not been able to coax that info onto the wire without querying it manually from the panel or remote. Strangely, even injecting the command that the panel sends to return the salinity fails when I force it from the Arduino. Other things do work this way so it’s baffling me still.

Thanks for the comment. Jason

## Mike | September 14, 2015

Hi Jason,

After implementing my Pentair pool controller in Python, I have re-written a simple controller in node.js which I plan to move to a Rasberry Pi when I get it to the state I like. I’ve also combined it with control for my Yamaha RX675 which has a REST interface. Now, I can create a tasker task on my android to POST commands to node.js to turn on pool features and music. It’s been a lot of fun and basically works but there’s more work to do to polish things.

I’ve really only decoded the Pentair info as far as required to meet my needs and relied heavily on the sources I could find online (due to laziness). This makes it difficult to make available to others, so maybe I need to revisit before posting publicly.

Oh, once nice feature that I added tonight. If there’s a problem due to loss of prime or pump breaker, the pump stops (only happens once in a great while). I check for the state of pool==on and pump RPM==0. This indicates a problem, so I send a notification to my phone via Pushbullet. I haven’t tested it yet but will get around to it in a few days.

All in good fun and a great way to bring a bit of home automation to my pool. Maybe a OpenHab binding for Pentair is in my future?? That’d be a great project!!

Anyway – thanks again for posting this info online. It’s a great service for the growing community of people like me 🙂

## Mike | September 15, 2015

Oh yes, one more thing. Have you tried to decode the light sequences for the LEDs? I haven’t dug into that yet, so maybe it’s something I can share soon.

Jason | September 15, 2015

I have not. I have the IntelliBrite LED and while it looks nice, I just leave it on white since it’s the only color that looks decent in my pool. I figured that there wasn’t a code sequence based on how you cycle through the colors – toggling on/off/on. Even when we select a party color I can hear the relay in the panel clicking it’s way through the sequence. So maybe the light just has a set count of patterns and the on/off is the trigger? IDK. I never looked for that sequence.

## Mike | September 16, 2015

Hi Jason,

I’ve put my first cut of the node.js based pool controller on github if you’re interested. I’m not particularly proud of it since it’s my first adventure in node.js, but the speed, small footprint, and asynchronous handling of requests was desirable to me. (plus it will run on a Pi)

https://github.com/michaelusner/Home-Device-Controller

## Jason | September 16, 2015
Nice. I should prob upload to Git myself… maybe when more time permits… and I’m more comfortable with the code I’ve hacked together. It keeps periodically locking up. I should mention, I was looking at your Git’s and saw the packet spec doc – I think I leveraged that without knowing who was behind it, so a big thanks is in order. I think it was in a ZIP file from Cocoon IIRC.

Do yo know how to decipher the CFI byte? I think there is some magic that could be unleashed if I understood it further. That, and I still want the damn elusive IntelliChlor salinity output.

I have some big deadlines at work that will deter my efforts for a bit but I will get back to hacking away at this in a few weeks. Thanks for sharing the Git link.

## Erhard | January 21, 2016

Hi Jason,

thank you for this excellent write up! I’m trying to get data off my Pentair IntelliFlo VS pump (model #011018‎), but I can’t get the pump to talk at all over the COM port. I verified the following:
* pump is in “Run Schedules” mode
* supplied wire from Pentair is working using a multimeter
* RS485 Arduino shield is working using an oscilloscope
* Voltage on COM cable is always high at 5V. No chatter on the oscilloscope either

I was wondering if there is anything else I need to do to get the pump to send data over the COM port?

Thank you,
Erhard.

## Jason | January 21, 2016
Erhard, if you have the pump as a stand-alone device, meaning no Pentair control panel then it’s my understanding the pump keeps it’s mouth sealed. Once you send it a properly formatted datagram will it begin communicating, and receiving additional commands. I don’t know what that command is. I seem to recall people posting in forums something along this line. One helpful thread I found is here, and I highly encourage you to follow the link on that threads second page. It leads you to a zip file that you will need to register before you can download. It’s worth the registration time, because once you read the ‘readme.txt’ file inside it will open your eyes a little more to the Pentair protocol (hint: that guy had a stand-alone pump, too).

My pump wires to the panel, and the panel has a salt cell, wireless remote and my 485 shield all on the same bus. It is very chatty. The controller initiates the comms with my pump so my code doesn’t need to do that routine. I’m basically joining the conversation already in progress, listening to the events and sending scripted datagrams with instructions. My pump’s control pad doesn’t really have any useful purpose because when connected to a Pentair controller it basically disables it. Even the display is blank until I put the panel in lockout.

Hope this helped.
JY-

## Erhard | January 22, 2016
Thank you, Jason! That helped a lot. I’m going to experiment with that code this weekend.

## Tagyoureit | May 21, 2016

Jason,
awesome work. I’ve used your work and Mike’s work and am working on a Homebridge (Siri support for the impatient) plug-in module for Pentair. So far, I’ve been able to read the buffer in NodeJS and am working on decoding it. I’m finding my pool setup is different from both of yours, which is turn going to be different from the next persons. So, I’ve tried to track down the logical circuits to the physical ones. This will enable me to write code that can be configured by the next person to use it.

Unfortunately, the [Screenlogic] circuit & circuit names can be assigned to any physical circuit. This makes looking at the Screenlogic end user UI pretty much useless, except for the Config screen. When you go into the Setup Circuits (Page 2/8 in the Screen Logic app; or somewhere deep in the menus on the actual controller) the physical circuits are listed in order with their logical assignments. They aren’t numbered but they are in order.

Your physical circuits 1-8 are what you call the mode bit. The next two bits in my system are also flags for expansion ports. (The one exception is the Mode+1 bit value 0000010 (binary = 2). It seems to be skipped.

The ‘2’ bit between the Source and Length bit indicates Pool heat. 2 means that the controller is either in Heater/Solar Only/Solar Pref mode. The absence of this bit indicates the heat is set to off for my pool. And, of course in Pentair fashion, the packet length for the pool heater command off is 24 bits and when the pool heater command is one of the on commands it is 29 bits in length.

More work to do… thanks for the great work!

## Mike | July 18, 2016
Great information. I am working on the same kind of project for a co-worker. His wired remote went out, so I am not sure how to figure out the modes for each feature, and if most pools are plumbed the same. Does the system send out the string on the 485 bus if the buttons at the main control panel are pressed? Another part I am having issues with is that the co-worker is not very pool or tech savvy so that he is not sure what each button does, i.e. when the “F” button on the panel is pressed does it just turn on the pump, or does it also move the valves? Thanks for any info.

## Nick DeMarco | July 26, 2016
The IntelliChlor detects salinity via conductivity, perhaps with a temperature correction. The returned value from IntelliChlor must either be conductivity, or the pool’s volume must be sent to IntelliChlor first.

## Jason | July 26, 2016
Thanks for the feedback. With help from another individual we were able to find the value in two separate frames on the bus. I do not believe it is temperature compensating because this time of year my pool swings as high as 8 degrees in a 24 hour cycle. Since the IC-40 seems to only sample salinity upon startup, when the pool starts in the middle of the day the salinity reading is always higher than when it starts early in the morning. Here’s a link to what I mean. Scroll down to Salinity, then change it to 7 days.

## Jeff | September 6, 2016
Hi Jason – were you able to decode the 10/02 IC-40 status messages?

## Jason | September 7, 2016
Hello Jeff, not completely. So far out of the shorter 10/02 frame I can get the current salinity setpoint of the IC40 (ex. 20%, 50%) and the longer 10/02 frame that contains one additional byte has the current salinity reading.
Here’s where they are. The top one is in the PCT byte in hex, so convert the 32 to decimal and you get 50%. The bottom one is the SAL byte, and that one is in decimal. Multiply it by 50 and you get the salinity reading (80 * 50 = 4000ppm). Thanks to Nick Stone for finding this nugget here in the comments.

Pentair IC-40 Decode

## Jeff | September 7, 2016
I found the Salinity is output as the 3rd data byte in the 4-byte long data packet. Since they measure to only 50ppm accuracy, you multiple this value by 50 to get the actual ppm reported in the diagnostics menu.

Jason | September 7, 2016
I agree. We’re on the same page.


## Tom Lafleur | August 1, 2016
Ever thought of moving this great project to Github to give it more visibility….??

Also, have you publish any of the early decoding test programs?

Thanks

## Tom Lafleur | August 12, 2016
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

## Jason | August 12, 2016
Tom, I assume you’re extracting the QUE byte from the byte immediately following the SRC byte. If so, that’s referred to as the CFI byte which according to this doc carries various instructions depending on the DST for that frame. Drill into lines 153 thru 172.

I agree that it looks like the ACK for the write command previous to it, like OK, you sent a command 85, got it. But the doc ostensibly infers that there could be another use case for that byte to use 85 and when sent to a different node on the bus could carry a different meaning. Which byte are you pulling the ID byte from? That appears in both frames.

So it would make more sense that it’s ACKing frame ID 0B with command 85 because the ACK contains those same bytes. But there could apparently be another 85 being ACK’d with a different frame ID. I think the 85 without the ID isn’t specific enough to ACK a unique frame but I would like to know which byte position it’s pulled from.

## Tom Lafleur | August 13, 2016
Yes, its the byte after the SRC byte…

## Jason | August 23, 2016
Sorry, been traveling for work. So that’s the byte I figured it was. Based on this I’d say my third paragraph is most likely the way the messaging works – binding the frame ID to a message payload (CFI) so the ACK is set against a unique message. I don’t know how there could be any other explanation but this protocol is peculiar to say the least.

Did you look at the reference doc I linked to? There is a scratch-on-the-surface CFI byte explanation there. That guy did some good work but he wrote that decoding this byte wasn’t his objective. Too bad, possibly holds some valuable data.

My Arduino has been running error-free for 35.5 days so I plan to hold off until millis() overflows just to confirm it is a benign event. Then back to tweaking some more.

## Tom Lafleur | August 13, 2016
Some more info on the Protocol…

SunTouch, QuickTouch and IntelliFlo RS-485 Bus Protocol
(SunTouch Firmware Version 2.070)

Message format

Messages consist of seven fields, as shown below. The length of a message can vary from 10 to 265 bytes, depending on the length of the DATA field.

HEADER 4 bytes: 0x00 0xFF 0xA5
VERSION 1 byte protocol version
DESTINATION 1 byte source identifier
SOURCE 1 byte destination identifier
COMMAND 1 byte command code
DATA LENGTH 1 byte unsigned int data length (in bytes)
DATA Length as specified by previous byte.Interpretation varies
by command.
CHECKSUM 2 bytes: high-order byte followed by low order byte. The
checksum is calculated by summing all the byte in the
message, starting on the second byte of the HEADER (0xA5)
and ending with the last byte of the DATA. Any overflow is
ignored (i.e., the checksum calculated mod 216).

Version

Controllers and remotes send 0x01.
Intelliflo Pumps send 0x00.

Source and Destination Identifiers

DST SRC
=== ===
Periodic status message: 0f 10
Remote layout request: 10 48
Remote layout response: 0f 10
Circuit state change request: 10 48
Circuit state change response: 48 10
Pump status message: 10 60

The SunTouch (controller) is 10. The QuickTouch transceiver is 48. The ID 0f appears to be “anyone who might be interested.” Intelliflo Pump is 60.

Command Code Ox02 – System Status Message

This message is sent every 2 seconds or so by the pool controller to report the current system status.
BYTE VALUE
==== =============================================
[0] 24-hr time in hours (0-23, decimal)
[1] Time in minutes (0-59, decimal)
[2] Circuits that are on:
When SPA is on, 0x01 (2^0) bit is set
When AUX1 (light) is on, 0x02 (2^1) bit is set
When AUX2 (sweep) is on, 0x04 (2^2) bit is set
When AUX3 is on, 0x08 (2^3) bit is set
When POOL is on, 0x20 (2^5) bit is set
When FEATURE1 is on, 0x10 (2^4) bit is set
When FEATURE2 is on, 0x40 (2^6) bit is set
When FEATURE3 is on, 0x80 (2^7) bit is set
If SPA and POOL bits are both set, spa runs (not pool).
[3] Additional circuits that are on:
When FEATURE4 is on, 0x01 (2^0) bit is set
[4:8] All zero
[9] When controller is set to timeout mode, 0x81 is set
[10] 0x0f if heater is on; 0x03 if heater is off
[11] Zero
[12] 0x4 (2^2) bit indicates DELAY on AUX2 (and perhaps other circuits).
Bits 0x30 appears to be on all the time. Don’t know why.
[13] 0x08 (on 1.0 fw); 0x00 or 0x01 on 2.070 FW; HAVEN’T FIGURED OUT!!!
[14] Water Temperature (degrees F, only meaningful when circulating)
[15] ” ” ” ” ” ” ”
[16] 0x01 on 1.0 FW; 0x02 of 2.070 FW. Major version number? *
[17] Zero on 1.0 FW 0x46 (= 70 decimal) on 2.070 FW
[18] Air Temperature (degrees Fahrenheit)
[19] Solar Temperature (degrees Fahrenheit)
[20] Zero
[21] 0x32 (50 decimal) in 2.070 FW *
[22] Heat setting:
Low order 2 bits are pool: 0 off, 1 heater, 2 solar pref, 3 solar
Next 2 bits are spa: 0 off, 4 htr, 8 solar pref, 12 solar
[23] zero in 1.0 FW; 0x10 in 2.070 FW
[24:26] All zero
[27] 0x19 / 0x38 *
[28] 0x0A; 0x0B on 2.070 FW *

* I’ve only seen one value in this field, and I don’t know what it represents.
Ted Cabeen reports 0x38, but we still don’t know what it represents.
Command Code 0x86 – Circuit Status Change Request

This message is sent by the QuickTouch remote control transceiver to effect a circuit state change.

The first byte of the data is the circuit number. This is not byte [2] of status message! Rather, it is one of these codes: 0x01 represents the SPA, 0x02 is AUX1, 0x03 is AUX2, 0x04 is AUX3, and 0x05 is FEATURE1, 0x06 is POOL, 0x07 is FEATURE2, 0x08 is FEATURE3, 0x09 is FEATURE4, 0x85 is HEAT_BOOST. The second byte of the data represents the desired state of the circuit, 0x01 for on, 0x00 for off. So, for example, this command would turn the spa on:

DST SRC CMD DLEN CHECKSUM
0x00 0xFF 0xA5 0x01 0x10 0x48 0x86 0x02 0x01 0x01 0x01 0x88

Command Code Ox01 – Circuit Status Change Response

This message is sent in response to a circuit status change rquest, whether it had any effect or not. It has one byte of data, which is always 0x86.

Command Code OxE1 – Remote Layout Request
This message is sent periodically by the QuickTouch transceiver to request the button assignments for the remote. It has one byte of data, which is always 0x01.

Command Code Ox21 – Remote Layout Response

This message is sent by the controller to indicate which circuits are assigned to which row on the QuickTouch remote. The data consists of four bytes, which are the circuit codes corresponding to rows 1-4 on the remote. The circuit codes are POOL 0x06, SPA 0x01, AUX1 0x02, AUX2 0x03, AUX3 0x04, HEAT_BOOST 0x85.

Command Code Ox05 – (I have no idea)

This message was not present in firmware version 1.0, but is in 2.0.70. It is sent periodically, approximately once every two minutes. Data length is eight bytes. First two are timestamp (same format as system status message). Byte 4 is the number of days the unit has been on (goes up by one at midnight each day). Byte 3 is has a single bit set; it rotates to the left at midnight each day, i.e., it’s a simple function of Byte 4. Last four bytes change infrequently. Last time I worked on this program, they were [9, 4, 0, 0]; now they’re [8, 6, 0, 0].

Command Code 0x07 (Pump Status to Controller)

Pumps reports status to the controller when requested by the controller (generally every 30 seconds).

BYTE VALUE
==== =============================================
[0] Pump Running:
Started 0x0a
Stopped 0x04
[1] Pump Mode:
Filter 0x00
Manual 0x01
Backwash 0x02
Feature 1 0x06
External Program 1 0x09
External Program 2 0x0a
External Program 3 0x0b
External Program 4 0x0c
[2] Pump State:
Running 0x02
Priming 0x01
System Priming 0x04

Here some more info on the pentair protocol that was done a few years back…

http://cocoontech.com/forums/files/file/173-pab014sharezip/

https://github.com/dminear/easy-touch-raspberry-pi

## tag | September 19, 2016
Hi guys, I haven’t checked this blog in a while. Nice to see there are some people actively working on the program. Myself, and a few others, have also been actively working on this at the same CocoonTech site that has been mentioned a few times here already (http://cocoontech.com/forums/topic/13548-intelliflow-pump-rs485-protocol/page-9#entry249808).

I’ve also posted my NodeJS based Pentair project to Github. https://github.com/tagyoureit/nodejs-Pentair
It has a decent (but not complete) wiki (https://github.com/tagyoureit/nodejs-Pentair/wiki
) of the Pentair protocol packets. I don’t know how this relates to SunTouch or QuickTouch, but since much of the equipment (aka pumps/chlorinators) are interchangeable, it shouldn’t be too different.

Since my last post in May, myself, with the help of others, have decoded MANY of the CFI (and other) packets, are able to dynamically read the custom names, circuit names, schedules and pool/pump information. I have not worked too much on the chlorinator (I have one, but turned it off) just because I haven’t needed to.

Would be great to combine everyone’s efforts either here or at CocoonTech (or log bugs to my Github project and help contribute to the Wiki).

## tag | September 20, 2016
Hi, Since there is so much movement on this code, as well as duplication of efforts, I setup a new community on Gitter @ https://gitter.im/pentair_pool/Lobby. I invite everyone to come collaborate in one single place and together we should be able to make even more significant progress.

## charles | September 25, 2016
Hi:
I am not familiar with gitter, so I am posting here. I have followed this very helpful discussion, and been able to communicate with my pool pump. My system does not have a pool controller, and I am trying to make my own. Since I do not have a controller I cannot capture messages sent to control the pump. I basically want to select pump modes 0 (off) to 4. I looked at the code published here, but it is not my language of choice, so I have not been able to indentify the format of pump commands, and wonder if anyone can help.

Here is what I have found so far to work:

Pump is a Pentair Intelliflow 2 VST, there is NO controller present. Connect motor RS485 to a RS485 to RS232 converter, run RealTerm, set DTR, HalfDuplex, Hex display
The protocol uses 9600-8-N-1 over two wire (half duplex) RS-485.  Transmissions are in binary, 8 bit. Below transmissions are shown as HEX with a space between bytes.

Request: FF 00 FF A5 00 60 10 07 00 01 1C
Pump Response: FF 00 FF A5 00 10 60 07 0F 0A 02 01 00 E8 05 DC 37 2C 00 00 00 00 10 1B 03 8F

FF 00 FF A5 Preamble
00 10 60 07 Header – pump to control panel (no control panel, just out the wire…)
00 Unknown – perhaps a protocol version according to Tom Latfeur who says pumps are always 0
If the destination is first followed by the source, then
The request sends 60 then 10 – presumably the panel (or RealTerm) is 10, pump 60
The response sends 10 then 60 – presumably the motor is 60
10 60 appears to always be a pump status message
07 is unknown, or is the Command according to Tom L.
0F Length – 15 bytes
Payload:
0A 02 01 00 E8 05 DC 37 2C 00 00 00 00 10 1B Payload – 15 bytes
0A (becomes 04 when stopped)
02 (mode 2. becomes 00 when stopped, 01 on mode 1, )
01 (becomes FF when stopped)
00 E8 (232 watts)
05 DC (1500 rpm
37 2C =14,124 or 37=55 (not the temp which was 90F) 2c =44d – unknown
00 00 00 00 (seemingly unused)
10 1B (time, 24 hour format, this is 1627

03 8F Checksum

FF 00 FF A5 00 60 10 07 00 01 1C
FF 00 FF A5 00 10 60 07 0F 0A 02 01 00 E8 05 DC 37 2C 00 00 00 00 10 1B 03 8F
FF 00 FF A5 00 60 10 07 00 01 1C
FF 00 FF A5 00 10 60 07 0F 0A 02 01 00 E8 05 DC 37 2C 00 00 00 00 10 1B 03 8F
FF 00 FF A5 00 60 10 07 00 01 1C
FF 00 FF A5 00 10 60 07 0F 0A 02 01 00 E6 05 DC 37 2C 00 00 00 00 11 1E 03 91
FF 00 FF A5 00 60 10 07 00 01 1C

FF 00 FF A5 00 10 60 07 0F 0A 02 01 00 E6 05 DC 37 2C 00 00 00 00 10 29 03 9B
10 29 = 16:41 < correct time, 230w, 1500rpm, prog 2

FF 00 FF A5 00 10 60 07 0F 0A 01 01 00 2A 02 EE 37 2C 00 00 00 00 10 31 02 F5
4:49PM=10 31=1649; prog 1, 42w, 750RPM

FF 00 FF A5 00 10 60 07 0F 04 00 FF 00 00 00 00 37 2C 00 00 00 00 11 0A 02 AC
5:10PM, OFF (no progs running), 0 rpm, 0 watts

FF 00 FF A5 00 10 60 07 0F 0A 00 01 00 00 00 00 37 2C 00 00 C0 2D 12 0B 02 A3
turned back to auto ON, not currently running a mode.

FF 00 FF A5 00 10 60 07 0F 0A 02 01 00 E8 05 DC 37 2C 00 00 00 00 12 12 03 88
Back to mode 2 around 6:10PM

## Jason | September 26, 2016

Hello Charles,
The CFI byte is slowly being decoded by the guys on Gitter. I’m kind of stalled out lately due to a lot of work commitments but I took a look there and see that the CFI byte (aka command byte) 7 = PUMP STATUS. Take a look at this link and see if this helps any. Remember the CFI/command byte will always follow the source byte.

## Vladimir | January 27, 2017
Maybe you can try https://docklight.de/ to monitor full duplex data flow. They also have scripting for some more advanced data processing

## Nick C | November 16, 2016
Hello Jason, Thanks for a very impressive write up. It will help me a lot when we finish building our pool!
One question, where you breakdown the contents of the pump message (not in cleaner mode) I feel you must have an “A” missing after the length byte otherwise the payload is only 14 bytes long. i.e. shouldn’t there be 3 bytes between the length and watth bytes?

## Zuntara | January 30, 2017
Hi Jason,

Thanks for all the info ! This helped me a lot in controller my pool. I’m currently making an Arduino library for pentair (for the pumps mostly) and your insights and information helped me a lot!

I’ve put it all on github so anyone may feel free to contribute.

https://github.com/Zuntara/Arduino.Pentair