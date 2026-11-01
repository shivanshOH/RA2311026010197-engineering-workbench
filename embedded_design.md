# Embedded integration concept (all interactions simulated)

## Selected devices
- Microcontroller: ST NUCLEO-F401RE (STM32 Nucleo-64 MB1136 board family).
- FPGA: Digilent Arty A7-35T (Rev. C reference-manual assumptions).
- Debug adapter: SEGGER J-Link EDU Mini (Cortex-M target connector). The adapter targets the MCU through SWD. The Arty uses its own Digilent USB-JTAG interface on J10 for FPGA configuration. This avoids treating a CPU debug probe as a generic FPGA compute/programmer.

## Labelled topology

```text
                        USB (workbench requests/status)
                 +-------------------------------------------+
                 |                                           v
          +------+-------+     USB CDC / ST-LINK VCP   +-----+----------------+
          | Host PC      |---------------------------->| NUCLEO-F401RE       |
          | Workbench UI |                             | STM32 MCU, 3.3 V I/O|
          +------+-------+                             +----+----------------+
                 | USB J10 (Digilent onboard bridge)        ^
                 +-----------------------------------------+----> Arty A7-35T
                 |                                              FPGA / 3.3 V I/O
                 | USB host power + data
          +------+-------+      keyed 9-pin to CN4 cable
          | J-Link EDU   |---------------------------------> CN4 SWD (MCU)
          | Mini         |  VTref, SWCLK, GND, SWDIO,
          +--------------+  NRST, SWO; simulated only
```

The diagram shows independent host links. No direct GPIO interconnect between the MCU and FPGA is required by the prototype. If one is added, pin standard, direction and voltage compatibility must be checked before wiring.

## Interface and electrical table

| Device/connector | Pin or link | Direction / role | Voltage and safe assumption |
|---|---|---|---|
| NUCLEO-F401RE CN1 | USB | Host power, ST-LINK and virtual COM | USB host 5 V input; MCU VDD is 3.3 V. Use the board's configured USB-power path. |
| NUCLEO-F401RE CN4 pin 1 | VDD_TARGET | Target to adapter VTref | Sense/reference only; do not inject adapter power. |
| NUCLEO-F401RE CN4 pin 2 | SWCLK | J-Link to MCU | 3.3 V target domain. |
| NUCLEO-F401RE CN4 pin 3 | GND | Common signal return | Join signal grounds as specified; this is not a power-sharing instruction. |
| NUCLEO-F401RE CN4 pin 4 | SWDIO | Bidirectional SWD data | 3.3 V target domain. |
| NUCLEO-F401RE CN4 pin 5 | NRST | Adapter to MCU reset | Active-low reset; verify solder bridge SB12 if used. |
| NUCLEO-F401RE CN4 pin 6 | SWO | MCU trace output | Optional; verify target firmware support. |
| J-Link EDU Mini target pins | 1 VTref; 2 SWDIO/TMS; 3,5 GND; 4 SWCLK/TCK; 6 SWO/TDO; 8 TDI; 10 nRESET | Adapter owns clock and output/control pins; target owns TDO/SWO | Use only the model's stated target-voltage range. Map only the signals needed for SWD to CN4. |
| NUCLEO-F401RE CN2 | ST-LINK isolation jumpers | Route onboard probe to target MCU | Remove the two CN2 jumpers for the external debug path per ST UM1724; otherwise do not contend two probes on SWD. |
| Arty A7-35T J10 | USB-JTAG/USB-UART bridge | Host to on-board Digilent bridge | Board is powered via USB or specified external 7-15 V input. Use the onboard bridge for FPGA programming. |
| Arty A7 expansion I/O | Pmod/shield GPIO | Board-specific FPGA bank pins | Arty documentation identifies recommended interface limits; use the selected bank's documented I/O standard, normally 3.3 V. Never assume a different bank or external module voltage. |

## Device discovery, capabilities and failure

The UI returns three mock device records with type, connection path and voltage note. Connect/disconnect produces a simulated telemetry string (`mock temperature 24.0 C`). The timeout control waits two seconds, returns a recoverable timeout result and leaves the UI responsive; the user may retry. No serial port is opened, no firmware is flashed and no FPGA bitstream is sent.

## Limitations and safe next steps

The Arty A7-35T reference manual specifies the shared USB-JTAG circuit and board power inputs; the J-Link EDU Mini uses a 9-pin 0.05-inch target interface. A keyed, verified adapter cable is required to connect its relevant SWD signals to NUCLEO CN4; never infer pin 1 from cable colors. Check the exact board revision, connector orientation, probe revision, target voltage and jumper state against vendor documents before any physical setup. Keep board power supplies separate and prevent 5 V GPIO from reaching 3.3 V target pins. The software mocks are for architecture/UI assessment only and establish no physical compatibility test.

## Primary references
- STMicroelectronics, [UM1724 STM32 Nucleo-64 boards (MB1136)](https://www.st.com/resource/en/user_manual/um1724-stlinkv21-in-circuit-debuggerprogrammer-for-stm8-and-stm32-stmicroelectronics.pdf), debug connector CN4 and external-debug instructions.
- Digilent, [Arty A7 Reference Manual](https://digilent.com/reference/_media/reference/programmable-logic/arty/arty_rm.pdf), board power, J10 USB-JTAG and FPGA configuration.
- SEGGER, [9-pin JTAG/SWD connector](https://kb.segger.com/9-pin_JTAG/SWD_connector), pin ownership, VTref and ground.
- SEGGER, [J-Link EDU Mini specifications](https://www.segger.com/products/debug-probes/j-link/models/j-link-edu-mini/), target-voltage range.
