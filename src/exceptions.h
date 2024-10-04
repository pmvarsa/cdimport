#pragma once

#include <exception>
#include <stdexcept>

/// Error querying the GNU CDDB replacement database.
class CddbError : public std::runtime_error {
  public:
	/// Only allow construction with a message.
	/// @param message A suitable run time error message for the user.
	CddbError(const std::string & message)
	  : std::runtime_error(message)
	{}
};

/// No CD was found in the CDROM drive.
class NoCdFound : public std::runtime_error {
  public:
	/// Allow default construction with a hard-coded message to the user.
  	NoCdFound()
	  : runtime_error("No CD was found in the CDROM drive.")
	{ }
};

