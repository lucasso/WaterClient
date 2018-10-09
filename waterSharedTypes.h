#ifndef LIBRARIES_WATERCLIENT_WATERSHAREDTYPES_H_
#define LIBRARIES_WATERCLIENT_WATERSHAREDTYPES_H_

#include "waterClient.h"

namespace water
{

enum class RequestType : uint8_t
{
	LOGIN_BY_USER = 11,
	LOGIN_BY_RFID = 12
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

template <class FunT> bool serializeRequest(WaterClient::Request & rq, char * buffer)
{
	uint32_t offset = 0;
	FunT::readWrite(rq.requestSeqNumAtBegin, reinterpret_cast<WaterClient::RequestSeqNum*>(buffer)[0]);
	offset += sizeof(WaterClient::RequestSeqNum);
	FunT::readWrite(rq.requestType, reinterpret_cast<water::RequestType*>(buffer+offset)[0]);
	offset += sizeof(water::RequestType);
	switch (rq.requestType)
	{
	case water::RequestType::LOGIN_BY_USER:
		FunT::readWrite(rq.impl.loginByUser.userId, reinterpret_cast<WaterClient::UserId*>(buffer+offset)[0]);
		offset += sizeof(WaterClient::UserId);
		FunT::readWrite(rq.impl.loginByUser.pin, reinterpret_cast<WaterClient::Pin*>(buffer+offset)[0]);
		offset += sizeof(WaterClient::Pin);
		break;
	case water::RequestType::LOGIN_BY_RFID:
		FunT::readWrite(rq.impl.loginByRfid.rfidId, reinterpret_cast<WaterClient::RfidId*>(buffer+offset)[0]);
		offset += sizeof(WaterClient::RfidId);
		break;
	default:
		return false;
	}
	FunT::readWrite(rq.consumeCredit, reinterpret_cast<WaterClient::Credit*>(buffer+offset)[0]);
	offset += sizeof(WaterClient::Credit);
	FunT::readWrite(rq.requestSeqNumAtEnd, reinterpret_cast<WaterClient::RequestSeqNum*>(buffer+offset)[0]);
	return true;
}

struct Reply
{
	WaterClient::RequestSeqNum replySeqNumAtBegin;
	WaterClient::LoginReply impl;
	WaterClient::RequestSeqNum replySeqNumAtEnd;
};

#define REQUEST_ADDRESS 51
#define REPLY_ADDRESS 52

}

#endif /* LIBRARIES_WATERCLIENT_WATERSHAREDTYPES_H_ */
