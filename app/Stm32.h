/*
 * Stm32.h
 *
 *  Created on: Jun 28, 2016
 *      Author: lieven
 */

#ifndef STM32_H_
#define STM32_H_

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Services/ArduinoJson/include/ArduinoJson.h>
#include <Services/WebHelpers/base64.h>
#include <Bytes.h>

class Stm32 {
	static bool _alt_serial;
	static bool _boot0;
	static uint64_t _timeout;
public:
	enum Op {
		X_WAIT_ACK = 0x40,
		X_SEND = 0x41,
		X_RECV = 0x42,
		X_RECV_VAR = 0x43,
		X_RECV_VAR_MIN_1 = 0x44,
		X_RESET,
		X_BOOT0
	};
	static Erc begin();
	static Erc reset();
	static Erc setBoot0(bool);
	static Erc setAltSerial(bool);
	static Erc engine(Bytes& reply, Bytes& req);
	static bool timeout();
	static void timeout(uint32_t delta);
};

#endif /* STM32_H_ */
