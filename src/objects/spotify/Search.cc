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

#include "Search.h"

#include <libspotify/api.h>
#include "../../Application.h"
#include "../../callbacks/SearchCallbacks.h"

extern Application* application;

Search::Search(const Search& other) : search(other.search) {
  sp_search_add_ref(search);
};

Search::~Search() {
  sp_search_release(search);
};

std::string Search::link() {
  std::string link;
  if(sp_search_is_loaded(search)) {
    sp_link* spLink = sp_link_create_from_search(search);
    char linkChar[256];
    sp_link_as_string(spLink, linkChar, 256);
    link = std::string(linkChar);
    sp_link_release(spLink);
  }
  return link;
}

std::string Search::didYouMeanText() {
  std::string didYouMeanText;
  if(sp_search_is_loaded(search)) {
    didYouMeanText = std::string(sp_search_did_you_mean(search));
  }
  return didYouMeanText;
}

std::vector<std::shared_ptr<Track>> Search::getTracks() {
  std::vector<std::shared_ptr<Track>> tracks(sp_search_num_tracks(search));
  if(sp_search_is_loaded(search)) {
    for(int i = 0; i < (int)tracks.size() ; ++i) {
      tracks[i] = std::make_shared<Track>(sp_search_track(search, i));
    }
  }
  return tracks;
}

std::vector<std::shared_ptr<Album>> Search::getAlbums() {
  std::vector<std::shared_ptr<Album>> albums(sp_search_num_albums(search));
  if(sp_search_is_loaded(search)) {
    for(int i = 0; i < (int)albums.size() ; ++i) {
      albums[i] = std::make_shared<Album>(sp_search_album(search, i));
    }
  }
  return albums;
}

std::vector<std::shared_ptr<Artist>> Search::getArtists() {
  std::vector<std::shared_ptr<Artist>> artists(sp_search_num_artists(search));
  if(sp_search_is_loaded(search)) {
    for(int i = 0; i < (int)artists.size() ; ++i) {
      artists[i] = std::make_shared<Artist>(sp_search_artist(search, i));
    }
  }
  return artists;
}

std::vector<std::shared_ptr<Playlist>> Search::getPlaylists() {
  std::vector<std::shared_ptr<Playlist>> playlists(sp_search_num_playlists(search));
  if(sp_search_is_loaded(search)) {
    for(int i = 0; i < (int)playlists.size() ; ++i) {
      playlists[i] = std::make_shared<Playlist>(sp_search_playlist(search, i), -1);
    }
  }
  return playlists;
}

int Search::totalTracks() {
  int totalTracks = 0;
  if(sp_search_is_loaded(search)) {
    totalTracks = sp_search_total_tracks(search);
  }
  return totalTracks;
}

int Search::totalAlbums() {
  int totalAlbums = 0;
  if(sp_search_is_loaded(search)) {
    totalAlbums = sp_search_total_albums(search);
  }
  return totalAlbums;
}

int Search::totalArtists() {
  int totalArtists = 0;
  if(sp_search_is_loaded(search)) {
    totalArtists = sp_search_total_artists(search);
  }
  return totalArtists;
}

int Search::totalPlaylists() {
  int totalPlaylists = 0;
  if(sp_search_is_loaded(search)) {
    totalPlaylists = sp_search_total_playlists(search);
  }
  return totalPlaylists;
}

void Search::execute(std::string query, int trackOffset, int trackLimit,
    int albumOffset, int albumLimit,
    int artistOffset, int artistLimit,
    int playlistOffset, int playlistLimit) {
  this->search = sp_search_create(application->session, query.c_str(),
    trackOffset, trackLimit,
    albumOffset, albumLimit,
    artistOffset, artistLimit,
    playlistOffset, playlistLimit,
    SP_SEARCH_STANDARD, //?
    SearchCallbacks::searchComplete,
    this
  );
}