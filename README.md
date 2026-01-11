# LegionFanControl NEW

This is a dirty viby steamy FORK of https://github.com/0x1F9F1/LegionFanControl

NEITHER I NOR GEMINI ASSUME ANY RESPONSIBILITY WHATSOEVER

This makes your Legion Pro 5 16IRX8 (SO IT IS GENERATION 8 year 2023) finally shut the fans off at low temps. Perfect with lenovo legion toolkit custom profiles and disabled intel boost. Save the health of your fans today!

All thanks to the previous author, to SmokelessCPUv2, Zandyz, and Underv0lti, and to Gemini, guided by my wise nudges.

//the whole program is a simple, single .cpp file so feel free to inspect/build it yourself

//If you don't know how to build the project don't worry I can't create a standalone .exe too, we're all learning... Just download the free Visual studio open the .sln and press the Green Button. Also you may need to disable Core Isolation/Memory Integrity and change this registry key 
VulnerableDriverBlocklistEnable to 0 
at
HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\CI\Config

you can tweak the values at LegionFanControl.cpp line 124:

const std::vector<FanCurvePoint> silentCurve = {
    {0, 0, 5, 5, 67, 0, 53, 0, 40, 0},               // Level 0: OFF
    {0, 0, 5, 5, 67, 63, 53, 50, 45, 35},            // Level 1: OFF
    {0, 0, 5, 5, 67, 63, 53, 50, 50, 40},            // Level 2: OFF
    {0, 0, 5, 5, 67, 63, 53, 50, 127, 45},           // Level 3: OFF
    {0, 0, 2, 2, 72, 63, 56, 50, 127, 127},        // Level 4: Fans ON at 72C
    {0, 0, 2, 2, 77, 67, 59, 53, 127, 127},        // ... subsequent levels are same as default
    {29, 29, 2, 2, 80, 72, 65, 56, 127, 127},
    {34, 35, 2, 2, 84, 77, 68, 62, 127, 127},
    {37, 37, 2, 2, 88, 80, 75, 65, 127, 127},
    {44, 46, 2, 2, 91, 84, 85, 69, 127, 127},
    {54, 54, 2, 2, 127, 88, 127, 81, 127, 127}
    ↑    ↑
    ↑    ↑ 
    put more or less zeroes in these two columns




