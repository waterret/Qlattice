import rbc_ukqcd_params as rup

job_tag = "24D"

dict_params = {}

rup.dict_params[job_tag] = dict_params

dict_params["total_site"] = [ 24, 24, 24, 64, ]

dict_params["load_config_params"] = { "twist_boundary_at_boundary":[ 0.0, 0.0, 0.0, -0.5, ] }

def mk_fermion_params(inv_type, inv_acc):
    params = {}
    params["M5"] = 1.8
    params["boundary_phases"] = [1.0, 1.0, 1.0, 1.0] # twist boundary after loading configuration
    params["b"] = 2.5
    params["c"] = 1.5
    if inv_type == 0:
        params["mass"] = 0.00107
    elif inv_type == 1:
        params["mass"] = 0.0850
    else:
        assert False
    if inv_type == 0 and (inv_acc == 0 or inv_acc == 1):
        params["b"] = 1.0
        params["c"] = 0.0
        params["omega"] = [
                1.0903256131299373,
                0.9570283702230611,
                0.7048886040934104,
                0.48979921782791747,
                0.328608311201356,
                0.21664245377015995,
                0.14121112711957107,
                0.0907785101745156,
                0.05608303440064219 - 0.007537158177840385j,
                0.05608303440064219 + 0.007537158177840385j,
                0.0365221637144842 - 0.03343945161367745j,
                0.0365221637144842 + 0.03343945161367745j,
                ]
    else:
        params["Ls"] = 24
    return params

def mk_dict_fermion_params():
    params = {}
    for inv_type in [0, 1,]:
        params[inv_type] = {}
        for inv_acc in [0, 1, 2,]:
            params[inv_type][inv_acc] = mk_fermion_params(inv_type, inv_acc)
    return params

dict_params["fermion_params"] = mk_dict_fermion_params()

def mk_lanc_params(inv_type, inv_acc):
    assert inv_type == 0
    assert inv_acc == 0
    fermion_params = dict_params["fermion_params"][inv_type][inv_acc]
    pit_params = { "eps": 0.01, "maxiter": 500, "real": True }
    cheby_params = { "low": 2.97e-4, "high": 5.5, "order": 200 }
    irl_params = {
            "Nstop": 1000,
            "Nk": 1040,
            "Nm": 1200,
            "resid": 1e-8,
            "betastp": 0.0,
            "maxiter": 20,
            "Nminres": 1,
            # "maxapply": 100
            }
    return {
            "fermion_params": fermion_params,
            "pit_params": pit_params,
            "cheby_params": cheby_params,
            "irl_params": irl_params,
            }

dict_params["lanc_params"] = { 0:{ 0:mk_lanc_params(0, 0) } }

def mk_clanc_params(inv_type, inv_acc):
    assert inv_type == 0
    assert inv_acc == 0
    block = [ 2, 3, 3, 2, ]
    nbasis = 1000
    cheby_params = {"low": 0.000684, "high": 5.5, "order": 200}
    irl_params = {
            "Nstop": 2000,
            "Nk": 2100,
            "Nm": 2600,
            "resid": 1e-8,
            "betastp": 0.0,
            "maxiter": 20,
            "Nminres": 0,
            # "maxapply": 100
            }
    smoother_params = {"eps": 1e-8, "maxiter": 100}
    save_params = {"nsingle": 150, "mpi": [ 1, 1, 1, 8, ]}
    return {
            "block": block,
            "nbasis": nbasis,
            "cheby_params": cheby_params,
            "irl_params": irl_params,
            "smoother_params": smoother_params,
            "save_params": save_params,
            }

dict_params["clanc_params"] = { 0:{ 0:mk_clanc_params(0, 0) } }
