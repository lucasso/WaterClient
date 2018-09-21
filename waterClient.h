#ifndef _WATER_CLIENT_H
#define _WATER_CLIENT_H

#include "Arduino.h"
#include "WString.h"
#include "ModbusRTUSlave.h"
#include <option.h>

#define SEND_BUFFER_SIZE_BYTES 16
#define RECEIVE_BUFFER_SIZE_BYTES 10

// uncomment below line to have debug messages logged on standard arduino Serial
#define WATER_DEBUG

class WaterClient
{
public:

	typedef uint32_t UserId;
	typedef uint64_t Pin;
	typedef uint64_t RfidId	;
	typedef uint32_t Credit	;

	struct LoginReply
	{
		enum class Status : byte
		{
			SUCCESS = 1,
			INVALID_ID, // invalid UserId or invalid RfidId
			INVALID_PIN,
			TIMEOUT, // could not connect to server within connectTimeoutSec period
			SERVER_INTERNAL_ERROR // connect to server succeeded, but e.g web service or database on server not working
		};

		Status status;
		Option<Credit> creditAvail; // empty iff non-success status
	};

	WaterClient(uint8_t controlPinNumber, HardwareSerial*, uint8_t slaveNum, uint32_t connectTimeoutSec);

	// In every 'loop' call client code should call one of the following methods.
	// If client does not need to log in or log out currently, then keepCommunicationAlive should be repeated in every 'loop' call.
	// Please do not hang in some other operation without calling 'keepCommunicationAlive' for a long time because this will slow
	// down communication with other Arduino slave devices.

	LoginReply loginByUser(UserId, Pin);
	LoginReply loginByRfid(RfidId);
	void logout(Credit creditConsumed);

	void keepCommunicationAlive();

	// In case when one has string instead of RfidId he may call below converter function.
	// If strng contains valid hex number, then return value will be non-empty, otherwise it will be an empty option.
	static Option<RfidId> CreateRfidIdFromStringInHex(String const &);

	typedef uint8_t RequestSeqNum; // used internally by class only

private:

	template <typename RequestTypeT, typename RequestImplT>
	LoginReply loginImpl(RequestTypeT, RequestImplT const &, Credit);

	ModbusRTUSlave rtu;
	RequestSeqNum nextRequestId;
	byte sendBuffer[SEND_BUFFER_SIZE_BYTES];
	byte receiveBuffer[RECEIVE_BUFFER_SIZE_BYTES];
	uint32_t timeoutSec;

	struct LoginByUserRequest
	{
		UserId userId;
		Pin pin;
	};

	struct LoginByRfidRequest
	{
		RfidId rfidId;
	};

	// only one of below options may be non-empty
	Option<LoginByUserRequest> lastLoginByUser;
	Option<LoginByRfidRequest> lastLoginByRfid;

	union RequestImpl;
	struct Request;
};

#ifdef WATER_DEBUG
# define LOG(...) \
	Serial.println(__VA_ARGS__)
#else
# define LOG(...)
#endif

#endif
