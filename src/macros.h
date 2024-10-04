
#pragma once

/// It's really annoying to that Qt doesn't handle std::string implicitly, and
/// that the conversion method has such a long name. Alternatively, once can
/// coerce a zero-terminated C-style string easily. However, according to a
/// stack overflow post, the long-named static function is more robust. I.e.,
/// rather than doing something like this:
/// \code
/// std::string s = "Hello, how are you?";
/// QString qs = s.c_str();
/// \endcode
/// A better approach is to do this:
/// \code
/// std::string s = "Hello, how are you?";
/// QString qs = QString::fromStdString(s);
/// \endcode
/// However, that takes 24 characters, and I like an 80-column shell. So, I've
/// created this macro to save some horizontal space.
#define QStr(S) (QString::fromStdString(S))

