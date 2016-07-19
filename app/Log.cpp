/*
 * LOg.cpp
 *
 *  Created on: Jul 3, 2016
 *      Author: lieven
 */

#include <Log.h>
#include <stdlib.h>
#include <stdio.h>
#include <WString.h>
#include <Arduino.h>
#include <cstdio>

LogManager Log;

void serialLog(char* start, uint32_t length) {
#ifdef SMING
	for(int i=0;i<length;i++)
		Serial.write(*(start+i));
#else
	Serial.write(start, length);
	Serial.write("\r\n");
#endif
}

LogManager::LogManager() {
	_record = new char[LINE_LENGTH];
	enable();
	_offset = 0;
	defaultOutput();
}

LogManager::~LogManager() {
	delete _record;
}

bool LogManager::enabled() {
	return _enabled;
}
void LogManager::disable() {
	_enabled = false;
}
void LogManager::enable() {
	_enabled = true;
}

void LogManager::defaultOutput() {
	_logFunction = serialLog;
}

void LogManager::setOutput(LogFunction function) {
	_logFunction = function;
}
#ifdef SMING
#define vsnprintf m_vsnprintf
#endif
void LogManager::printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	if (_offset < LINE_LENGTH)
		_offset += vsnprintf((char*) (_record + _offset), LINE_LENGTH - _offset,
				fmt, args);
	va_end(args);
}

void LogManager::flush() {
	if (_logFunction)
		_logFunction(_record, _offset);
	_offset = 0;
}

