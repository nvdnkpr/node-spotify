/**
The MIT License (MIT)

Copyright (c) <2013> <Moritz Schulze>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
**/

#include "NodePlayer.h"
#include "NodeTrack.h"

#include "../../events.h"
#include "../../Application.h"

extern "C" {
  #include "../../audio/audio.h"
}

extern Application* application;

/* REMOVE ME */
namespace spotify {
extern int framesReceived;
extern int currentSecond;
}

Handle<Value> NodePlayer::pause(const Arguments& args) {
  HandleScope scope;
  NodePlayer* nodePlayer = node::ObjectWrap::Unwrap<NodePlayer>(args.This());
  sp_session_player_play(application->session, 0);
  audio_fifo_flush(&application->audio_fifo);
  nodePlayer->isPaused = true;
  return scope.Close(Undefined());
}

Handle<Value> NodePlayer::stop(const Arguments& args) {
  HandleScope scope;
  sp_session_player_unload(application->session);
  return scope.Close(Undefined());
}

Handle<Value> NodePlayer::resume(const Arguments& args) {
  HandleScope scope;
  NodePlayer* nodePlayer = node::ObjectWrap::Unwrap<NodePlayer>(args.This());
  if(nodePlayer->isPaused) {
    sp_session_player_play(application->session, 1);
    nodePlayer->isPaused = false;
  }
  return scope.Close(Undefined());
}

Handle<Value> NodePlayer::play(const Arguments& args) {
  HandleScope scope;
  spotify::framesReceived = 0;
  spotify::currentSecond = 0;
  NodeTrack* nodeTrack = node::ObjectWrap::Unwrap<NodeTrack>(args[0]->ToObject());
  sp_session_player_load(application->session, nodeTrack->track->track);
  sp_session_player_play(application->session, 1);
  return scope.Close(Undefined());
}

Handle<Value> NodePlayer::seek(const Arguments& args) {
  HandleScope scope;
  int second = args[0]->ToInteger()->Value();
  sp_session_player_seek(application->session, second*1000);
  spotify::currentSecond = second;
  return scope.Close(Undefined());
}

void NodePlayer::setCurrentSecond(int _currentSecond) {
  currentSecond = _currentSecond;
  //call(PLAYER_SECOND_IN_SONG);
}

Handle<Value> NodePlayer::getCurrentSecond(Local<String> property, const AccessorInfo& info) {
  NodePlayer* nodePlayer = node::ObjectWrap::Unwrap<NodePlayer>(info.Holder());
  return Integer::New(nodePlayer->currentSecond);
}

void NodePlayer::init() {
  HandleScope scope;
  Handle<FunctionTemplate> constructorTemplate = NodeWrappedWithCallbacks::init("Player");

  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "play", play);
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "pause", pause);
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "resume", resume);
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "stop", stop);
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "seek", seek);
  constructorTemplate->InstanceTemplate()->SetAccessor(String::NewSymbol("currentSecond"), &getCurrentSecond, emptySetter);
  constructor = Persistent<Function>::New(constructorTemplate->GetFunction());
  scope.Close(Undefined());
}

NodePlayer& NodePlayer::getInstance() {
  return *instance.get();
}

std::unique_ptr<NodePlayer> NodePlayer::instance = std::unique_ptr<NodePlayer>(new NodePlayer());
