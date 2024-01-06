
## Wire

(opposite of the panel protocol)
 - long pulse = 0
 - short pulse = 1

## Protocol Frames

two frames are sent to the panel regularly 
Bytes are in reverse order ; LSB on the left.

### Frame A

Contains the configurations of the board

```
Bytes :    A1       A2       A3       A4       A5       A6       A7       A8       A9       A10
Sample: 01001011 01101000 01011000 10110100 11100000 10110000 00001010 00111100 10111111 11101000
Value 
after   ..210.....tC.22.....26 th.....45.......7........13.......80.......60.......253......23
reverse
```

| Byte | Value |
| --- | --- |
| A1 | - |
| A2 | Target Temp Cooling |
| A3 | Target Temp Heating |
| A4 | Defrost cycle length |
| A5 | Defrost start temp trigger (minus) from -30 to -0 |
| A6 | Defrost stop temp trigger |
| A7 | - |
| A8.1 | Number of systems |
| A8.2 | Automatic restart |
| A8.3-4 | -  |
| A8.7 | HEAT O |N

### Frame B

Contains more the sensors values
```
   B1       B2       B3       B4       B5       B6       B7       B8       B9       B10
10111011 01001000 00101000 11101000 00000000 00011000 00000000 00000000 10101010 11101000
....221....A:18.....B:20.....C:23.......0........24.......0........0........85.......23
```

| Byte | Value |
| --- | --- |
| B1 | header
 |
| B2 | Intake water temp sensor |
| B3 | Exhaust water temp sensor |
| B4 | Condenser temp sensor |
| B5 | Canceling? |
| B6 | External temp |
| B7 | - |
| B8 | - |
| B9 | - |

### Remarks on A10 and B10

 - B10 == A10
 - a???????
 - a= 1 if temp is .5

