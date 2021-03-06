#include "waterSharedTypes.h"

namespace water
{

WaterClient::WaterClient(uint8_t const controlPinNumber, HardwareSerial* hwSerial, uint8_t slaveNum, uint32_t connectTimeoutSec) :
	rtu(slaveNum, hwSerial, controlPinNumber),
	nextRequestId(1),
	timeoutSec(connectTimeoutSec)
{
	// adding areas
	this->rtu.addWordArea(REQUEST_ADDRESS, reinterpret_cast<u16*>(this->sendBuffer), SEND_BUFFER_SIZE_BYTES/2);
	this->rtu.addWordArea(REPLY_ADDRESS, reinterpret_cast<u16*>(this->receiveBuffer), SEND_BUFFER_SIZE_BYTES/2);

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

	auto rpl = this->loginImpl(RequestType::LOGIN_BY_USER, impl, 0);
	if (rpl.status == WaterClient::LoginReply::Status::SUCCESS) this->lastLoginByUser = impl.loginByUser;
	return rpl;
}

WaterClient::LoginReply
WaterClient::loginByRfid(RfidId const rfid)
{
	RequestImpl impl;
	impl.loginByRfid.rfidId = rfid;

	auto rpl = this->loginImpl(RequestType::LOGIN_BY_RFID, impl, 0);
	if (rpl.status == WaterClient::LoginReply::Status::SUCCESS) this->lastLoginByRfid = impl.loginByRfid;
	return rpl;
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
		this->lastLoginByRfid = Option<LoginByRfidRequest>();
	}
	else if (this->lastLoginByUser.hasSome())
	{
		LOG("logout by userId and pin");
		LOG(this->lastLoginByUser.getValue().userId);
		LOG(creditConsumed);

		RequestImpl impl;
		impl.loginByUser = this->lastLoginByUser.getValue();
		this->loginImpl(RequestType::LOGIN_BY_USER, impl, creditConsumed);
		this->lastLoginByUser = Option<LoginByUserRequest>();
	}
	else
	{
		LOG("user not logged, failed to logout");
	}
}

struct BufferWriter
{
	template <class T>
	static void readWriteRequest(T inMemory, T & inBuffer) { inBuffer = inMemory; }
	template <class T>
	static void readWriteReply(T & inMemory, T inBuffer) { inMemory = inBuffer; }
};

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

	bool const writeResult = serializeRequest<BufferWriter>(rq, static_cast<char*>(this->sendBuffer));

	unsigned long lastrecv = millis();

	while (true)
	{
		this->rtu.process();
		Reply reply;
		serializeReply<BufferWriter>(reply, static_cast<char*>(this->receiveBuffer));

		if (reply.replySeqNumAtBegin == seqNumExpectedInReply && reply.replySeqNumAtEnd == seqNumExpectedInReply)
		{
			LOG("reply received");
			LOG(static_cast<int>(reply.impl.status));
			LOG(reply.impl.creditAvail);
			return reply.impl;
		}

		unsigned long currentMillis = millis();
		if ((currentMillis > this->timeoutSec * 1000 + lastrecv) || currentMillis < lastrecv) break;
	}

	LOG("waiting for reply timed out");
	return LoginReply{LoginReply::Status::TIMEOUT, 0};

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

} // ns water
