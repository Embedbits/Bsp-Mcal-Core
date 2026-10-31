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
dwt_RequestState_t Dwt_Get_DefaultConfig(dwt_Config_t *config);

### Module Version

dwt_ModuleVersion_t Dwt_Get_ModuleVersion(void);

### Time base (cycle counter as a clock for other modules)

dwt_RequestState_t Dwt_Get_State(dwt_FunctionState_t *state);
dwt_RequestState_t Dwt_Get_CycleFrequency(dwt_Frequency_Hz_t *freq);
dwt_RequestState_t Dwt_Get_ElapsedTime_us(dwt_CounterValue_t startCycle, dwt_Time_us_t *elapsed);

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

## Time base for other modules

Modules that have to wait for a start-up or settling time (OpAmp, Comp, ...)
measure the wait with the cycle counter instead of counting calls of their
`<Mod>_Task`:

1. `Dwt_Get_State` tells whether the cycle counter runs (DEMCR.TRCENA and
   DWT_CTRL.CYCCNTENA set). A module that needs the time base returns its error
   value from `<Mod>_Init` when the counter does not run. The getter has no
   side effects.
2. At the start of the wait the module stores `Dwt_Get_CycleCounter()`.
3. `Dwt_Get_ElapsedTime_us( startCycle, &elapsed )` returns the time since the
   stored value in microseconds. The difference is computed in unsigned 32-bit
   arithmetic, so one wrap-around of the counter is handled. The result is
   truncated (rounded down) to whole microseconds and saturates at 2^32 - 1.
   The function returns `DWT_REQUEST_ERROR` when the counter does not run or
   the pointer is NULL.
4. `Dwt_Get_CycleFrequency` returns the frequency of the counter (the core clock,
   HCLK) in Hz. It returns `DWT_REQUEST_ERROR` when the counter does not run.

### Source of the frequency

The cycle counter runs with HCLK. The frequency is taken from the CMSIS variable
`SystemCoreClock`, so Dwt does not call the Rcc API (the variable itself is
defined in `Rcc.c`, therefore a firmware that uses these getters has to link
Rcc, which every platform build does). Rcc refreshes `SystemCoreClock` in
`Rcc_Init` and in `Rcc_Set_OscDiv` (HSI divider). It does not refresh it after a
later `Rcc_Set_ClkBusDivider` of the AHB bus: after such a change the elapsed
time is computed with the old frequency until the variable is updated by the
application (`LL_SetSystemCoreClock`).

### Rules for the users of the time base

- Nobody shall call `Dwt_Reset_CycleCounter` (or `Dwt_Deinit`) while another module
  measures a wait: the stored start value would no longer match the counter.
- CYCCNT stops while the core clock is stopped (Sleep, Stop): the measured waits
  get longer, never shorter, so a start-up time is never cut short.
- The counter wraps after 2^32 cycles (about 17 s at 250 MHz). A wait longer than
  that cannot be measured; start-up and settling waits are some microseconds.
- The counter also stops in debug halt; a breakpoint in the middle of a wait
  prolongs it.

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

Default configuration (`Dwt_Get_DefaultConfig`): cycle counter only.

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
