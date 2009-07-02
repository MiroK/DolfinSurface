// Copyright (C) 2003-2009 Anders Logg.
// Licensed under the GNU LGPL Version 2.1.
//
// Thanks to Jim Tilander for many helpful hints.
//
// Modified by Ola Skavhaug, 2007, 2009.
// Modified by Garth N. Wells, 2009.
//
// First added:  2003-03-13
// Last changed: 2009-07-02

#include <boost/scoped_array.hpp>
#include <cstdarg>
#include <stdio.h>
#include <sstream>
#include <dolfin/common/types.h>
#include <dolfin/common/constants.h>
#include <dolfin/common/Variable.h>
#include <dolfin/parameter/NewParameters.h>
#include "LogManager.h"
#include "log.h"

using namespace dolfin;

static boost::scoped_array<char> buffer(0);
static unsigned int buffer_size= 0;

// Buffer allocation
void allocate_buffer(std::string msg)
{
  // va_list, start, end require a char pointer of fixed size so we
  // need to allocate the buffer here. We allocate twice the size of
  // the format string and at least DOLFIN_LINELENGTH. This should be
  // ok in most cases.
  unsigned int new_size = std::max(static_cast<unsigned int>(2*msg.size()),
                                   static_cast<unsigned int>(DOLFIN_LINELENGTH));
  if (new_size > buffer_size)
  {
    buffer.reset(new char[new_size]);
    buffer_size = new_size;
  }
}

// Macro for parsing arguments
#define read(buffer, msg) \
  allocate_buffer(msg); \
  va_list aptr; \
  va_start(aptr, msg); \
  vsnprintf(buffer, buffer_size, msg.c_str(), aptr); \
  va_end(aptr);

//-----------------------------------------------------------------------------
void dolfin::info(std::string msg, ...)
{
  if (!LogManager::logger.is_active()) return; // optimization
  read(buffer.get(), msg);
  LogManager::logger.info(buffer.get());
}
//-----------------------------------------------------------------------------
void dolfin::info(int log_level, std::string msg, ...)
{
  if (!LogManager::logger.is_active()) return; // optimization
  read(buffer.get(), msg);
  LogManager::logger.info(buffer.get(), log_level);
}
//-----------------------------------------------------------------------------
void dolfin::info(const Variable& variable)
{
  if (!LogManager::logger.is_active()) return; // optimization
  info(variable.str());
}
//-----------------------------------------------------------------------------
void dolfin::info(const NewParameters& parameters)
{
  // Need separate function for Parameters since we can't make Parameters
  // a subclass of Variable (gives cyclic dependencies)
  if (!LogManager::logger.is_active()) return; // optimization
  info(parameters.str());
}
//-----------------------------------------------------------------------------
void dolfin::info_stream(std::ostream& out, std::string msg)
{
  if (!LogManager::logger.is_active()) return; // optimization
  std::ostream& old_out = LogManager::logger.get_output_stream();
  LogManager::logger.set_output_stream(out);
  LogManager::logger.info(msg);
  LogManager::logger.set_output_stream(old_out);
}
//-----------------------------------------------------------------------------
void dolfin::info_underline(std:: string msg, ...)
{
  if (!LogManager::logger.is_active()) return; // optimization
  read(buffer.get(), msg);
  LogManager::logger.info_underline(buffer.get());
}
//-----------------------------------------------------------------------------
void dolfin::warning(std::string msg, ...)
{
  if (!LogManager::logger.is_active()) return; // optimization
  read(buffer.get(), msg);
  LogManager::logger.warning(buffer.get());
}
//-----------------------------------------------------------------------------
void dolfin::error(std::string msg, ...)
{
  read(buffer.get(), msg);
  LogManager::logger.error(buffer.get());
}
//-----------------------------------------------------------------------------
void dolfin::begin(std::string msg, ...)
{
  if (!LogManager::logger.is_active()) return; // optimization
  read(buffer.get(), msg);
  LogManager::logger.begin(buffer.get());
}
//-----------------------------------------------------------------------------
void dolfin::begin(int log_level, std::string msg, ...)
{
  if (!LogManager::logger.is_active()) return; // optimization
  read(buffer.get(), msg);
  LogManager::logger.begin(buffer.get(), log_level);
}
//-----------------------------------------------------------------------------
void dolfin::end()
{
  if (!LogManager::logger.is_active()) return; // optimization
  LogManager::logger.end();
}
//-----------------------------------------------------------------------------
void dolfin::logging(bool active)
{
  LogManager::logger.logging(active);
}
//-----------------------------------------------------------------------------
void dolfin::set_log_level(uint level)
{
  LogManager::logger.set_log_level(level);
}
//-----------------------------------------------------------------------------
dolfin::uint dolfin::get_log_level()
{
  return LogManager::logger.get_log_level();
}
//-----------------------------------------------------------------------------
std::string dolfin::indent(std::string s)
{
  const std::string indentation("  ");
  std::stringstream is;
  is << indentation;
  for (uint i = 0; i < s.size(); ++i)
  {
    is << s[i];
    if (s[i] == '\n') // && i < s.size() - 1)
      is << indentation;
  }

  return is.str();
}
//-----------------------------------------------------------------------------
void dolfin::summary(bool reset)
{
  if (!LogManager::logger.is_active()) return; // optimization
  LogManager::logger.summary(reset);
}
//-----------------------------------------------------------------------------
double dolfin::timing(std::string task, bool reset)
{
  return LogManager::logger.timing(task, reset);
}
//-----------------------------------------------------------------------------
void dolfin::__debug(std::string file, unsigned long line,
                     std::string function, std::string format, ...)
{
  read(buffer.get(), format);
  std::ostringstream ost;
  ost << file << ":" << line << " in " << function << "()";
  std::string msg = std::string(buffer.get()) + " [at " + ost.str() + "]";
  LogManager::logger.__debug(msg);
}
//-----------------------------------------------------------------------------
void dolfin::__dolfin_assert(std::string file, unsigned long line,
                      std::string function, std::string format, ...)
{
  read(buffer.get(), format);
  std::ostringstream ost;
  ost << file << ":" << line << " in " << function << "()";
  std::string msg = std::string(buffer.get()) + " [at " + ost.str() + "]";
  LogManager::logger.__assert(msg);
}
//-----------------------------------------------------------------------------
