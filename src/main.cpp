#include <SPI.h>

constexpr int spiCsPin = 9;
constexpr int canIntPin = 2;

#include "mcp2515_can.h"
mcp2515_can CAN(spiCsPin);

unsigned int canId = 0;
unsigned char canInput[8];
bool input = false;

void setup() {
    SERIAL_PORT_MONITOR.begin(115200);
    while (!Serial) {
    };

    while (CAN_OK != CAN.begin(CAN_500KBPS))
        delay(100);
}

void taskCanRecv() {
    unsigned char len = 0;
    unsigned char buf[8];

    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        CAN.readMsgBuf(&len, buf);

        SERIAL_PORT_MONITOR.println(CAN.getCanId(), HEX);
        for (int i = 0; i < len; i++)
            SERIAL_PORT_MONITOR.print(buf[i], HEX);
        SERIAL_PORT_MONITOR.println();
    }
}

void taskCanInput() {
    // example data: 123 00 00 00 00 00 00 00 00
    while (SERIAL_PORT_MONITOR.available()) {
        const auto strInput = SERIAL_PORT_MONITOR.readStringUntil('\n');

        if (strInput.length() > 0) {
            input = true;
            
            const auto strId = strInput.substring(0, 3);
            const auto strData = strInput.substring(4, strInput.length());

            canId = strtol(strId.c_str(), nullptr, 16);

            for (int i = 0; i < 8; i++) {
                const auto strByte = strData.substring(i * 2, i * 2 + 2);
                canInput[i] = strtol(strByte.c_str(), nullptr, 16);
            }

            const int emptyData = 8 - strData.length() / 2;
            if (emptyData > 0)
                for (int i = 8; i > 8 - emptyData; i--)
                    canInput[i] = 0;
        }
    }
}

void loop() {
    taskCanRecv();
    taskCanInput();

    if (input) {
        input = 0;
        CAN.sendMsgBuf(canId, 0, 8, canInput);
        for (int i = 0; i < 8; i++)
            canInput[i] = 0;
    }
}
