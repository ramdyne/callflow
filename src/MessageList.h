#ifndef MESSAGELIST_H
#define MESSAGELIST_H

#include <ptlib.h>
#include <vector>

class MessageList
{

public:
  
  MessageList();
  ~MessageList();

  void addMessage( PString caption, PString source, PString destination, PString time, PString number );
  void addGateKeeper( PString ipv4Address );
  
  void dump( PTextFile &lv_output );

private:

  class Message
  {
  public:
    Message();
    Message( PString cap, int src, int dest, PString time, PString number );
    ~Message();

    int getSource( void ) { return source; };
    int getDestination( void ) { return destination; };
    PString getCaption( void ) { return caption; };
    PString getTime( void ) { return frameTime; };
    PString getFrameNumber( void ) { return frameNumber; };
  private:
    PString caption;
    PString frameTime;
    PString frameNumber;
    int source;
    int destination;
  };

  class Endpoint
  {
  public:
    Endpoint();
    Endpoint( int nr, PString addr );
    ~Endpoint();

    PString getAddress( void );
    int     getNumber ( void );

  private:
    int number;
    PString iPv4;
  };

  int findEndpoint( PString addr );

  std::vector<Message*> messageList;
  std::vector<Endpoint*> endpointList;
};


#endif //MESSAGELIST_H