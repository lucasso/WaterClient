#include "waterClient.h"

enum class RequestType : byte
{
	LOGIN_BY_USER = 1,
	LOGIN_BY_RFID,
	LOGOUT
};

struct LoginByUserRequest
{
	WaterClient::UserId userId;
	WaterClient::Pin pin;
};

struct LoginByRfidRequest
{
	WaterClient::RfidId rfidId;
};

struct LogoutRequest
{
	WaterClient::RequestSeqNum loginSeqNum;
};

union RequestImpl
{
	LoginByUserRequest loginByUser;
	LoginByRfidRequest loginByRfid;
	LogoutRequest logout;
};

struct Request
{
	WaterClient::RequestSeqNum requestSeqNumAtBegin;
	RequestType requestType;
	RequestImpl impl;
	WaterClient::RequestSeqNum requestSeqNumAtEnd;
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
	nextRequestId(1)
{
	this->rtu.addWordArea(REQUEST_ADDRESS, static_cast<void*>(this->sendBuffer), SEND_BUFFER_SIZE_BYTES/2);
	this->rtu.addWordArea(REPLY_ADDRESS, static_cast<void*>(this->receiveBuffer), RECEIVE_BUFFER_SIZE_BYTES/2);
	// add here areas
	this->rtu.begin(9600);
}

WaterClient::LoginReply
WaterClient::loginByUser(UserId const userId, Pin const pin)
{
	RequestImpl impl;
	impl.loginByUser.userId = userId;
	impl.loginByUser.pin = pin;
	Request rq{
		this->nextRequestId,
		RequestType::LOGIN_BY_USER,
		impl,
		this->nextRequestId,
	};

	RequestSeqNum const seqNumExpectedInReply = this->nextRequestId;
	++ this->nextRequestId;

	memcpy(this->sendBuffer, &rq, sizeof(rq));

	return LoginReply{LoginReply::Status::INVALID_ID, Option<Credit>()};

}

WaterClient::LoginReply
WaterClient::loginByRfid(RfidId)
{
	return LoginReply{LoginReply::Status::INVALID_ID, Option<Credit>()};
}

void
WaterClient::logout(Credit ) // creditConsumed
{

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


