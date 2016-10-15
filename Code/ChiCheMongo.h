// ChiCheMongo.h

#pragma once

#include <wx/datetime.h>
#include <list>

struct _bson_t;
struct _mongoc_client_t;
struct _mongoc_collection_t;

namespace ChiChe
{
	class Mongo;
}

//=====================================================================================
class ChiChe::Mongo
{
public:

	Mongo( void );
	virtual ~Mongo( void );

	static bool Init( void );
	static bool CleanUp( void );

	bool Connect( void );
	bool Disconnect( void );

	struct WinEntry
	{
		wxString winnerName;
		long score;
		int turnCount;
		wxDateTime dateOfWin;
	};

	typedef std::list< WinEntry* > WinEntryList;

	bool InsertWinEntry( const WinEntry& winEntry );

	static bool FreeWinEntryList( WinEntryList& winEntryList );
	bool GetTopHighScoresList( WinEntryList& winEntryList, int winEntryListSize );
	bool GetFastestWinList( WinEntryList& winEntryList, int winEntryListSize );
	bool GetWinEntryList( WinEntryList& winEntryList, int winEntryListSize, const wxString& jsonQuery );

private:

	bool WinEntryToBson( const WinEntry& winEntry, _bson_t*& bsonDoc );
	bool WinEntryFromBson( WinEntry& winEntry, const _bson_t* bsonDoc );

	_mongoc_client_t* client;
	_mongoc_collection_t* win_collection;
};

// ChiCheMongo.h