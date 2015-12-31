/*
 * CallFlow.h
 *
 * Tool for converting a Windows Icon file to a format 
 * useable by the PWLib application "pwrc"
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
 * The Initial Developer of the Original Code is Phlilips Electronics N.V.
 *
 */

#ifndef CallFlow_H
#define CallFlow_H

#include <ptlib.h>

#include "MessageList.h"

// Compile as a normal application with a console window
class CallFlow : public PProcess
{
  PCLASSINFO(CallFlow, PProcess)
public:
  // Resume normal class definition
  void Main();

private:
  bool ReadFrame( PTextFile &input );
  bool ReadIPv4( PTextFile &input );
  bool ReadH323( PTextFile &input );
  bool ReadRAS( PTextFile &input );
  bool ReadCS( PTextFile &input );
  bool ReadH245( PTextFile &input );
  bool ReadTPKT( PTextFile &input );

  PString currentSource;
  PString currentDestination;
  PString currentCaption;
  PString currentTime;
  PString currentFrame;

  MessageList messageList;
};

#endif
