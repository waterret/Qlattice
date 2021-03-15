import qlat_gpt as qg
import qlat as q
import gpt as g

def get_total_site(job_tag):
    if job_tag == "test-8nt16":
        return [8, 8, 8, 16]
    elif job_tag == "24D":
        return [24, 24, 24, 64]
    else:
        raise Exception("get_total_site")

def get_fermion_param(job_tag, inv_type, inv_accuracy):
    params = {
            "M5": 1.8,
            "b": 1.0,
            "c": 0.0,
            "boundary_phases": [1.0, 1.0, 1.0, 1.0],
            }
    if job_tag == "test-8nt16":
        params["b"] = 1.5
        params["c"] = 0.5
        if inv_type == 0:
            params["mass"] = 0.01
        elif inv_type == 1:
            params["mass"] = 0.04
        params["Ls"] = 8
    elif job_tag == "24D" or job_tag == "32D" or job_tag == "48D" or job_tag == "24DH":
        params["b"] = 2.5
        params["c"] = 1.5
        if inv_type == 0:
            params["mass"] = 0.00107
        elif inv_type == 1:
            params["mass"] = 0.0850
        else:
            raise Exception("get_fermion_param")
        if inv_type == 0 and (inv_accuracy == 0 or inv_accuracy == 1):
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
                    0.0365221637144842 + 0.03343945161367745j
                    ]
        else:
            params["Ls"] = 24
    elif job_tag == "48I":
        params["b"] = 1.5
        params["c"] = 0.5
        if inv_type == 0:
            params["mass"] = 0.0006979
        elif inv_type == 1:
            params["mass"] = 0.03580
        else:
            raise Exception("get_fermion_param")
        if inv_type == 0 and (inv_accuracy == 0 or inv_accuracy == 1):
            raise Exception("get_fermion_param")
        elif inv_type == 1 or inv_accuracy == 2:
            params["Ls"] = 24
        else:
            raise Exception("get_fermion_param")
    else:
        raise Exception("get_fermion_param")
    return params

@q.timer
def mk_inverter(gf, job_tag, inv_type, inv_accuracy):
    gpt_gf = qg.gpt_from_qlat(gf)
    pc = g.qcd.fermion.preconditioner
    if inv_type == 1:
        param = get_fermion_param(job_tag, inv_type, inv_accuracy)
        qm = None
        if "omega" in param:
            qm = g.qcd.fermion.zmobius(gpt_gf, param)
        else:
            qm = g.qcd.fermion.mobius(gpt_gf, param)
        inv = g.algorithms.inverter
        cg_mp = None
        if inv_type == 0:
            cg_mp = inv.cg({"eps": 5e-5, "maxiter": 200})
        elif inv_type == 1:
            cg_mp = inv.cg({"eps": 5e-5, "maxiter": 300})
        else:
            raise Exception("mk_inverter")
        slv_5d_mp = inv.preconditioned(pc.eo2_ne(), cg_mp)
        maxiter = 100
        if inv_accuracy == 0:
            maxiter = 1
        elif inv_accuracy == 1:
            maxiter = 2
        elif inv_accuracy == 2:
            maxiter = 50
        else:
            raise Exception("mk_inverter")
        slv_qm = qm.propagator(
                inv.defect_correcting(
                    inv.mixed_precision(
                        slv_5d_mp, g.single, g.double),
                    eps=1e-8, maxiter=maxiter))
        timer = q.Timer(f"py:inv({job_tag},{inv_type},{inv_accuracy})", True)
        inv_qm = qg.InverterGPT(inverter = slv_qm, timer = timer)
        return inv_qm
    elif inv_type == 0:
        raise Exception("mk_inverter")
    else:
        raise Exception("mk_inverter")