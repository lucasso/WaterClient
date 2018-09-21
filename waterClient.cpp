#include "waterClient.h"

enum class RequestType : byte
{
	LOGIN_BY_USER = 1,
	LOGIN_BY_RFID = 2
};

union WaterClient::RequestImpl
{
	LoginByUserRequest loginByUser;
	LoginByRfidRequest loginByRfid;
};

struct WaterClient::Request
{
	RequestSeqNum requestSeqNumAtBegin;
	RequestType requestType;
	RequestImpl impl;
	Credit consumeCredit;
	RequestSeqNum requestSeqNumAtEnd;
};

struct Reply
{
	WaterClient::RequestSeqNum replySeqNumAtBegin;
	WaterClient::LoginReply impl;
	WaterClient::RequestSeqNum replySeqNumAtEnd;
};

#define REQUEST_ADDRESS 101
#define REPLY_ADDRESS 102

WaterClient::WaterClient(uint8_t const controlPinNumber, HardwareSerial* hwSerial, uint8_t slaveNum, uint32_t connectTimeoutSec) :
	rtu(slaveNum, hwSerial, controlPinNumber),
	nextRequestId(1),
	timeoutSec(connectTimeoutSec)
{
	// adding areas
	this->rtu.addWordArea(REQUEST_ADDRESS, reinterpret_cast<u16*>(this->sendBuffer), SEND_BUFFER_SIZE_BYTES/2);
	this->rtu.addWordArea(REPLY_ADDRESS, reinterpret_cast<u16*>(this->receiveBuffer), RECEIVE_BUFFER_SIZE_BYTES/2);

	this->rtu.begin(9600);

#ifdef WATER_DEBUG
	Serial.begin(9600);
#endif
}

WaterClient::LoginReply
WaterClient::loginByUser(UserId const userId, Pin const pin)
{
	RequestImpl impl;
	impl.loginByUser.userId = userId;
	impl.loginByUser.pin = pin;

	return this->loginImpl(RequestType::LOGIN_BY_USER, impl, 0);
}

WaterClient::LoginReply
WaterClient::loginByRfid(RfidId const rfid)
{
	RequestImpl impl;
	impl.loginByRfid.rfidId = rfid;

	return this->loginImpl(RequestType::LOGIN_BY_RFID, impl, 0);

}

void
WaterClient::logout(Credit const creditConsumed)
{
	if (this->lastLoginByRfid.hasSome())
	{
		if (this->lastLoginByUser.hasSome())
		{
			LOG("BUG: logged in both ways - by RFID and by PIN");
			return;
		}
		LOG("logout by rfid");
		LOG(creditConsumed);

		RequestImpl impl;
		impl.loginByRfid = this->lastLoginByRfid.getValue();
		this->loginImpl(RequestType::LOGIN_BY_RFID, impl, creditConsumed);

	}
	else if (this->lastLoginByUser.hasSome())
	{
		LOG("logout by userId and pin");
		LOG(this->lastLoginByUser.getValue().userId);
		LOG(creditConsumed);

		RequestImpl impl;
		impl.loginByUser = this->lastLoginByUser.getValue();
		this->loginImpl(RequestType::LOGIN_BY_USER, impl, creditConsumed);
	}
	else
	{
		LOG("user not logged, failed to logout");
	}
}

template <typename RequestTypeT, typename RequestImplT>
WaterClient::LoginReply
WaterClient::loginImpl(RequestTypeT const requestType, RequestImplT const & requestImpl, WaterClient::Credit const creditToConsume)
{
	LOG("login started");

	Request rq{
		this->nextRequestId,
		requestType,
		requestImpl,
		creditToConsume,
		this->nextRequestId,
	};

	RequestSeqNum const seqNumExpectedInReply = this->nextRequestId;
	++ this->nextRequestId;

	memcpy(this->sendBuffer, &rq, sizeof(rq));

	uint32_t attemptsLeft = this->timeoutSec;

	while (--attemptsLeft > 0)
	{
		this->rtu.process();
		Reply* reply = reinterpret_cast<Reply*>(this->receiveBuffer);
		if (reply->replySeqNumAtBegin == seqNumExpectedInReply && reply->replySeqNumAtEnd == seqNumExpectedInReply)
		{
			LOG("reply received");
			LOG(static_cast<int>(reply->impl.status));
			if (reply->impl.creditAvail.hasSome()) { LOG(reply->impl.creditAvail.getValue()); }

			return reply->impl;
		}
	}

	LOG("waiting for reply timed out");
	return LoginReply{LoginReply::Status::TIMEOUT, Option<Credit>()};

}


void
WaterClient::keepCommunicationAlive()
{
	this->rtu.process();
}

Option<WaterClient::RfidId>
WaterClient::CreateRfidIdFromStringInHex(String const &)
{
	return Option<WaterClient::RfidId>();
}


