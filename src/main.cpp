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
    while (SERIAL_PORT_MONITOR.available()) {
        String strInput = SERIAL_PORT_MONITOR.readStringUntil('\n');

        if (strInput.length() > 0) {
            input = true;
            for (int i = 0; i < 8; i++)
                if (i == 0)
                    canId = strInput.toInt();
                else
                    canInput[i - 1] = strtoul(
                        strInput.substring(i * 3 - 2, i * 3 + 1).c_str(), NULL,
                        16);
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
