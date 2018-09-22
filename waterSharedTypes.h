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
