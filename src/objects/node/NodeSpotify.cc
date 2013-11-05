
#include "NodeSpotify.h"
#include "../../Application.h"
#include "../../Callbacks/SessionCallbacks.h"
#include "../spotify/SpotifyOptions.h"
#include "NodePlaylist.h"
#include "NodePlayer.h"
#include "NodeArtist.h"
#include "NodeAlbum.h"
#include "NodeTrack.h"

extern Application* application;

NodeSpotify::NodeSpotify(Handle<Object> options) {
  /*
   * Important note: The session callbacks must be initialized before
   * the sp_session is started. The uv timer and async handle must be set up
   * as they will be used as soon as the sp_session is created, which happens in the
   * NodeSpotify ctor.
   */
  //initiate uv_timer and uv_async
  SessionCallbacks::init();

  SpotifyOptions _options;
  HandleScope scope;
  Handle<String> settingsFolderKey = String::New("settingsFolder");
  Handle<String> cacheFolderKey = String::New("cacheFolder");
  Handle<String> traceFileKey = String::New("traceFile");
  if(options->Has(settingsFolderKey)) {
    String::Utf8Value settingsFolderValue(options->Get(settingsFolderKey)->ToString());
    _options.settingsFolder = *settingsFolderValue;
  } else {
    _options.settingsFolder = "settings";
  }
  if(options->Has(cacheFolderKey)) {
    String::Utf8Value cacheFolderValue(options->Get(cacheFolderKey)->ToString());
    _options.cacheFolder = *cacheFolderValue;
  } else {
    _options.cacheFolder = "cache";
  }
  if(options->Has(traceFileKey)) {
    String::Utf8Value traceFileValue(options->Get(traceFileKey)->ToString());
    _options.traceFile = *traceFileValue;
  }
  spotify = std::unique_ptr<Spotify>(new Spotify(_options));
  scope.Close(Undefined());
}

Handle<Value> NodeSpotify::createFromLink(const Arguments& args) {
  HandleScope scope;
  Handle<Value> out;
  String::Utf8Value linkToParse(args[0]->ToString());
  sp_link* parsedLink = sp_link_create_from_string(*linkToParse);
  if(parsedLink != nullptr) {
    sp_linktype linkType = sp_link_type(parsedLink);
    switch(linkType) {
      case SP_LINKTYPE_TRACK:
      {
        sp_track* track = sp_link_as_track(parsedLink);
        NodeTrack* nodeTrack = new NodeTrack(std::make_shared<Track>(track));
        out = nodeTrack->getV8Object();
        break;
      }
      case SP_LINKTYPE_ALBUM:
      {
        sp_album* album = sp_link_as_album(parsedLink);
        NodeAlbum* nodeAlbum = new NodeAlbum(std::make_shared<Album>(album));
        out = nodeAlbum->getV8Object();
        break;
      }
      case SP_LINKTYPE_ARTIST:
      {
        sp_artist* artist = sp_link_as_artist(parsedLink);
        NodeArtist* nodeArtist = new NodeArtist(std::make_shared<Artist>(artist));
        out = nodeArtist->getV8Object();
        break;
      }
      case SP_LINKTYPE_SEARCH:
        out = Undefined();
        break;
      case SP_LINKTYPE_PLAYLIST:
        out = Undefined();
        break;
      default:
        out = Undefined();
    }
  } else {
    out = Undefined();
  }
  sp_link_release(parsedLink);
  return scope.Close(out);
}

Handle<Value> NodeSpotify::login(const Arguments& args) {
  HandleScope scope;
  NodeSpotify* nodeSpotify = node::ObjectWrap::Unwrap<NodeSpotify>(args.This());
  String::Utf8Value v8User(args[0]->ToString());
  String::Utf8Value v8Password(args[1]->ToString());
  bool rememberMe = args[2]->ToBoolean()->Value();
  bool withRemembered = args[3]->ToBoolean()->Value();
  std::string user(*v8User);
  std::string password(*v8Password);
  nodeSpotify->spotify->login(user, password, rememberMe, withRemembered);
  return scope.Close(Undefined());
}

Handle<Value> NodeSpotify::logout(const Arguments& args) {
  HandleScope scope;
  NodeSpotify* nodeSpotify = node::ObjectWrap::Unwrap<NodeSpotify>(args.This());
  nodeSpotify->spotify->logout();
  return scope.Close(Undefined());
}

Handle<Value> NodeSpotify::ready(const Arguments& args) {
  HandleScope scope;
  Handle<Function> fun = Handle<Function>::Cast(args[0]);
  Persistent<Function> p = Persistent<Function>::New(fun);
  SessionCallbacks::loginCallback = p;
  return scope.Close(Undefined());
}

Handle<Value> NodeSpotify::getPlaylists(const Arguments& args) {
  HandleScope scope;
  std::vector<std::shared_ptr<Playlist>> playlists = application->playlistContainer->getPlaylists();
  Local<Array> nPlaylists = Array::New(playlists.size());
  for(int i = 0; i < (int)playlists.size(); i++) {
    NodePlaylist* nodePlaylist = new NodePlaylist(playlists[i]);
    nPlaylists->Set(Number::New(i), nodePlaylist->getV8Object());
  }
  return scope.Close(nPlaylists);
}

Handle<Value> NodeSpotify::getStarred(const Arguments& args) {
  HandleScope scope;
  NodePlaylist* starredPlaylist = new NodePlaylist(application->playlistContainer->starredPlaylist);
  return scope.Close(starredPlaylist->getV8Object());
}

Handle<Value> NodeSpotify::getRememberedUser(Local<String> property, const AccessorInfo& info) {
  HandleScope scope;
  NodeSpotify* nodeSpotify = node::ObjectWrap::Unwrap<NodeSpotify>(info.Holder());
  return scope.Close(String::New(nodeSpotify->spotify->rememberedUser().c_str()));
}

void NodeSpotify::init() {
  HandleScope scope;
  Handle<FunctionTemplate> constructorTemplate = NodeWrapped::init("Spotify");
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "login", login);
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "logout", logout);
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "getPlaylists", getPlaylists);
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "getStarred", getStarred);
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "ready", ready);
  NODE_SET_PROTOTYPE_METHOD(constructorTemplate, "createFromLink", createFromLink);
  constructorTemplate->InstanceTemplate()->SetAccessor(String::NewSymbol("rememberedUser"), getRememberedUser, emptySetter);
  constructor = Persistent<Function>::New(constructorTemplate->GetFunction());
  scope.Close(Undefined());
}