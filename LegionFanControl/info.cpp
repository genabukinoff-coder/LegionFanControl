/*
   EC MEMORY DUMPER FOR LENOVO LEGION
   Based on LegionFanControl by SmokelessCPUv2, Zandyz, and Underv0lti.
   Repurposed to identify the EC and dump its full memory space.
   Usage: Compile and run. It will create ec_dump.txt.
*/

#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <conio.h>

#include "winio.h" // Assumes winio.h and WinIox64.lib are set up
#pragma comment(lib, "WinIox64.lib")

//========================== Hardware port access macros ================================
#define READ_PORT(port, data2) GetPortVal(port, &data2, 1);
#define WRITE_PORT(port, data2) SetPortVal(port, data2, 1)
//=======================================================================================

//================================ KBC/PM Channel Functions =================================
// Using PM Channel (62/66) for standard EC RAM access
#define PM_STATUS_PORT66 0x66
#define PM_CMD_PORT66 0x66
#define PM_DATA_PORT62 0x62
#define PM_OBF 0x01
#define PM_IBF 0x02

void Wait_PM_IBE(void) {
    DWORD data;
    do { READ_PORT(PM_STATUS_PORT66, data); } while (data & PM_IBF);
}

void Wait_PM_OBF(void) {
    DWORD data;
    do { READ_PORT(PM_STATUS_PORT66, data); } while (!(data & PM_OBF));
}

BYTE EC_ReadByte_PM(BYTE index) {
    DWORD data;
    Wait_PM_IBE();
    WRITE_PORT(PM_CMD_PORT66, 0x80); // Read command
    Wait_PM_IBE();
    WRITE_PORT(PM_DATA_PORT62, index);
    Wait_PM_OBF();
    READ_PORT(PM_DATA_PORT62, data);
    return (BYTE)data;
}
//=======================================================================================

//============================ Extended EC Direct Access ================================
// This is the "magic" protocol to access the full 64KB memory space
UINT8 EC_ADDR_PORT = 0x4E;
UINT8 EC_DATA_PORT = 0x4F;

unsigned char ECRamReadExt_Direct(unsigned short iIndex) {
    DWORD data;
    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x11);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex >> 8); // High byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x10);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    WRITE_PORT(EC_DATA_PORT, iIndex & 0xFF); // Low byte

    WRITE_PORT(EC_ADDR_PORT, 0x2E);
    WRITE_PORT(EC_DATA_PORT, 0x12);
    WRITE_PORT(EC_ADDR_PORT, 0x2F);
    READ_PORT(EC_DATA_PORT, data);
    return (BYTE)data;
}

#define EC_RAM_READ_EXT ECRamReadExt_Direct
//=======================================================================================

// Main function to perform the dump
int main(int argc, char* argv[]) {
    // Attempt to initialize the driver
    if (!InitializeWinIo()) {
        printf("FATAL: Failed to initialize WinIO driver.\n");
        printf("Ensure WinIox64.dll and WinIox64.sys are present and run as Administrator.\n");
        return 1;
    }
    printf("WinIO driver initialized successfully.\n");

    // Open the output file
    FILE* dumpFile;
    if (fopen_s(&dumpFile, "ec_dump.txt", "w") != 0) {
        printf("FATAL: Could not create ec_dump.txt.\n");
        ShutdownWinIo();
        return 1;
    }
    printf("Output file 'ec_dump.txt' created.\n");

    // --- Step 1: Identify the Embedded Controller ---
    printf("Reading EC identification...\n");
    fprintf(dumpFile, "--- Embedded Controller Information ---\n");

    uint8_t ec_id1 = EC_RAM_READ_EXT(0x2000);
    uint8_t ec_id2 = EC_RAM_READ_EXT(0x2001);
    uint8_t ec_ver = EC_RAM_READ_EXT(0x2002);

    printf("EC Chip ID: ITE-%02X%02X\n", ec_id1, ec_id2);
    printf("EC Firmware Version: %u\n", ec_ver);
    fprintf(dumpFile, "EC Chip ID: ITE-%02X%02X\n", ec_id1, ec_id2);
    fprintf(dumpFile, "EC Firmware Version: %u\n", ec_ver);
    fprintf(dumpFile, "--------------------------------------\n\n");


    // --- Step 2: Dump the Standard 256-byte EC RAM (PM Channel) ---
    printf("Dumping standard 256-byte EC RAM (0x00-0xFF)...\n");
    fprintf(dumpFile, "--- Standard EC RAM Dump (256 Bytes, via PM Channel) ---\n");
    fprintf(dumpFile, "Offset | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
    fprintf(dumpFile, "-------|-------------------------------------------------\n");

    for (int i = 0; i <= 0xFF; ++i) {
        if (i % 16 == 0) {
            fprintf(dumpFile, "%02X     | ", i);
        }
        fprintf(dumpFile, "%02X ", EC_ReadByte_PM((BYTE)i));
        if (i % 16 == 15) {
            fprintf(dumpFile, "\n");
        }
    }
    fprintf(dumpFile, "\n\n");

    // --- Step 3: Dump the Full 64KB Extended EC RAM (Direct I/O) ---
    printf("Dumping full 64KB extended EC RAM (0x0000-0xFFFF). This will take a moment...\n");
    fprintf(dumpFile, "--- Full Extended EC RAM Dump (64KB, via Direct I/O) ---\n");
    fprintf(dumpFile, "Address  | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
    fprintf(dumpFile, "---------|-------------------------------------------------\n");

    for (int i = 0; i <= 0xFFFF; ++i) {
        if (i % 16 == 0) {
            fprintf(dumpFile, "0x%04X   | ", i);
        }
        fprintf(dumpFile, "%02X ", EC_RAM_READ_EXT((unsigned short)i));
        if (i % 16 == 15) {
            fprintf(dumpFile, "\n");
            // Optional: Print progress to console
            if (i % 256 == 255) {
                printf("\rProgress: %d%%", (i * 100) / 0xFFFF);
            }
        }
    }
    printf("\rProgress: 100%%\n");


    // --- Cleanup ---
    fclose(dumpFile);
    ShutdownWinIo();
    printf("\nDump complete. All data saved to ec_dump.txt.\n");

    // Pause to allow user to see the output
    printf("Press any key to exit.\n");
    _getch();

    return 0;
}