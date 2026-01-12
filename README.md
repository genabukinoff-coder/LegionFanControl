# LegionFanControl NEW

This is a dirty viby steamy FORK of https://github.com/0x1F9F1/LegionFanControl

```text
╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║           !!!   EXTREME RISK & DISCLAIMER   !!!              ║
║         This is NOT a tool for daily or casual use.          ║
║                                                              ║
╠══════════════════════════════════════════════════════════════╣
║                                                              ║
║   This program grants raw, unfiltered write-access to your   ║
║   laptop's Embedded Controller (EC). It is a proof-of-concept║
║   that bypasses all manufacturer-installed thermal logic.    ║
║                                                              ║
║   Let it be known: The author, after its creation, deemed    ║
║   it too volatile and dangerous for regular operation and    ║
║   **DOES NOT USE THIS TOOL ON THEIR OWN MACHINE.**           ║
║                                                              ║
║   This repository exists for **research purposes only**.     ║
║                                                              ║
║   You are solely responsible for any damage, permanent or    ║
║   otherwise, that may result from its use. You have been     ║
║   warned.                                                    ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝
╔══════════════════════════════════════════════════════════════╗

THE TEMPERATURE START TO SLOWLY RISE IF YOU TURN OFF THE FANS COMPLETELY!

At ~75 degrees CPU experiences heavy throttling making the laptop unusable, even with cooling stand and everything tweaked with lenovo legion toolkit so I haven't been able to use the laptop without the fans unfortunately, which was the point of the project.

╚══════════════════════════════════════════════════════════════╝
```
NEITHER I NOR GEMINI ASSUME ANY RESPONSIBILITY WHATSOEVER



This program allows you to tweak the fans of Legion Pro 5 16IRX8 (SO IT IS GENERATION 8 year 2023).

All thanks to the previous author, to SmokelessCPUv2, Zandyz, and Underv0lti, and to Gemini, guided by my wise nudges.

//The whole program is a simple, single .cpp file so feel free to inspect/build it yourself

You may need to disable Core Isolation/Memory Integrity and change this registry key 
VulnerableDriverBlocklistEnable to 0 
at
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\CI\Config

you can tweak the values in the .ini file.

Available options:
  [1] Apply CUSTOM Fan Curve (from fancurve.ini)
  [2] Restore DEFAULT Fan Curve
  [3] READ and Display Current Fan Curve from EC
  [4] DUMP Full EC Memory to ec_dump.txt
  [5] Exit

the default curve in the source:
```code
const std::vector<FanCurvePoint> defaultCurve = {
    {0, 0, 5, 5, 67, 0, 53, 0, 40, 0},               // Level 0 (from 0xDF00)
    {17, 17, 5, 5, 67, 63, 53, 50, 45, 35},          // Level 1 (from 0xDF10)
    {19, 19, 5, 5, 67, 63, 53, 50, 50, 40},          // Level 2 (from 0xDF20)
    {21, 21, 5, 5, 67, 63, 53, 50, 127, 45},         // Level 3 (from 0xDF30)
    {23, 22, 2, 2, 72, 63, 56, 50, 127, 127},        // Level 4 (from 0xDF40)
    {25, 27, 2, 2, 77, 67, 59, 53, 127, 127},        // Level 5 (from 0xDF50)
    {29, 29, 2, 2, 80, 72, 65, 56, 127, 127},        // Level 6 (from 0xDF60)
    {34, 35, 2, 2, 84, 77, 68, 62, 127, 127},        // Level 7 (from 0xDF70)
    {37, 37, 2, 2, 88, 80, 75, 65, 127, 127},        // Level 8 (from 0xDF80)
    {44, 46, 2, 2, 91, 84, 85, 69, 127, 127},        // Level 9 (from 0xDF90)
    {54, 54, 2, 2, 127, 88, 127, 81, 127, 127}       // Level 10 (from 0xDFA0)};  
    ↑    ↑               ↑ 
    ↑    ↑               ↑ 
    the values in the first two columns are the speed of the fans at certain temperatures, the value in the third column is the temperature
```

