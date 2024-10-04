
#pragma once

/// Abstract class that contains utility methods.
class Utility
{
  private:
  	/// Private default contructor.
	Utility();

  public:

	/// Utility to format an integer number of seconds into a human-readable string.
	/// @param l A non-negative length of time in seconds.
	/// @return Returns a pretty-printed string representing the time. If the
	///         number of minutes less than 100, then hours isn't output,
	///         instead a two-digit number of minutes are output.
	static std::string readableLength(int l);
};

