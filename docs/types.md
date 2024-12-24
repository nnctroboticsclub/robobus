
## Generic

```txt
T[]: {
  len: u16
  data: T[len]
}
```

## Device

```txt
Device {
  path: u8 -> Str // Internal
  device_id: u8 // Internal

  proto_ver: u32 // MM mm Pt Cm
  sManu, sRod, sSerial: u8 -> Str

  interfaces: u8
  peripheral_id: u8
}
```

## Interface

```txt
Interface {
  // Serialize
  intf_id: u16
  name: u8 -> Str

  ui: Component
  config: {
    map_id: u16
    slot: u32
    type: u8
    name: u8[]
  }[]
  data: NodeMap,
  actions: {map_id: u16, action_id: u32}-
  monitor_layout: MonitorLayout
}
```


## UI

```txt
Component {type: u16, data: ...}
  0000[Grid]
    rows: Size[], cols: Size[]
    parts: {
      (x, y, w, h): u16,
      c: Component
    }[]
  0001[Free]
    part: {
      base: [T|B][L|R]
      (x, y, w, h): Size
      c: Component
    }[]
  0002[Tab]
    tabs: {
      name: u8[]
      page: u16
      c: Component
    }[]
  0003[Auto]
    part: Component[]
  0100[Label]
    text: u8[] @enc (utf8)
  0101[Action]
    map_id: u16
    inner: Component
  0102[NVS]
    map_id: u16
  0103[Link]
    intf: u16
    page: u16
```