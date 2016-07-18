/*
 * Stm32.cpp
 *
 *  Created on: Jul 2, 2016
 *      Author: lieven
 */

#include "Stm32.h"

#define PIN_RESET 4 // GPIO4 D2
#define PIN_BOOT0 5 // GPIO5 D1
#define ACK 0x79
#define NACK 0x1F
#define DELAY 100

bool Stm32::_alt_serial = false;
uint64_t Stm32::_timeout = 0;
bool Stm32::_boot0 = true;

Erc Stm32::setAltSerial(bool flag) {
	if (_alt_serial == flag)
		return E_OK;
	_alt_serial = flag;
	if (_alt_serial)
		system_uart_swap();
	else
		system_uart_de_swap();
	return E_OK;
}

Erc Stm32::setBoot0(bool flag) {
	if (_boot0 == flag)
		return E_OK;
	digitalWrite(PIN_BOOT0, flag);	// 1 : bootloader mode, 0: flash run
	_boot0 = flag;
	return E_OK;
}

Erc Stm32::begin() {
	Serial.systemDebugOutput(false);
	pinMode(PIN_RESET, OUTPUT);
	digitalWrite(PIN_RESET, 1);
	pinMode(PIN_BOOT0, OUTPUT);
	setBoot0(true);
	setAltSerial(true);
}

Erc Stm32::reset() {
	digitalWrite(PIN_RESET, 0);
	delay(10);
	digitalWrite(PIN_RESET, 1);
	delay(10);
	Serial.write(0x7F);	// send sync for bootloader
	return E_OK;
}

bool Stm32::timeout() {
	return _timeout < millis();
}

void Stm32::timeout(uint32_t delta) {
	_timeout = millis() + delta;
}

Erc Stm32::engine(Bytes& reply, Bytes& req) {
	Erc error;
	req.offset(0);
	uint8_t instr;
	uint16_t lengthToRead;
	reply.clear();
	while (Serial.available()) { // flush read buffer
		reply.write(Serial.read());
	}

//	LOGF(" scenario %s", req.toHex(line.clear()));
//	Serial1.write(req.data(),req.length());

	while (req.hasData() && error == E_OK) {

		instr = req.read();

		switch (instr) {
		case X_RESET: {
			LOGF("X_RESET");
			reset();
			break;
		}
		case X_WAIT_ACK: {
			LOGF("X_WAIT_ACK");
			timeout(10);
			while (true) {
				if (timeout()) {
					error = ETIMEDOUT;
					goto END;
				};
				if (Serial.available()) {
					uint8_t b;
					while (Serial.available()) {
						reply.write(b = Serial.read());

					}
					if (b == ACK)
						break;
				}
			}
			break;
		}
		case X_SEND: {

			LOGF("X_SEND");
			if (!req.hasData()) {
				error = ENODATA;
				goto END;
			}
			int length = req.read() + 1;
			LOGF("X_SEND %d", length);
			while (length && req.hasData()) {
				Serial.write(req.read());
				length--;
			}
			break;

		}
		case X_RECV: {

			LOGF("X_RECV");
			if (!req.hasData()) {
				error = ENODATA;
				goto END;
			}
			lengthToRead = req.read() + 1;
			timeout(DELAY);
			while (lengthToRead && error == E_OK) {
				if (timeout()) {
					error = ETIMEDOUT;
					goto END;
				}
				while (Serial.available()) {
					reply.write(Serial.read());
					lengthToRead--;
				}
			}
			break;

		}
		case X_RECV_VAR: {

			LOGF("X_RECV_VAR");
			uint8_t b;
			timeout(DELAY);
			while (1) {
				if (timeout()) {
					error = ETIMEDOUT;
					goto END;
				}
				if (Serial.available()) {
					reply.write(b = Serial.read());
					break;
				}
			}
			LOGF("X_RECV_VAR : %d", b);
			lengthToRead = b + 1;
			timeout(DELAY);
			while (lengthToRead && error == E_OK) {
				if (timeout()) {
					error = ETIMEDOUT;
					goto END;
				}
				while (Serial.available()) {
					reply.write(Serial.read());
					lengthToRead--;
				}
			}
			break;
		}
		}
	}
	END: return error;
}

