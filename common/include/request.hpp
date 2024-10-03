// well, I'll need to refactor the client and server requests to use this enum
enum RequstType {
	REGISTER,
	AUTHORIZE,
	SEND_MESSAGE,
	GET_CHATS,
	GET_MESSAGE,
	GET_MESSAGES_OFFLINE // rename
};