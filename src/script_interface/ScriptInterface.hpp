/*
  Copyright (C) 2010,2011,2012,2013,2014,2015,2016 The ESPResSo project
  Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2009,2010
    Max-Planck-Institute for Polymer Research, Theory Group

  This file is part of ESPResSo.

  ESPResSo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  ESPResSo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SCRIPT_INTERFACE_SCRIPT_INTERFACE_HPP
#define SCRIPT_INTERFACE_SCRIPT_INTERFACE_HPP

#include "initialize.hpp"
#include "ScriptInterfaceBase.hpp"

namespace ScriptInterface {
  enum { NOT_SET = -1 };

  inline std::shared_ptr<ScriptInterfaceBase> get_instance(Variant value) {
  const int id = boost::get<OId>(value).id;

  return ScriptInterfaceBase::get_instance(id).lock();
}

} /* namespace ScriptInterface */

#endif
