#pragma once

namespace common {

enum RequstType {
	REGISTER,
	AUTHORIZE,
	SEND_MESSAGE,
	GET_CHATS,
	GET_MESSAGE,
	GET_MESSAGES_OFFLINE // rename
};

} //namespace common