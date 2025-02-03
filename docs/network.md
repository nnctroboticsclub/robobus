# Robobus CAN ID Format

| Character | Bitwidth | Desc                       |
| :-------- | -------- | :------------------------- |
| d/D       | 10b      | Device ID                  |

| Type             |   3   |   8   |   6   |   2   | ::display()              |
| ---------------- | :---: | :---: | :---: | :---: | ------------------------ |
| Pri Interface Ch |   0   |   i   |  d1   |  d2   | {d1} ==> {d2} #{i} (Pri) |
| Specific Channel |   1   |  41   |  xx   |       | rf transmit              |
| Specific Channel |   1   |  42   |  xx   |       | rf receive               |
| Sub Interface Ch |  2,3  |   i   |  d1   |  d2   | {d1} ==> {d2} #{i} (Sub) |

## Interface

## Control Transfer

### Control Marker

|    value    | Desc |
| :---------: | :--- |
| kServerData | 0    |
| kServerCtrl | 1    |
| kClientData | 2    |
| kClientCtrl | 3    |

### Control Pipe

| byte | desc     |
| :--- | :------- |
| 0    | TxSeq    |
| 1    | ^        |
| 2    | RxSeq    |
| 3    | ^        |
| 4    | ChunkCRC |
| 5    | ^        |
| 4    | FullCRC  |
| 5    | ^        |

## App
enumerate: find, respond, reset_id, set_id, get_descriptor
rbus: property, cell, method, signal
interface:
  00: enumerate
  01: SDS
  10: Ident

8ac7 2304 89e8 0000