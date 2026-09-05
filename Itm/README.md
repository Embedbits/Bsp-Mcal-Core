# ITM – Instrumentation Trace Macrocell  
MCAL Driver (`Itm_Lib`)

The **Instrumentation Trace Macrocell (ITM)** module provides access to Cortex-M
instrumentation tracing channels (ITM stimulus ports). This driver exposes a clean
API for transmitting trace data, enabling timestamps, and controlling synchronization.

---

## 🧩 Architecture Overview

Itm_Lib
 ├── Itm_Port.h
 ├── Itm_Types.h
 ├── Itm.c/.h
 └── Config (optional)

Application code shall only include `Itm_Port.h`.

---

## 🚀 Features

- Transmission of 8/16/32-bit debug messages via ITM ports
- Supports all 32 ITM channels (ITM_PORT_0 – ITM_PORT_31)
- Timestamping enable/disable
- Synchronization enable/disable
- Busy-state and transmit-state reporting
- Simple initialization and deinitialization

---

## 📦 Public API Summary

### Initialization

Itm_Init(itm_Config_t *config);  
Itm_Deinit(void);

### Timestamp Control

Itm_Set_TimestampEnable(void);  
Itm_Set_TimestampDisable(void);

### Sync Control

Itm_Set_SyncEnable(void);  
Itm_Set_SyncDisable(void);

### Data Transmission

Itm_Transmit_8b (itm_Port_t port, uint8_t data);  
Itm_Transmit_16b(itm_Port_t port, uint16_t data);  
Itm_Transmit_32b(itm_Port_t port, uint32_t data);

### Status

Itm_Get_TransmitState(itm_Port_t port);  
Itm_Get_BusyState(void);

---

## ⚙️ Configuration Structure

typedef struct
{
    itm_Port_t          PortId;
    itm_FunctionState_t TimestampState;
    itm_FunctionState_t SyncState;
} itm_Config_t;

---

## 🛠 CMake Integration

add_library(Itm_Lib ...)  
target_include_directories(Itm_Lib PUBLIC .)

Usage:

target_link_libraries(App PRIVATE Itm_Lib)

---

## License

This project is licensed under the [CC BY-NC](LICENSE.md) license.  
You are free to use, modify, and share for **non-commercial purposes** with attribution.