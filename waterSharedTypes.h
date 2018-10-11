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
	FunT::readWriteRequest(rq.requestSeqNumAtBegin, reinterpret_cast<WaterClient::RequestSeqNum*>(buffer)[0]);
	offset += sizeof(WaterClient::RequestSeqNum);
	FunT::readWriteRequest(rq.requestType, reinterpret_cast<water::RequestType*>(buffer+offset)[0]);
	offset += sizeof(water::RequestType);
	switch (rq.requestType)
	{
	case water::RequestType::LOGIN_BY_USER:
		FunT::readWriteRequest(rq.impl.loginByUser.userId, reinterpret_cast<WaterClient::UserId*>(buffer+offset)[0]);
		offset += sizeof(WaterClient::UserId);
		FunT::readWriteRequest(rq.impl.loginByUser.pin, reinterpret_cast<WaterClient::Pin*>(buffer+offset)[0]);
		offset += sizeof(WaterClient::Pin);
		break;
	case water::RequestType::LOGIN_BY_RFID:
		FunT::readWriteRequest(rq.impl.loginByRfid.rfidId, reinterpret_cast<WaterClient::RfidId*>(buffer+offset)[0]);
		offset += sizeof(WaterClient::RfidId);
		break;
	default:
		return false;
	}
	FunT::readWriteRequest(rq.consumeCredit, reinterpret_cast<WaterClient::Credit*>(buffer+offset)[0]);
	offset += sizeof(WaterClient::Credit);
	FunT::readWriteRequest(rq.requestSeqNumAtEnd, reinterpret_cast<WaterClient::RequestSeqNum*>(buffer+offset)[0]);
	return true;
}

struct Reply
{
	WaterClient::RequestSeqNum replySeqNumAtBegin;
	WaterClient::LoginReply impl;
	WaterClient::RequestSeqNum replySeqNumAtEnd;
};

template <class FunT> void serializeReply(Reply & rpl, char * buffer)
{
	uint32_t offset = 0;
	FunT::readWriteReply(rpl.replySeqNumAtBegin, reinterpret_cast<WaterClient::RequestSeqNum*>(buffer)[0]);
	offset += sizeof(WaterClient::RequestSeqNum);
	FunT::readWriteReply(rpl.impl.status, reinterpret_cast<WaterClient::LoginReply::Status*>(buffer+offset)[0]);
	offset += sizeof(WaterClient::LoginReply::Status);
	FunT::readWriteReply(rpl.impl.creditAvail, reinterpret_cast<WaterClient::Credit*>(buffer+offset)[0]);
	offset += sizeof(WaterClient::Credit);
	FunT::readWriteReply(rpl.replySeqNumAtEnd, reinterpret_cast<WaterClient::RequestSeqNum*>(buffer)[0]);
}

#define REQUEST_ADDRESS 51
#define REPLY_ADDRESS 52

}

#endif /* LIBRARIES_WATERCLIENT_WATERSHAREDTYPES_H_ */
