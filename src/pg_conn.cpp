
#include <cassert>
#include <iostream>
#include <tuple>

#include "pg_conn.h"

/// Not much of a macro, but I want to match the #CATCH macro.
#define TRY try {

/// Boilerplate code to create a database connection and start a transaction.
#define CONN \
pqxx::connection conn(DB_CONNECTION_STRING); \
pqxx::work w(conn);

/// Boilerplate code to commit the transaction.
#define ABORT w.abort();

/// Boilerplate code to commit the transaction.
#define COMMIT w.commit();

/// Boilerplate catch block for a failed database connection.
#define CATCH \
} catch(pqxx::broken_connection e) { \
	std::cerr << "Failed to connect to the database: " << e.what() << std::endl; \
}

const std::string PgConn::DB_NAME { "albums" };
const std::string PgConn::DB_HOST { "elephant" };
const std::string PgConn::DB_USER { "pmvarsa" };
const std::string PgConn::DB_CONNECTION_STRING { "postgresql://pmvarsa@elephant/albums" };

pqxx::result PgConn::queryCdDiscId(const std::string & cdDiscId)
{
	TRY
		CONN	// Create an RAII connection and a transaction

		auto results = w.exec_params(
			"SELECT artist, title, categories.category "
				"FROM albums "
				"INNER JOIN categories "
				"ON albums.category_id = categories.category_id "
				"WHERE albums.disc_id = $1;", cdDiscId);
		COMMIT	// Commit the transaction
		return results;
	CATCH
	return pqxx::result();
}

pqxx::result PgConn::queryArtistTitle(const std::string & artist, const std::string & title)
{
	TRY
		CONN
		auto results = w.exec_params(
			"SELECT artist, title, category "
			"FROM v_albums "
			"WHERE title ILIKE $1 AND artist ILIKE $2;", title, artist);
		COMMIT
		return results;
	CATCH
	return pqxx::result();
}

bool PgConn::insertCd(const Cd::CdAlbumData & album, const Track::TrackList & tracks)
{
	using std::get;
	TRY
		CONN
#ifdef DEBUG
		using std::cout, std::endl, std::flush;
		cout << "Inserting an entry into the albums table." << endl;
#endif
		auto result = w.exec_params(
			"INSERT INTO albums("
				"medium_id, "
				"type_id, "
				"category_id, "
				"is_compilation, "
				"result_id, "
				"disc_id, "
				"title, "
				"artist, "
				"genre, "
				"length, "
				"extra_info, "
				"year, "
				"num_tracks"
			") VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13) "
			"RETURNING album_id",
			get<Cd::MediumId>(album),
			get<Cd::TypeId>(album),
			get<Cd::CategoryId>(album),
			get<Cd::IsCompilation>(album),
			get<Cd::ResultId>(album),
			get<Cd::DiscId>(album),
			get<Cd::Title>(album),
			get<Cd::Artist>(album),
			get<Cd::Genre>(album),
			get<Cd::Length>(album),
			get<Cd::ExtraInfo>(album),
			get<Cd::Year>(album),
			get<Cd::NumberOfTracks>(album)
		);

		// Test that the insert succeeded
		if(result.size() > 0) {
			int album_id = result[0][0].as<int>(); // index is faster than name
#ifdef DEBUG
			cout << "Successfully inserted " << result.size() << " item(s) into the database. "
				 << "The new album_id is " << album_id << "." << endl;
#endif
			for(int i=0;i<tracks.size();++i) {
				auto track = tracks[i];
				auto result = w.exec_params(
					"INSERT INTO tracks (" 
						"album_id, "
						"number, "
						"name, "
						"length, "
						"extra_info"
					") VALUES ($1, $2, $3, $4, $5) "
					"RETURNING track_id",
					album_id,
					i+1,
					get<Track::Title>(track),
					get<Track::Length_S>(track),
					get<Track::ExtraInfo>(track)
				);
				assert(result.size() == 1);
#ifdef DEBUG
				int track_id = result[0][0].as<int>();
				cout << "Successfully added track " << (i+1) << ". '"
					 << get<Track::Title>(track) << "' "
					 << "as track_id " << track_id << "." << endl;
#endif
			}
		} else {
			std::cerr << "Failed to insert into the albums table." << std::endl;
			return false;
		}
#ifdef DEBUG
		// Abort the transaction in Debug mode
		cout << "*** *** *** ABORTING TRANSACTION IN Debug MODE *** *** ***" << endl;
		ABORT
#else
		COMMIT
#endif
	CATCH

	return true;
}

