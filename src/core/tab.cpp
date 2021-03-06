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
/** \file tab.cpp
 *
 *  Implementation of \ref tab.hpp
 */
#include "tab.hpp"

#ifdef TABULATED
#include "communication.hpp"

int tabulated_set_params(int part_type_a, int part_type_b, char *filename) {
  IA_parameters *data;
  FILE *fp;
  int npoints;
  double minval, maxval;
  int i, newsize;
  int token;
  double dummr;
  token = 0;

  data = get_ia_param_safe(part_type_a, part_type_b);

  if (!data)
    return 1;

  if (strlen(filename) > MAXLENGTH_TABFILE_NAME - 1)
    return 2;

  /* Open the file containing force and energy tables */
  fp = fopen(filename, "r");
  if (!fp)
    return 3;

  /* Look for a line starting with # */
  while (token != EOF) {
    token = fgetc(fp);
    if (token == '#') {
      break;
    }
  }
  if (token == EOF) {
    fclose(fp);
    return 4;
  }

  /* First read two important parameters we read in the data later*/
  if (fscanf(fp, "%d %lf %lf", &npoints, &minval, &maxval) != 3)
    return 5;

  // Set the newsize to the same as old size : only changed if a new force table
  // is being added.
  newsize = tabulated_forces.max;

  if (data->TAB_npoints == 0) {
    // A new potential will be added so set the number of points, the startindex
    // and newsize
    data->TAB_npoints = npoints;
    data->TAB_startindex = tabulated_forces.max;
    newsize += npoints;
  } else {
    // We have existing data for this pair of monomer types check array sizing
    if (data->TAB_npoints != npoints) {
      fclose(fp);
      return 6;
    }
  }

  /* Update parameters symmetrically */
  data->TAB_maxval = maxval;
  data->TAB_minval = minval;
  strcpy(data->TAB_filename, filename);

  /* Calculate dependent parameters */
  data->TAB_stepsize = (maxval - minval) / (double)(data->TAB_npoints - 1);

  /* Allocate space for new data */
  tabulated_forces.resize(newsize);
  tabulated_energies.resize(newsize);

  /* Read in the new force and energy table data */
  for (i = 0; i < npoints; i++) {
    if (fscanf(fp, "%lf %lf %lf", &dummr,
               &(tabulated_forces[i + data->TAB_startindex]),
               &(tabulated_energies[i + data->TAB_startindex])) != 3)
      return 5;
  }

  fclose(fp);

  /* broadcast interaction parameters including force and energy tables*/
  mpi_bcast_ia_params(part_type_a, part_type_b);

  return 0;
}

int tabulated_bonded_set_params(int bond_type,
                                TabulatedBondedInteraction tab_type,
                                char *filename) {
  int i, token = 0, size;
  double dummr;
  FILE *fp;

  if (bond_type < 0)
    return 1;

  make_bond_type_exist(bond_type);

  fp = fopen(filename, "r");
  if (!fp)
    return 3;

  /*Look for a line starting with # */
  while (token != EOF) {
    token = fgetc(fp);
    if (token == '#') {
      break;
    } // magic number for # symbol
  }
  if (token == EOF) {
    fclose(fp);
    return 4;
  }

  /* set types */
  bonded_ia_params[bond_type].type = BONDED_IA_TABULATED;
  bonded_ia_params[bond_type].p.tab.type = tab_type;

  /* set number of interaction partners */
  if (tab_type == TAB_BOND_LENGTH)
    bonded_ia_params[bond_type].num = 1;
  else if (tab_type == TAB_BOND_ANGLE)
    bonded_ia_params[bond_type].num = 2;
  else if (tab_type == TAB_BOND_DIHEDRAL)
    bonded_ia_params[bond_type].num = 3;
  else {
    runtimeError("Unsupported tabulated bond type.");
    return 1;
  }

  /* copy filename */
  size = strlen(filename);
  bonded_ia_params[bond_type].p.tab.filename =
      (char *)Utils::malloc((size + 1) * sizeof(char));
  strcpy(bonded_ia_params[bond_type].p.tab.filename, filename);

  /* read basic parameters from file */
  if (fscanf(fp, "%d %lf %lf", &size, &bonded_ia_params[bond_type].p.tab.minval,
             &bonded_ia_params[bond_type].p.tab.maxval) != 3)
    return 5;
  bonded_ia_params[bond_type].p.tab.npoints = size;

  /* Check interval for angle and dihedral potentials.  With adding
     ROUND_ERROR_PREC to the upper boundary we make sure, that during
     the calculation we do not leave the defined table!
  */
  if (tab_type == TAB_BOND_ANGLE) {
    if (bonded_ia_params[bond_type].p.tab.minval != 0.0 ||
        std::abs(bonded_ia_params[bond_type].p.tab.maxval - PI) > 1e-5) {
      fclose(fp);
      return 6;
    }
    bonded_ia_params[bond_type].p.tab.maxval = PI + ROUND_ERROR_PREC;
  }
  /* check interval for angle and dihedral potentials */
  if (tab_type == TAB_BOND_DIHEDRAL) {
    if (bonded_ia_params[bond_type].p.tab.minval != 0.0 ||
        std::abs(bonded_ia_params[bond_type].p.tab.maxval - (2 * PI)) > 1e-5) {
      fclose(fp);
      return 6;
    }
    bonded_ia_params[bond_type].p.tab.maxval = (2 * PI) + ROUND_ERROR_PREC;
  }

  /* calculate dependent parameters */
  bonded_ia_params[bond_type].p.tab.invstepsize =
      (double)(size - 1) / (bonded_ia_params[bond_type].p.tab.maxval -
                            bonded_ia_params[bond_type].p.tab.minval);

  /* allocate force and energy tables */
  bonded_ia_params[bond_type].p.tab.f =
      (double *)Utils::malloc(size * sizeof(double));
  bonded_ia_params[bond_type].p.tab.e =
      (double *)Utils::malloc(size * sizeof(double));

  /* Read in the new force and energy table data */
  for (i = 0; i < size; i++) {
    if (fscanf(fp, "%lf %lf %lf", &dummr,
               &bonded_ia_params[bond_type].p.tab.f[i],
               &bonded_ia_params[bond_type].p.tab.e[i]) != 3)
      return 5;
  }
  fclose(fp);

  mpi_bcast_ia_params(bond_type, -1);

  return ES_OK;
}

#endif
