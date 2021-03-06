import cqlat as c

from qlat.timer import *
from qlat.lat_io import *
from qlat.propagator import *

@timer
def contract_chvp3_field(prop1, prop2, tslice):
    ld = LatData()
    c.contract_chvp3_sfield(ld, prop1, prop2, tslice)
    return ld
