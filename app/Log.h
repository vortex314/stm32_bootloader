/*
 * Log.h
 *
 *  Created on: Jun 6, 2016
 *      Author: lieven
 */

#ifndef LOG_H_
#define LOG_H_

#define LOGF(fmt,...) Serial.printf("%ld | %s:%d-%s | ", millis(),__FILE__,__LINE__,__FUNCTION__);Serial.printf(fmt,##__VA_ARGS__);Serial.println();//delay(10);
#define ASSERT_LOG(xxx) if ( !(xxx)) { LOGF(" Assertion failed %s",#xxx); while(1){};}

class Log {
private:
	static bool _on;
public:
	Log();
	virtual ~Log();
	static void on() {
		_on = true;
	}
	static void off() {
		_on = false;
	}
	inline static bool isOn() {
		return _on;
	}
	static void log(const char* function, int line, const char* fmt, ...);
};

#endif /* LOG_H_ */
