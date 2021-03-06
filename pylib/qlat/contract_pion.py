import cqlat as c

from qlat.timer import *
from qlat.lat_io import *
from qlat.propagator import *

@timer
def contract_pion_field(prop, tslice):
    ld = LatData()
    if isinstance(prop, Prop):
        c.contract_pion_field(ld, prop, tslice)
    elif isinstance(prop, SelProp):
        c.contract_pion_sfield(ld, prop, tslice)
    else:
        raise Exception("contract_pion_field")
    return ld
