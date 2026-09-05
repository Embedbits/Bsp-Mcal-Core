# DWT – Data Watchpoint and Trace Unit  
MCAL Driver (`Dwt_Lib`)

The **Data Watchpoint and Trace Unit (DWT)** module provides access to Cortex-M
debugging and performance-monitoring counters such as:

- CPU Cycle Counter
- CPI Counter (Cycles Per Instruction)
- Exception Counter
- Sleep Counter
- Load/Store Counter (LSU)
- Folded Instruction Counter

This MCAL driver implements a clean, hardware-abstracted public interface and
provides full configurability through `dwt_Config_t`.

---

## 🧩 Architecture Overview

Dwt_Lib
 ├── Dwt_Port.h
 ├── Dwt_Types.h
 └── Dwt.c/.h

Application code shall only include `Dwt_Port.h`.

---

## 🚀 Features

- Fully configurable enabling/disabling of all DWT counters
- Safe initialization/deinitialization API
- Runtime counter read and reset functionality
- Module version information
- Minimal hardware coupling
- No global state exposed

---

## 📦 Public API Summary

### Initialization & Lifecycle

dwt_RequestState_t Dwt_Init(dwt_Config_t *config);  
void               Dwt_Deinit(void);  
void               Dwt_Task(void);

### Module Version

dwt_ModuleVersion_t Dwt_Get_ModuleVersion(void);

### Counter APIs

void               Dwt_Reset_CycleCounter(void);  
dwt_CounterValue_t Dwt_Get_CycleCounter(void);

void               Dwt_Reset_CpiCounter(void);  
dwt_CounterValue_t Dwt_Get_CpiCounter(void);

void               Dwt_Reset_ExceptionCounter(void);  
dwt_CounterValue_t Dwt_Get_ExceptionCounter(void);

void               Dwt_Reset_SleepCounter(void);  
dwt_CounterValue_t Dwt_Get_SleepCounter(void);

void               Dwt_Reset_LoadStoreCounter(void);  
dwt_CounterValue_t Dwt_Get_LoadStoreCounter(void);

void               Dwt_Reset_FoldCounter(void);  
dwt_CounterValue_t Dwt_Get_FoldCounter(void);

---

## ⚙️ Configuration Structure

typedef struct
{
    dwt_FunctionState_t CycleCounterState;
    dwt_FunctionState_t CpiCounterState;
    dwt_FunctionState_t ExceptionCounterState;
    dwt_FunctionState_t SleepCounterState;
    dwt_FunctionState_t LoadStoreCounterState;
    dwt_FunctionState_t FoldCounterState;
} dwt_Config_t;

---

## 🛠 CMake Integration

add_library(Dwt_Lib ...)  
target_include_directories(Dwt_Lib PUBLIC .)

Usage:

target_link_libraries(App PRIVATE Dwt_Lib)

---

## License

This project is licensed under the [CC BY-NC](LICENSE.md) license.  
You are free to use, modify, and share for **non-commercial purposes** with attribution.
