# ESP-NOW Auto Pairing Library

A library that provides automatic pairing functionality using ESP-NOW between ESP32 devices.

## Features

- Selectable Master/Slave roles at instantiation
- Automatic saving of pairing results to non-volatile memory (EEPROM)
- Functions for reading and clearing pairing data
- Simple API for ESP-NOW communication

## Installation

1. Copy this library folder to the Arduino IDE libraries folder
2. Restart Arduino IDE

## Basic Usage

### 1. Include the Library

```cpp
#include <ESPNowAutoPairing.h>
```

### 2. Create Instance

```cpp
// For Master use
ESPNowAutoPairing espNow(MASTER);

// For Slave use
ESPNowAutoPairing espNow(SLAVE);
```

### 3. Initialization

```cpp
void setup() {
    Serial.begin(115200);
    espNow.begin();
}
```

## API Reference

### Constructor

```cpp
ESPNowAutoPairing(DeviceRole role)
```

- `role`: Specify `MASTER` or `SLAVE`

### Methods

#### `void begin()`
Initialize the library. Call this in setup().

#### `void startPairingMode()`
Start pairing mode.

#### `void stopPairingMode()`
Stop pairing mode.

#### `bool isPaired()`
Check if pairing is complete.

**Return value:**
- `true`: Paired
- `false`: Not paired

#### `uint8_t* getPairedMacAddress()`
Get the paired MAC address.

**Return value:**
- Pointer to paired MAC address (6 bytes)

#### `void clearPairingData()`
Clear saved pairing data.

#### `void sendData(uint8_t* data, size_t len)`
Send data to paired device.

**Parameters:**
- `data`: Pointer to data to send
- `len`: Data size

#### `void setUserRecvCallback(void (*callback)(const uint8_t*, const uint8_t*, int))`
Set user-defined receive callback function.

**Parameters:**
- `callback`: Function pointer called on data reception

## Pairing Procedure

### Master Side Procedure
1. Call `startPairingMode()` to enter pairing mode
2. Broadcast PAIR_REQUEST
3. Receive PAIR_RESPONSE from Slave
4. Send PAIR_CONFIRM to complete pairing

### Slave Side Procedure
1. Call `startPairingMode()` to enter pairing mode
2. Wait for PAIR_REQUEST from Master
3. Send PAIR_RESPONSE when PAIR_REQUEST is received
4. Receive PAIR_CONFIRM from Master to complete pairing

## Message Structure

```cpp
typedef struct {
    uint8_t type;        // Message type
    uint8_t mac[6];      // Sender's MAC address
    uint8_t data[8];     // Additional data
} pairing_message_t;
```

### Message Types

- `PAIR_REQUEST (0x01)`: Pairing request
- `PAIR_RESPONSE (0x02)`: Pairing response
- `PAIR_CONFIRM (0x03)`: Pairing confirmation
- `NES_COMMAND (0x10)`: Normal data communication

## Important Callback Function Notes

**⚠️ Important**: Drawing operations with M5.Lcd inside ESP-NOW callback functions will cause memory violations.

### Problem Cause
- ESP-NOW callback functions execute in interrupt context
- M5.Lcd drawing operations internally perform task switching and memory allocation
- SPI communication conflicts may occur

### Correct Implementation

```cpp
// Manage data with global variables
pairing_message_t msg;
bool RCVD = false;

// Only minimal processing in callback function
void OnDataRecvUser(const uint8_t *mac_addr, const uint8_t *incomingData, int data_len) {
  if (data_len >= (int)sizeof(pairing_message_t)) {
    RCVD = true;
    memcpy(&msg, incomingData, sizeof(msg));
  }
}

// Safe drawing operations in main loop
void loop() {
  if(RCVD == true && PAIR_STATUS == PAIRED) {
    if (msg.type == NES_COMMAND) {
      // Safe drawing operations here
      M5.Lcd.printf("Received: %02X %02X...", msg.data[0], msg.data[1]);
    }
    RCVD = false;  // Reset flag
  }
}
```

## Usage Examples

See the `examples` folder for detailed usage examples:

- `MasterAtomS3_M5Unified`: Master side implementation using AtomS3
- `SlaveM5StackS3_M5Unified`: Slave side implementation using M5StackS3

## Sample Code Features

### Display Improvements
- Appropriate screen display according to pairing status
- Screen clearing only on status changes (prevents flickering)
- Visual display of MAC addresses and transmitted/received data

### Operation Methods
- **Master/Slave Common**: Long press Button A (1 second) to start pairing mode
- Automatic pairing data clear function
- Real-time data transmission/reception display

### Data Communication
- MAC address transmission every 2 seconds
- Communication in 8-byte data format
- Detailed logging via serial output

## Notes

- Uses 7 bytes of EEPROM (6 bytes MAC address + 1 byte pairing status)
- Normal data communication is not performed during pairing mode
- Once pairing is complete, settings are retained even after power off
- **Do not perform drawing operations in callback functions** (causes memory violations)
- Tested with M5Unified library

## License

This library is released under the MIT License.
