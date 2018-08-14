#ifndef _WATER_CLIENT_H
#define _WATER_CLIENT_H

#include "Arduino.h"
#include <optional>

class WaterClient
{
public:

	typedef UserId	uint32_t;
	typedef Pin		uint64_t;
	typedef RfidId	uint64_t;
	typedef Credit	uint32_t;

	struct LoginReply
	{
		enum class Status
		{
			SUCCESS,
			INVALID_ID, // invalid UserId or invalid RfidId
			INVALID_PIN,
			TIMEOUT, // could not connect to server within connectTimeoutSec period
			SERVER_INTERNAL_ERROR // connect to server succeeded, but e.g web service or database on server not working
		};

		Status status;
		std::optional<Credit> creditAvail; // empty iff non-success status
	};

	WaterClient(HardwareSerial*, uint8_t slaveNum, uint32_t connectTimeoutSec);
	~WaterClient();

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
	static std::optional<RfidId> CreateRfidIdFromStringInHex(std::string const &);

private:

	// ...

};

#endif
