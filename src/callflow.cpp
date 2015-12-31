/*
 * CallFlow.cpp
 *
 * Tool for creating a callflow from an ethereal trace which is printed 
 * to file
 *
 * Copyright (c) 2001 Philips Electronics N.V.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is CallFlow
 *
 * The Initial Developer of the Original Code is Philips Electronics N.V.
 *
 */

#include <ptlib.h>

#include "CallFlow.h"

PCREATE_PROCESS(CallFlow)

PString getArgumentsParseString()
{
	return PString( 
    "f-filename:"
    "g-gatekeeper:"
    "o-output:"
    );
}

const PString CR_LF = "\r\n";

PString InttoStr( int in ) 
{
  if (in == 0 ) return PString( "0" );
  
  PString result;
  char lv_s[255];
  sprintf(lv_s, "%d", in);
  result = lv_s;
  
  return result;
}

void CallFlow::Main()
{

  PArgList & commandLine = GetArguments();
	commandLine.Parse( getArgumentsParseString() );

  PString lv_filename;
  PString lv_gatekeeper;
  PString lv_outputFileName = "output.txt";

  bool lv_filenamefound = false;
  bool lv_gatekeeperfound = false;
  
  cout << "CallFlow 0.02, (c) 2001,2002 Royal Philips Electronics NV"<< endl;
  
  if ( commandLine.HasOption( "filename" ) )
  {
    lv_filename = commandLine.GetOptionString( "filename" );
    lv_filenamefound = true;
  }
  
  if ( commandLine.HasOption( "gatekeeper" ) )
  {
    lv_gatekeeper = commandLine.GetOptionString( "gatekeeper" );
    lv_gatekeeperfound = true;
  }

  if ( commandLine.HasOption( "output" ) )
  {
    lv_outputFileName = commandLine.GetOptionString( "output" );
  }

  if ( ( lv_gatekeeperfound == true ) && ( lv_gatekeeperfound == true ) )
  {
    if ( ( lv_filename != "" ) && ( lv_gatekeeper != "" ) )
    {

      cout << "Input file  : " << lv_filename << endl;
      cout << "Output file : " << lv_outputFileName << endl;
      cout << "Gatekeeper  : " << lv_gatekeeper << endl;

      // Add the gatekeeper to the list
      messageList.addGateKeeper( lv_gatekeeper );

      PTextFile lv_input;
      PTextFile lv_output;
      
      lv_input.Open( lv_filename, PFile::ReadOnly );
      
      if ( lv_input.Open() )
      {
        cout << "Reading input file" << endl;
        bool lv_result = true;
        
        while ( lv_result == true )
        {
          lv_result = ReadFrame( lv_input );
          if ( lv_result == true )
          {
            lv_result = ReadIPv4( lv_input );
            if ( lv_result == true )
            {
              lv_result = ReadH323( lv_input );
              if ( lv_result == true )
              {
                // Add the message to the list
                messageList.addMessage( currentCaption, currentSource, currentDestination, currentTime, currentFrame );
              }
            }
          }
        }

        if ( lv_output.Open( lv_outputFileName, PFile::WriteOnly, PFile::Create | PFile::Exclusive ) )
        {
          cout << "Writing output file" << endl;
          messageList.dump( lv_output );
        }
        else
        {
          cout << "Error writing to output file (file already exists?)" << endl;
        }
      } 
      else
      {
        cout << "Could not open file " << lv_filename << endl;
      }
    }
    else
    {
      cout << "No filename supplied" << endl;
    }
  }
  else
  {
    cout << "To generate a H.323 call flow from an Ethereal capture which has "<< endl;
    cout << "been printed to file" << endl << endl;
    cout << "Usage: " << endl;
    cout << "callflow -f <filename> -g <gatekeeper> [-o <outputname>]" << endl;
    cout << "  where:" << endl;
    cout << "  filename       The filename of the Ethereal print file [REQUIRED]" << endl;
    cout << "  gatekeeper     The IPv4 address of the gatekeeper      [REQUIRED]" << endl;
    cout << "  outputname     The name of the output file             [OPTIONAL]" << endl;
    cout << "                 This output file should not be an existing file" << endl;
    cout << "The gatekeeper will always be the second column in the callflow." << endl;
    cout << "The first endpoint will be to the left of the gatekeeper, all " << endl;
    cout << "other endpoints will be printed to the right of the gatekeeper " << endl;
    cout << "column" << endl;

  }
}

PString findIPAddressInSourceOrDestination( PString &fi_str )
{
  //"Source: 192.168.11.253 (192.168.11.253)
  int lv_start = fi_str.Find( " ", 0 ) + 1;
  int lv_stop = fi_str.Find( " ", lv_start );

  PString lv_result = fi_str.Mid( lv_start, fi_str.GetSize() - lv_stop - 3 );

  return lv_result;
}

PString getReasonableTime( PString &fi_str )
{
  int lv_start = fi_str.Find( ":", 0 ) + 2;
  int lv_stop = fi_str.Find( " seconds", 0 );

  PString lv_result = fi_str.Mid( lv_start, fi_str.GetSize() - lv_stop );

  if ( lv_result.GetSize() >= 6 )
  {
    // Trim to 6 characters
    lv_result = lv_result.Mid( 0, 6 );
  }

  return lv_result;

}

PString getFrameNumber( PString &fi_str )
{
  int lv_start = fi_str.Find( ":", 0 ) + 2;
  int lv_stop = fi_str.Find( "\n", 0 );

  PString lv_result = fi_str.Mid( lv_start, fi_str.GetSize() - lv_stop );

  return lv_result;

}

bool CallFlow::ReadFrame( PTextFile &input )
{
  bool lv_result = false;

  PString lv_line;
  if ( input.ReadLine( lv_line ) == TRUE )
  {
    
    // First find Frame header
    while ( lv_line.Find( "on wire" ) > lv_line.GetSize() )
    {
      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
    }
    
    // Find and read source and destination IP addresses

    // Find Source line
    while ( lv_line.Find( "Time relative to first packet" ) > lv_line.GetSize() )
    {
      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
    }
    lv_line = lv_line.LeftTrim();
    //lv_line = lv_line.RightTrim();

    currentTime = getReasonableTime( lv_line );

    // Find the framenumber
    while ( lv_line.Find( "Frame Number" ) > lv_line.GetSize() )
    {
      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
    }
    lv_line = lv_line.LeftTrim();
    //lv_line = lv_line.RightTrim();

    currentFrame = getFrameNumber( lv_line );

    lv_result = true;
  }
  // Read timestamp info from the Frame and add it to the current Message
  return lv_result;
}

bool CallFlow::ReadIPv4( PTextFile &input )
{
  bool lv_result = false;

  PString lv_line;
  if ( input.ReadLine( lv_line ) == TRUE )
  {
    
    // First find IP header
    while ( lv_line.Find( "Internet Protocol" ) > lv_line.GetSize() )
    {
      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
    }
    
    // Find and read source and destination IP addresses

    // Find Source line
    while ( lv_line.Find( "Source: " ) > lv_line.GetSize() )
    {
      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
    }
    lv_line = lv_line.LeftTrim();
    lv_line = lv_line.RightTrim();

    currentSource = findIPAddressInSourceOrDestination( lv_line );

    // Find Destination line
    while ( lv_line.Find( "Destination: " ) > lv_line.GetSize() )
    {
      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
    }
    lv_line = lv_line.LeftTrim();
    lv_line = lv_line.RightTrim();

    currentDestination = findIPAddressInSourceOrDestination( lv_line );
    lv_result = true;
  }

  return lv_result;
}

bool CallFlow::ReadH323( PTextFile &input )
{
  bool lv_result = false;
  // Find and read H323 message and call either ReadRAS, ReadCs or Read H245 to do the real work
  // Find a line with the contents "ITU-T Recommendation"
  PString lv_line;

  bool lv_isCS = false;

  if ( input.ReadLine( lv_line ) == TRUE )
  {
    // Grr, never make typing errors and release that application to the outside world ;-)
    while ( ( lv_line.Find( "ITU-T Recommendation" ) > lv_line.GetSize() ) && ( lv_line.Find( "ITU-T Reccomendation" ) > lv_line.GetSize() ) )
    {
      // If the message contains Q931, then it's a CS and not RAS, so we also look for that
      
      if ( ( lv_line.Find( "TPKT" ) < lv_line.GetSize() ) || ( lv_line.Find( "Q.931" ) < lv_line.GetSize() ) )
      {
        lv_isCS = true;
      }

      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
    }

    // Found ITU-T line, now check if it's H.225 or H245

    if ( lv_line.Find( "H.225" ) < lv_line.GetSize() )
    {
      if ( lv_isCS == true )
        lv_result = ReadCS( input );
      else
        lv_result = ReadRAS( input );
    }
    else
    {
      lv_result = ReadH245( input );
    }

    // Find out whcih protocol it is
  }
  return lv_result;
}

bool CallFlow::ReadRAS( PTextFile &input )
{
  bool lv_result = false;

  PString lv_line;

  // Current line is ITU-T line, next one is message type
  if ( input.ReadLine( lv_line ) == FALSE )
  {
    return false;
  }

  lv_line = lv_line.LeftTrim();
  //lv_line = lv_line.RightTrim();

  currentCaption = lv_line;

  lv_result = true;

  return lv_result;
}

bool CallFlow::ReadCS( PTextFile &input )
{
  bool lv_result = false;

  PString lv_line;
  
  // Current line is ITU-T line, third one is message type
  if ( input.ReadLine( lv_line ) == FALSE )
  {
    return false;
  }
  if ( input.ReadLine( lv_line ) == FALSE )
  {
    return false;
  }
  if ( input.ReadLine( lv_line ) == FALSE )
  {
    return false;
  }

  lv_line = lv_line.LeftTrim();
  //lv_line = lv_line.RightTrim();

  currentCaption = lv_line;

  lv_result = true;

  return lv_result;
}

bool CallFlow::ReadH245( PTextFile &input )
{
  bool lv_result = false;

  PString lv_line;

  // Current line is ITU-T line, second one is message type
  if ( input.ReadLine( lv_line ) == FALSE )
  {
    return false;
  }
  if ( input.ReadLine( lv_line ) == FALSE )
  {
    return false;
  }

  lv_line = lv_line.LeftTrim();
  //lv_line = lv_line.RightTrim();

  currentCaption = lv_line;

  lv_result = true;

  while ( lv_line != "" )
  {
    while ( ( lv_line.Find( "ITU-T Recommendation" ) > lv_line.GetSize() ) && ( lv_line.Find( "ITU-T Reccomendation" ) > lv_line.GetSize() ) )
    {
      
      if ( lv_line == "" )
      {
        break;
      }
      
      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
    }
    if ( lv_line != "" )
    {
      // Current line is ITU-T line, second one is message type
      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
      if ( input.ReadLine( lv_line ) == FALSE )
      {
        return false;
      }
      
      lv_line = lv_line.LeftTrim();
      //lv_line = lv_line.RightTrim();
      
      currentCaption += PString( " " ) + lv_line;
      
      lv_result = true;
      
    }
  }
  
  return lv_result;
}

bool CallFlow::ReadTPKT( PTextFile &input )
{
  bool lv_result = false;
  return lv_result;
}