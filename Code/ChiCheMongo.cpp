// ChiCheMongo.cpp

#include "ChiCheMongo.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <wx/string.h>
#include <wx/utils.h>
#pragma warning( push )
#pragma warning( disable : 4005 )		// Not sure how to fix this problem.
#	include <bson.h>
#	include <mongoc.h>
#pragma warning( pop )

using namespace ChiChe;

//=====================================================================================
_bson_t* json_as_bson( const char* jsonDoc, bson_error_t* error )
{
	bson_t* bsonDoc = bson_new();
	if( !bson_init_from_json( bsonDoc, jsonDoc, strlen( jsonDoc ), error ) )
	{
		bson_destroy( bsonDoc );
		bsonDoc = nullptr;
	}
	return bsonDoc;
}

//=====================================================================================
Mongo::Mongo( void )
{
	client = nullptr;
	win_collection = nullptr;
	error = new bson_error_t;
}

//=====================================================================================
/*virtual*/ Mongo::~Mongo( void )
{
	Disconnect();

	delete error;
}

//=====================================================================================
/*static*/ bool Mongo::Init( void )
{
	mongoc_init();
	return true;
}

//=====================================================================================
/*static*/ bool Mongo::CleanUp( void )
{
	mongoc_cleanup();
	return true;
}

//=====================================================================================
wxString Mongo::GetError( void )
{
	return error->message;
}

//=====================================================================================
bool Mongo::Connect( void )
{
	if( client )
		return false;

	// Here it is; plain as day!  If I find that someone has corrupted the database,
	// I'll have to revisit this code and switch over to a better authentication mechanism.
	wxString userName = "chiche";
	wxString passWord = "chiche";
	wxString uri = "mongodb://" + userName + ":" + passWord + "@ds057066.mlab.com:57066/chiche";

	client = mongoc_client_new( uri );
	if( !client )
		return false;

	win_collection = mongoc_client_get_collection( client, "chiche", "win_collection" );
	if( !win_collection )
		return false;

	return true;
}

//=====================================================================================
bool Mongo::Disconnect( void )
{
	if( win_collection )
	{
		mongoc_collection_destroy( win_collection );
		win_collection = nullptr;
	}

	if( client )
	{
		mongoc_client_destroy( client );
		client = nullptr;
	}

	return true;
}

//=====================================================================================
bool Mongo::WinEntryToBson( const WinEntry& winEntry, _bson_t*& bsonDoc )
{
	bool success = false;

	do
	{
		bsonDoc = nullptr;

		rapidjson::Document doc;
		doc.SetObject();

		const char* winnerName = winEntry.winnerName.c_str();
		rapidjson::Value winnerNameValue;
		winnerNameValue.SetString( winnerName, strlen( winnerName ), doc.GetAllocator() );
		doc.AddMember( "winnerName", winnerNameValue, doc.GetAllocator() );
		
		doc.AddMember( "score", rapidjson::Value().SetInt64( winEntry.score ), doc.GetAllocator() );
		doc.AddMember( "turnCount", winEntry.turnCount, doc.GetAllocator() );

		int64_t time_seconds = winEntry.dateOfWin.GetTimeNow();
		int64_t time_milliseconds = time_seconds * 1000;
		rapidjson::Value dateOfWinValue;
		dateOfWinValue.SetObject();
		dateOfWinValue.AddMember( "$date", time_milliseconds, doc.GetAllocator() );
		doc.AddMember( "dateOfWin", dateOfWinValue, doc.GetAllocator() );

		rapidjson::StringBuffer buffer;
		rapidjson::Writer< rapidjson::StringBuffer > writer( buffer );
		if( !doc.Accept( writer ) )
			break;

		const char* jsonDoc = buffer.GetString();
		bsonDoc = json_as_bson( jsonDoc, error );
		if( !bsonDoc )
			break;

		success = true;
	}
	while( false );

	return success;
}

//=====================================================================================
bool Mongo::WinEntryFromBson( WinEntry& winEntry, const _bson_t* bsonDoc )
{
	bool success = false;
	char* jsonDoc = nullptr;
	
	do
	{
		jsonDoc = bson_as_json( bsonDoc, NULL );

		rapidjson::Document doc;
		doc.Parse( jsonDoc );
		if( !doc.IsObject() )
			break;

		winEntry.winnerName = doc[ "winnerName" ].GetString();
		winEntry.score = doc[ "score" ].GetInt64();
		winEntry.turnCount = doc[ "turnCount" ].GetInt();
		int64_t time_milliseconds = doc[ "dateOfWin" ][ "$date" ].GetInt64();
		int64_t time_seconds = time_milliseconds / 1000;
		winEntry.dateOfWin.Set( time_seconds );

		success = true;
	}
	while( false );

	if( jsonDoc )
		bson_free( jsonDoc );

	return success;
}

//=====================================================================================
bool Mongo::InsertWinEntry( const WinEntry& winEntry )
{
	bool success = false;
	bson_t* bsonDoc = nullptr;

	do
	{
		wxBusyCursor busyCursor;

		if( !WinEntryToBson( winEntry, bsonDoc ) )
			break;

		// Note that a cap has been put on the collection, we we can add to it without bound and not worry about overflowing our quota.
		// Oldest documents are kicked out to make room for new documents.  There is room for a very large number of documents!
		if( !mongoc_collection_insert( win_collection, MONGOC_INSERT_NONE, bsonDoc, NULL, NULL ) )
			break;

		success = true;
	}
	while( false );

	if( bsonDoc )
		bson_destroy( bsonDoc );

	return success;
}

//=====================================================================================
/*static*/ bool Mongo::FreeWinEntryList( WinEntryList& winEntryList )
{
	while( winEntryList.size() > 0 )
	{
		WinEntryList::iterator iter = winEntryList.begin();
		WinEntry* winEntry = *iter;
		delete winEntry;
		winEntryList.erase( iter );
	}

	return true;
}

//=====================================================================================
bool Mongo::GetTopHighScoresList( WinEntryList& winEntryList, int winEntryListSize )
{
	wxString jsonQuery = "{ \"$query\" : {}, \"$sort\" : { \"score\" : -1 } }";
	return GetWinEntryList( winEntryList, winEntryListSize, jsonQuery );
}

//=====================================================================================
bool Mongo::GetFastestWinList( WinEntryList& winEntryList, int winEntryListSize )
{
	wxString jsonQuery = "{ \"$query\" : {}, \"$sort\" : { \"turnCount\" : 1 } }";
	return GetWinEntryList( winEntryList, winEntryListSize, jsonQuery );
}

//=====================================================================================
bool Mongo::GetWinEntryList( WinEntryList& winEntryList, int winEntryListSize, const wxString& jsonQuery )
{
	bool success = false;
	mongoc_cursor_t* cursor = nullptr;
	bson_t* bsonQuery = nullptr;

	do
	{
		wxBusyCursor busyCursor;

		FreeWinEntryList( winEntryList );

		bsonQuery = json_as_bson( jsonQuery, error );
		if( !bsonQuery )
			break;

		cursor = mongoc_collection_find( win_collection, MONGOC_QUERY_NONE, 0, winEntryListSize, 0, bsonQuery, NULL, NULL );
		if( !cursor )
			break;

		const bson_t* bsonDoc = nullptr;
		while( mongoc_cursor_next( cursor, &bsonDoc ) )
		{
			WinEntry* winEntry = new WinEntry();
			if( WinEntryFromBson( *winEntry, bsonDoc ) )
				winEntryList.push_back( winEntry );
		}

		if( mongoc_cursor_error( cursor, error ) )
			break;

		success = true;
	}
	while( false );

	if( cursor )
		mongoc_cursor_destroy( cursor );

	if( bsonQuery )
		bson_destroy( bsonQuery );

	return success;
}

// ChiCheMongo.cpp