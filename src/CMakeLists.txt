set (CD_IMPORT_SOURCES
	cd_chooser.cpp
	cd_import.cpp
	cddb.cpp
	edit_track.cpp
	main.cpp
	pg_conn.cpp
	track_data_model.cpp
	utility.cpp
)

set (CD_IMPORT_UIS
	designer/cd_chooser.ui
	designer/edit_track.ui
	designer/cd_import.ui
)

add_executable (cdimport ${CD_IMPORT_SOURCES} ${CD_IMPORT_UIS})

# Installs nix in /usr/local/bin
INSTALL (
	TARGETS
		cdimport
	DESTINATION
		bin
	PERMISSIONS
		OWNER_READ
		OWNER_WRITE
		OWNER_EXECUTE
		GROUP_READ
		GROUP_EXECUTE
		WORLD_READ
		WORLD_EXECUTE
)

target_link_libraries(cdimport Qt5::Widgets pqxx)

