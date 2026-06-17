import sys
import glob

for p in glob.glob('/gems/gems/openc3-cosmos-nos3-*/targets/SAMPLE/scripts'):
    if p not in sys.path:
        sys.path.append(p)

from openc3.script import cmd, tlm, check, wait_check
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
        
        # Actively force the FSW to try to poll the dead hardware
        cmd("SAMPLE_DEBUG SAMPLE_REQ_DATA")
        
        # Wait up to 5 seconds for the error count to increment
        wait_check(f"SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ERR_COUNT > {dev_cmd_err_cnt}", 5)
                
        # Use >= here because the background scheduler might have snuck in a successful read
        # right before the simulator fully closed its socket.
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_COUNT >= {dev_cmd_cnt}")

        # Enable sim and confirm return to nominal operation
        sample_sim_enable()
        get_sample_data()
        get_sample_hk()
        confirm_sample_data_loop()

    ##
    ## Hardware status reporting fault
    ##
    for n in range(SAMPLE_TEST_LOOP_COUNT):
        # Prepare
        sample_prepare_ast()

        # Add a fault to status in the simulator
        sample_sim_set_status(255)
        
        # Give the simulator bridge a moment to process the fault state
        import time
        time.sleep(1)

        # Force the FSW to request data so it actually reads the 255 status from the sim
        cmd("SAMPLE_DEBUG SAMPLE_REQ_DATA")

        # Wait for the FSW to report the bad status and auto-safe itself
        wait_check("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_STATUS == 255", 5)
        wait_check("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ENABLED == 'DISABLED'", 5)
        
        # Clear simulator status fault
        sample_sim_set_status(0)