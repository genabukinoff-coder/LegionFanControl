/*
   PM-port(62/66)
   KBC-port(60/64)
   EC-port(2E/2F or 4E/4F)
*/

#include <Windows.h>

#include "winio.h"
#pragma comment(lib, "WinIox64.lib")

//==========================The hardware port to read/write function================================
#define READ_PORT(port, data2) GetPortVal(port, &data2, 1);
#define WRITE_PORT(port, data2) SetPortVal(port, data2, 1)
//==================================================================================================

//======================================== PM channel ==============================================
#define PM_STATUS_PORT66 0x66
#define PM_CMD_PORT66 0x66
#define PM_DATA_PORT62 0x62
#define PM_OBF 0x01
#define PM_IBF 0x02
//------------wait EC PM channel port66 output buffer full-----/
/*
 * =================================================================================
 * Legion EC Writer - A tool to modify the fan curve on ITE-5507 ECs
 * =================================================================================
 *
 * This program uses the WinIO driver to directly write a custom fan curve
 * to the Embedded Controller's extended RAM. It includes a "Silent" profile
 * and the ability to restore the factory default profile discovered from your dump.
 *
 * COMPILE: Link with WinIox64.lib and ensure winio.h is in your include path.
 * RUN:     Place the compiled .exe in the same folder as WinIox64.dll/.sys and
 *          run as Administrator.
 *
 * !! WARNING !!
 * This is a low-level tool. While it includes a restore function, misuse can
 * lead to system instability or overheating if thermal limits are ignored.
 * You proceed at your own risk.
 *
 */

#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <conio.h> // For _getch()

#include "winio.h"
#pragma comment(lib, "WinIox64.lib")

 // --- Low-Level I/O Port Functions ---
UINT8 EC_ADDR_PORT = 0x4E;
UINT8 EC_DATA_PORT = 0x4F;

// Protocol to read from extended EC RAM (addresses > 0xFF)
uint8_t ECRamReadExt(unsigned short address) {
    DWORD data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, address >> 8); // High byte
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, address & 0xFF); // Low byte
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    READ_PORT(EC_DATA_PORT, data);
    return (uint8_t)data;
}

// Protocol to write to extended EC RAM (addresses > 0xFF)
void ECRamWriteExt(unsigned short address, uint8_t value) {
    DWORD data = value;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, address >> 8); // High byte
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, address & 0xFF); // Low byte
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, data);
}

// --- Fan Curve Structure and Definitions ---

// Structure to hold one point of the fan curve
struct FanCurvePoint {
    uint8_t Fan1_RPM; uint8_t Fan2_RPM;
    uint8_t Accel;    uint8_t Decel;
    uint8_t CPU_Max;  uint8_t CPU_Min;
    uint8_t GPU_Max;  uint8_t GPU_Min;
    uint8_t HST_Max;  uint8_t HST_Min;
    // The EC table has 6 empty bytes at the end of each 16-byte entry
    uint8_t padding[6];
};

// Base address for the fan curve table in your EC dump
const unsigned short FAN_CURVE_BASE_ADDR = 0xDF00;

// Hardcoded DEFAULT fan curve from your ec_dump.txt
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
    {54, 54, 2, 2, 127, 88, 127, 81, 127, 127}       // Level 10 (from 0xDFA0)
};

// Custom SILENT fan curve
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
};

// --- Core Functions ---

// Writes a full fan curve to the EC
void WriteFanCurve(const std::vector<FanCurvePoint>& curve) {
    printf("Writing %zu fan curve points to EC memory...\n", curve.size());
    for (size_t i = 0; i < curve.size(); ++i) {
        unsigned short current_addr = FAN_CURVE_BASE_ADDR + (unsigned short)(i * 0x10);
        uint8_t* data = (uint8_t*)&curve[i];

        // Write the 16 bytes for this curve point
        for (int j = 0; j < sizeof(FanCurvePoint); ++j) {
            ECRamWriteExt(current_addr + j, data[j]);
        }
    }

    // Tell the EC to apply the changes instantly
    printf("Setting instant-apply counters...\n");
    ECRamWriteExt(0xC5FE, 0x64); // 100 in decimal
    ECRamWriteExt(0xC5FF, 0x64); // 100 in decimal

    printf("Write complete.\n");
}

// Reads the current fan curve from the EC and displays it
void ReadAndDisplayCurrentCurve() {
    printf("\n--- Reading Current Fan Curve from EC ---\n");
    printf("LVL | RPM1/2 | Acc/Dec | CPU(Max/Min) | GPU(Max/Min) | HST(Max/Min)\n");
    printf("----------------------------------------------------------------------\n");

    for (int i = 0; i < defaultCurve.size(); ++i) {
        unsigned short base_addr = FAN_CURVE_BASE_ADDR + (unsigned short)(i * 0x10);

        uint8_t fan1 = ECRamReadExt(base_addr + 0);
        uint8_t fan2 = ECRamReadExt(base_addr + 1);
        uint8_t accl = ECRamReadExt(base_addr + 2);
        uint8_t decl = ECRamReadExt(base_addr + 3);
        uint8_t cmax = ECRamReadExt(base_addr + 4);
        uint8_t cmin = ECRamReadExt(base_addr + 5);
        uint8_t gmax = ECRamReadExt(base_addr + 6);
        uint8_t gmin = ECRamReadExt(base_addr + 7);
        uint8_t hmax = ECRamReadExt(base_addr + 8);
        uint8_t hmin = ECRamReadExt(base_addr + 9);

        printf("%-3d | %-4d/%-4d| %-3d/%-3d | %-4dC/%-4dC  | %-4dC/%-4dC  | %-4dC/%-4dC\n",
            i, fan1 * 100, fan2 * 100, accl, decl, cmax, cmin, gmax, gmin, hmax, hmin);
    }
    printf("----------------------------------------------------------------------\n");
}


// --- Main Program Loop ---
int main() {
    // Initialize WinIO Driver
    if (!InitializeWinIo()) {
        printf("FATAL: Failed to initialize WinIO driver.\n");
        printf("Please ensure WinIox64.dll/.sys are present and RUN AS ADMINISTRATOR.\n");
        _getch();
        return 1;
    }

    // Main menu loop
    while (true) {
        system("cls"); // Clear the screen
        printf("======================================\n");
        printf("  Legion EC Fan Curve Writer (ITE-5507)\n");
        printf("======================================\n\n");
        printf("WARNING: This is a low-level tool. Use responsibly.\n\n");
        printf("Select an option:\n");
        printf("  [1] Apply SILENT Fan Curve (Fans off at low temps)\n");
        printf("  [2] Restore DEFAULT Fan Curve (From your dump)\n");
        printf("  [3] READ and Display Current Fan Curve from EC\n");
        printf("  [4] Exit\n\n");
        printf("Your choice: ");

        char choice = _getch();
        printf("%c\n\n", choice);

        switch (choice) {
        case '1':
            printf("Applying SILENT fan curve...\n");
            WriteFanCurve(silentCurve);
            printf("\nSILENT curve applied successfully!\n");
            break;
        case '2':
            printf("Applying DEFAULT fan curve...\n");
            WriteFanCurve(defaultCurve);
            printf("\nDEFAULT curve restored successfully!\n");
            break;
        case '3':
            ReadAndDisplayCurrentCurve();
            break;
        case '4':
            printf("Exiting...\n");
            ShutdownWinIo();
            return 0;
        default:
            printf("Invalid option. Please try again.\n");
            break;
        }

        printf("\nPress any key to return to the menu...");
        _getch();
    }

    // Cleanup
    ShutdownWinIo();
    return 0;
}