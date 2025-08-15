#include <stdio.h>
#include <hdf5_hl.h>
#include <hdf5.h>


#include "lib/typedefs.h"
#include "models.h"

#include "models.h"

#ifndef MARKET_H
#define MARKET_H

TradesArray read_hdf5(char* path);
i32 validate_file(char* path);

#endif
