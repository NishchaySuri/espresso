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

#include "virtual_sites.hpp"

#ifdef VIRTUAL_SITES
#include "pressure.hpp"
#include "cells.hpp"

// The following four functions are independent of the specif
// rules used to place virtual particles

void update_mol_vel_pos() {
  // replace this by a better implementation later!

  // ORDER MATTERS! Update_mol_vel may rely on correct positions of virtual
  // particcles
  update_mol_pos();
  update_mol_vel();
}

void update_mol_vel() {
#ifndef VIRTUAL_SITES_NO_VELOCITY
  for (auto &p : local_cells.particles()) {
    if (ifParticleIsVirtual(&p))
      update_mol_vel_particle(&p);
  }
#endif
}

void update_mol_pos() {
  for (auto &p : local_cells.particles()) {
    if (ifParticleIsVirtual(&p))
      update_mol_pos_particle(&p);
  }
}

#endif
