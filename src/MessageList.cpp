
#include "MessageList.h"


MessageList::MessageList()
{
}

MessageList::~MessageList()
{
  if ( endpointList.empty() != true )
  {
    std::vector<Endpoint*>::iterator iter = endpointList.begin();
    while ( endpointList.empty() != true )
    {
      // remove memory pointed by iter
      delete (*iter);
      // remove entry from list
      endpointList.erase( iter );
      iter = endpointList.begin();
    }
  }

  if ( messageList.empty() != true )
  {
    std::vector<Message*>::iterator iter = messageList.begin();
    while ( messageList.empty() != true )
    {
      // remove memory pointed by iter
      delete (*iter);
      // remove entry from list
      messageList.erase( iter );
      iter = messageList.begin();
    }
  }
}

void MessageList::addMessage( PString caption, PString source, PString destination, PString time, PString number )
{
  // Find the nr of the source
  int lv_nrSrc = findEndpoint( source );
  // Find the nr of the destination
  int lv_nrDest = findEndpoint( destination );
  // Add the message to the list
  messageList.push_back( new Message( caption, lv_nrSrc, lv_nrDest, time, number ) );
}

void MessageList::addGateKeeper( PString ipv4Address )
{
  //Add the gatekeeper to list
  Endpoint* lv_ep = new MessageList::Endpoint( 0, ipv4Address );
  endpointList.push_back( lv_ep );
}

int MessageList::findEndpoint( PString addr )
{
  int lv_nr = 0;

  if ( endpointList.empty() != true )
  {
    std::vector<Endpoint*>::iterator iter = endpointList.begin();
    while ( iter != endpointList.end() )
    {
      Endpoint* lv_ep = (*iter);
      if ( lv_ep->getAddress() == addr )
      {
        lv_nr = lv_ep->getNumber();
        break;
      }

      lv_nr++;
      iter++;
    }

    if ( iter == endpointList.end() )
    {
      // No endpoint found, add it.
      Endpoint* lv_ep = new MessageList::Endpoint( lv_nr, addr );
      endpointList.push_back( lv_ep );
    }
  }

  return lv_nr;
}

PString CreateArrowAndCaption( int src_column, int dest_column, PString caption )
{
  PString lv_result;
  bool lv_arrowToRight = true;

  int lv_left_margin = 0;

  int lv_nrColumns = 0;

  if ( src_column > dest_column )
  {
    lv_arrowToRight = false;
    lv_nrColumns = src_column - dest_column;
    lv_left_margin = dest_column;
  }
  else
  {
    lv_nrColumns = dest_column - src_column ;
    lv_left_margin = src_column;
  }

  // Every column is 20 characters wide
  PString lv_arrow;
  for( int i=0; i < lv_nrColumns; i++ )
  {
    lv_arrow += "------------------";
  }

  if ( lv_arrowToRight == false )
  {
    lv_arrow =  PString( "<" ) + lv_arrow;
  }
  else
  {
    lv_arrow +=  PString( ">" );
  }

  // Find the middle of the arrow, then deduct half the caption 
  // length from that. This is the length of the spaces before the caption
  // from column to column
  
  int lv_spaces_length = lv_arrow.GetSize() / 2;
  lv_spaces_length -= caption.GetSize() / 2;

  if ( lv_spaces_length < 0 ) 
    lv_spaces_length = 0;

  PString lv_caption;
  for( i=0; i < lv_spaces_length; i++ )
  {
    lv_caption += " ";
  }

  lv_caption += caption;

  // Add the left margins to the caption and the arrow

  // Add left margin times 20 spaces as margin to both strigns and add 10 
  // spaces to "center" the arrow
  for( i=0; i < lv_left_margin; i++ )
  {
    lv_caption = PString( "                   " ) + lv_caption;
    lv_arrow = PString( "                   " )  + lv_arrow;
  }
  // Add the 10 spaces at the beginning and 18 for the time and frame before the arrow
  lv_caption = PString( "    " ) + lv_caption;
  lv_arrow = PString( "               " ) + lv_arrow;

  lv_result = lv_caption + "\n"  + lv_arrow + "\n" ;

  return lv_result;
}

void MessageList::dump( PTextFile &lv_output )
{
  // Write all the endpoints to file as a heading, swapping gatekeeper and first endpoint
  {
    PString lv_str= "Frame Time                         GK\n          ";

    // If there's a gatekeeper and an  endpoint, swap them
    std::vector<Endpoint*>::iterator iter = endpointList.begin();
    if ( endpointList.size() >= 2 )
    {
      lv_str += endpointList[1]->getAddress() + "     ";
      lv_str += endpointList[0]->getAddress() + "     ";
      iter++;
      iter++;
    }

    while ( iter != endpointList.end() )
    {
      Endpoint* lv_ep = (*iter);
      lv_str += lv_ep->getAddress() + "     ";
      iter++;
    }
    lv_str +="\r\n";
    lv_output.WriteLine( lv_str );
  }
  // Write all the messages from one endpoint to the gateway
  {
    PString lv_str;
    std::vector<Message*>::iterator iter = messageList.begin();
    while ( iter != messageList.end() )
    {
      Message* lv_ep = (*iter);

      int lv_src = lv_ep->getSource();
      int lv_dest = lv_ep->getDestination();

      // Swap columns if the source or destination is gatekeeper or first endpoint
      if ( lv_src == 0 )
      {
        // Gatekeeper, move to second column
        lv_src = 1;
      }
      else
      {
        if ( lv_src == 1 )
        {
          lv_src = 0;
        }
      }

      if ( lv_dest == 0 )
      {
        // Gatekeeper, move to second column
        lv_dest = 1;
      }
      else
      {
        if ( lv_dest == 1 )
        {
          lv_dest = 0;
        }
      }

      PString time = lv_ep->getTime();
      while( time.GetSize() < 6 )
      {
        time += " ";
      }

      PString frame = lv_ep->getFrameNumber();
      while( frame.GetSize() < 5 )
      {
        frame += " ";
      }

      lv_output.WriteLine( frame + " " + time + CreateArrowAndCaption( lv_src, lv_dest, lv_ep->getCaption() ) );

      iter++;
    }
    lv_str +="\r\n";
    lv_output.WriteLine( lv_str );
    lv_str = "";
  }

}


MessageList::Message::Message() : source( -1 ), destination( -1 )
{
}

MessageList::Message::Message( PString cap, int src, int dest, PString time, PString number ) : caption( cap ), source( src ), destination( dest ), frameTime( time ), frameNumber( number )
{ 
}

MessageList::Message::~Message()
{
}

MessageList::Endpoint::Endpoint() : number( -1 )
{
}

MessageList::Endpoint::Endpoint( int nr, PString addr ) : number( nr ), iPv4( addr )
{
}

MessageList::Endpoint::~Endpoint()
{
}

PString MessageList::Endpoint::getAddress( void )
{
  return iPv4;
}

int MessageList::Endpoint::getNumber( void )
{
  return number;
}
