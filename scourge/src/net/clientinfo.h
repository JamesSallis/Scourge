#pragma once
#ifdef HAVE_SDL_NET

#ifndef CLIENT_INFO_H
#define CLIENT_INFO_H

#include "../persist.h"
#include "server.h"
#include "tcputil.h"
#include "commands.h"

int clientInfoLoop( void *data );

class Server;

/* does nothing that std::vector<char> does not
class Message {
 public:
  char *message;
  int length;
  Message(char *message, int length);
  ~Message();
};*/

typedef std::vector<char> Message;

class ClientInfo : public CommandInterpreter {
public:
	enum {
		RECEIVE = 0,
		SEND,
		DESC_SIZE = 80
	};

	Server *server;
	bool dead;
	TCPsocket socket;
	int id;
	std::string username;
	char desc[ DESC_SIZE ];
	bool threadRunning;
	SDL_Thread *thread;
	SDL_mutex *mutex;
	std::queue<Message*> messageQueue;
	std::map<int, Uint32> lagMap;
	Commands *commands;
	CreatureInfo* characterInfo;

	Uint32 totalLag, lastLagCheck;
	float aveLag;

	// if this many messages are not acknowledged, the client is disconnected
	static const int MAX_SCHEDULED_LAG_MESSAGES = 25;

	ClientInfo( Server *server, TCPsocket socket, int id, char const* username );
	virtual ~ClientInfo();
	inline int getId() {
		return id;
	}
	inline void setId( int id ) {
		this->id = id;
	}
	inline char const* getUsername() {
		return username.c_str();
	}
	char *describe();
	void sendMessageAsync( char *s, int length = 0 );
	void setLagTimer( int frame, Uint32 n );
	Uint32 updateLag( int frame );

	// commandInterpreter
	void chat( char *message );
	void logout();
	void ping( int frame );
	void handleUnknownMessage();
	void processGameState( int frame, char *p );
	void serverClosing();
	void character( char *bytes, int length );
	void addPlayer( Uint32 id, char *bytes, int length );

	// protected
	void receiveTCP();
	void sendTCP( char *message, int length );
	inline SDL_mutex *getMutex() {
		return mutex;
	}
	inline TCPsocket getSocket() {
		return socket;
	}
	inline bool isThreadRunning() {
		return threadRunning && !dead;
	}
	inline std::queue<Message*> *getMessageQueue() {
		return &messageQueue;
	}
	inline Commands *getCommands() {
		return commands;
	}
};

#endif
#endif
