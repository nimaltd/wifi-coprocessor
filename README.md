
# ESP32-C5 WiFi Co-Processor

**SPI/QSPI Slave Interface with Dynamic Mode Switching (1/2/4-bit)**

A WiFi co-processor implementation using ESP32-C5 that communicates via SPI/QSPI slave mode. Supports dynamic switching between 1-bit (standard SPI), 2-bit (dual), and 4-bit (quad) modes.

---

## Communication Protocol

### Command Structure

Every SPI transaction follows this frame format:

| Phase      | Size      | Direction | Field Content |
|------------|-----------|-----------|---------------|
| Command    | 1 byte    | M→S       | [IO Mode(4 bits)][Command(4 bits)] |
| Address    | 1 byte    | M→S       | Parameter or buffer address |
| Data       | 0-2048    | M↔S       | Payload (M→S for write, S→M for read) |
| **TOTAL**  | **2-2050**| —         | Minimum 2 bytes, maximum 2050 bytes |

### Command Byte Format

The command byte encodes both the I/O mode and command type:

```
Bit 7 6 5 4 | 3 2 1 0
    -------+-------
    IO Mode | Command
```

**Byte Breakdown:**
- **Bits [7:4]** = I/O Mode (which wires to use)
- **Bits [3:0]** = Command (what operation to perform)

**Upper 4 bits (IO Mode):**

| IO Mode  | Value | Wire Count | Description |
|----------|-------|-----------|-------------|
| 1-bit    | 0x0   | 1 wire    | Standard SPI (MOSI/MISO) |
| Dual Out | 0x1   | 2 wires   | Output only on 2 wires |
| Dual I/O | 0x2   | 2 wires   | Bidirectional on 2 wires |
| Quad Out | 0x4   | 4 wires   | Output only on 4 wires |
| Quad I/O | 0x8   | 4 wires   | Bidirectional on 4 wires |

**Lower 4 bits (Command):**

| Command  | Hex  | Type    | Data Dir | Purpose |
|----------|------|---------|----------|---------|
| EXQPI    | 0x00 | Control | —        | Exit Quad mode → 1-bit SPI |
| ENQPI    | 0x01 | Control | —        | Enter Quad mode (all wires) |
| WRBUF    | 0x02 | Data    | M→S      | Write data buffer to device |
| WRDMA    | 0x03 | Data    | M→S      | Write bulk stream to device |
| RDBUF    | 0x04 | Data    | S→M      | Read data buffer from device |
| RDDMA    | 0x05 | Data    | S→M      | Read bulk stream from device |
| SEG_DONE | 0x06 | Control | —        | Segment complete marker |
| WR_DONE  | 0x07 | Control | —        | Write stream complete |
| CMD8     | 0x08 | Control | —        | Reserved control command |
| CMD9     | 0x09 | Control | —        | Reserved control command |
| CMDA     | 0x0A | Control | —        | Reserved control command |

### Example Transactions

**Write WiFi Config (Master → Device):**
```
Master sends 2048 bytes to device:
  [0x02] [addr] + [SSID "network"] + [password "pass123"] + [padding...]
   ↓      ↓
 WRBUF  addr    Device receives in com.rx_data

Device parses credentials and connects to WiFi
```

**Read WiFi Status (Device → Master):**
```
Master requests status from device:
  [0x04] [addr]
   ↓      ↓
 RDBUF  addr    Device fills com.tx_data with:
               [connected=1][signal=-45dBm][unused][unused]...
               
Device sends 2048 bytes back to master
```

**Switch to Quad Mode (1-bit → 4-bit):**
```
Master: [0x01] [addr]  ==  ENQPI command
Device: Switches all buses (D0-D3) to quad mode

All following commands use 4-wire protocol until EXQPI received
```

### Data Directions

- **M→S (WRBUF/WRDMA)**: Master uploads data → device processes in `com.rx_data`
- **S→M (RDBUF/RDDMA)**: Device sends data from `com.tx_data` → master reads
- **Control (others)**: No data phase, just logging

### QPI (Quad SPI) Mode

- Send **ENQPI (0x01)** to enter quad mode
- Send **EXQPI (0x00)** to exit quad mode back to 1-bit SPI
- Hardware automatically switches between 1/2/4-wire based on mode

### Event Queue Format

When device has pending status changes (WiFi connected, scan results, signal strength):

| Field | Size | Content | Notes |
|-------|------|---------|-------|
| Type  | 1 byte | Event type identifier | User-defined event code |
| Param | 1 byte | Event parameter | Command-specific data |
| Len_H | 1 byte | Data length (high byte) | Total data size in big-endian |
| Len_L | 1 byte | Data length (low byte) | Combined: (Len_H << 8) \| Len_L |
| Data  | 0-2040 | Event payload | Raw event data |
| **Max Frame** | **2048** | Complete event frame | Total = 8 + data_size |

**Master reads devices via RDBUF command sequentially.**

---

**Target**: ESP32-C5  
**IDF Version**: v5.5.3
