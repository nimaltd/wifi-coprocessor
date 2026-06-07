
# ESP32-All Series WiFi Co-Processor
# Do not use, Under development

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
| Dummy      | 1 byte    | M→S       | Waiting |
| Data       | 0-1536    | M↔S       | Payload (M→S for write, S→M for read) |


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
| WRBUF    | 0x01 | Data    | M→S      | Write data buffer to device |
| RDBUF    | 0x02 | Data    | S→M      | Read data buffer from device |
| WRDMA    | 0x03 | Data    | M→S      | Write bulk stream to device |
| RDDMA    | 0x04 | Data    | S→M      | Read bulk stream from device |
| SEG_END  | 0x05 | Control | —        | Segment transaction end |
| EN_QPI   | 0x06 | Control | —        | Enable QPI mode (all wires) |
| WR_END   | 0x07 | Control | —        | Write transaction end |
| READ_END | 0x08 | Control | —        | Read transaction end |
| INT1     | 0x09 | Control | —        | Interrupt/Custom command 1 |
| INT2     | 0x0A | Control | —        | Interrupt/Custom command 2 |

