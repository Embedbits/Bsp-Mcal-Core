# SCB – System Control Block
MCAL Driver (`Scb_Lib`)

The **System Control Block (SCB)** module provides access to the Cortex-M
SCB_SCR register bits that control low power entry:

- SLEEPDEEP – selects deep sleep (Stop / Standby) instead of normal sleep
- SLEEPONEXIT – enters sleep immediately on return from an interrupt handler

It also wraps the two core intrinsics that enter the selected low power state:

- `__WFI()` – Wait For Interrupt
- `__WFE()` – Wait For Event

This MCAL driver implements a clean, hardware-abstracted public interface with
no module state to own - SCB is always clocked and accessible, no
initialization is required.

---

## 🧩 Architecture Overview

Scb_Lib
 ├── Scb_Port.h
 ├── Scb_Types.h
 └── Scb.c/.h

Application code shall only include `Scb_Port.h`.

---

## 🚀 Features

- Independent control of SLEEPDEEP and SLEEPONEXIT
- Read-back getters with no side effects
- Wrapped `__WFI()` / `__WFE()` low power entry
- Module version information
- No initialization/state required

---

## 📦 Public API Summary

### Module Version

scb_ModuleVersion_t Scb_Get_ModuleVersion(void);

### Deep Sleep

void                Scb_Set_DeepSleepActive(void);
void                Scb_Set_DeepSleepInactive(void);
scb_FunctionState_t Scb_Get_DeepSleepState(void);

### Sleep On Exit

void                Scb_Set_SleepOnExitActive(void);
void                Scb_Set_SleepOnExitInactive(void);
scb_FunctionState_t Scb_Get_SleepOnExitState(void);

### Low Power Entry

void                Scb_Set_WaitForInterrupt(void);
void                Scb_Set_WaitForEvent(void);

---

## 🛠 CMake Integration

add_library(Scb_Lib ...)
target_include_directories(Scb_Lib PUBLIC .)

Usage:

target_link_libraries(App PRIVATE Scb_Lib)

---

## STM32H5 implementation status

Branch `Dev/STM32H5`. Unit tests (RegMem + CmsisHost host model) pass on all
12 STM32H5 lines. Integration tests compile and link, not run on hardware.

---

## License

This project is licensed under the [CC BY-NC](LICENSE.md) license.
You are free to use, modify, and share for **non-commercial purposes** with attribution.
