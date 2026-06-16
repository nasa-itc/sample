import sys
import glob

for p in glob.glob('/gems/gems/openc3-cosmos-nos3-*/targets/SAMPLE/scripts'):
    if p not in sys.path:
        sys.path.append(p)

from openc3.script import cmd, tlm, check
from nos3.sample_lib import *

def run_sample_ast_test():
    ##
    ## Hardware failure
    ##
    for n in range(SAMPLE_TEST_LOOP_COUNT):
        # Prepare
        sample_prepare_ast()

        # Disable sim and confirm device error counts increase
        dev_cmd_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_COUNT")
        dev_cmd_err_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ERR_COUNT")
        sample_sim_disable()
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_COUNT == {dev_cmd_cnt}")
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ERR_COUNT >= {dev_cmd_err_cnt}")

        # Enable sim and confirm return to nominal operation
        sample_sim_enable()
        confirm_sample_data_loop()

    ##
    ## Hardware status reporting fault
    ##
    for n in range(SAMPLE_TEST_LOOP_COUNT):
        # Prepare
        sample_prepare_ast()

        # Add a fault to status in the simulator
        sample_sim_set_status(255)

        # Confirm that status register and that app disabled itself
        get_sample_hk()
        check("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_STATUS == 255")
        get_sample_hk()
        check("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ENABLED == 'DISABLED'")
        
        # Clear simulator status fault
        sample_sim_set_status(0)