import sys
import glob

for p in glob.glob('/gems/gems/openc3-cosmos-nos3-*/targets/SAMPLE/scripts'):
    if p not in sys.path:
        sys.path.append(p)

from openc3.script import cmd, tlm, check
from nos3.sample_lib import *

def run_sample_device_test():
    ##
    ## Enable / disable, control hardware communications
    ##
    for n in range(SAMPLE_TEST_LOOP_COUNT):
        # Get to known state
        safe_sample()

        # Manually command to disable when already disabled
        cmd_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT")
        cmd_err_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_ERR_COUNT")
        cmd("SAMPLE_DEBUG SAMPLE_DISABLE_CC")
        get_sample_hk()
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT == {cmd_cnt}")
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM CMD_ERR_COUNT == {cmd_err_cnt+1}")

        # Enable
        enable_sample()

        get_sample_data()
        get_sample_hk()

        # Confirm device counters increment without errors
        confirm_sample_data_loop()

        # Manually command to enable when already enabled
        cmd_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT")
        cmd_err_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_ERR_COUNT")
        cmd("SAMPLE_DEBUG SAMPLE_ENABLE_CC")
        get_sample_hk()
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT == {cmd_cnt}")
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM CMD_ERR_COUNT == {cmd_err_cnt+1}")

        # Reconfirm data remains as expected
        confirm_sample_data_loop()

        # Disable
        disable_sample()


    ##
    ## Configuration, reconfigure sample instrument register
    ##
    for n in range(SAMPLE_TEST_LOOP_COUNT):
        # Get to known state
        safe_sample()

        # Confirm configuration command denied if disabled
        cmd_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT")
        cmd_err_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_ERR_COUNT")
        cmd("SAMPLE_DEBUG SAMPLE_CONFIG_CC with DEVICE_CONFIG 10")
        get_sample_hk()
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT == {cmd_cnt}")
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM CMD_ERR_COUNT == {cmd_err_cnt+1}")
        
        # Enable
        enable_sample()

        get_sample_data()
        get_sample_hk()

        # Set configuration
        sample_cmd(f"SAMPLE_DEBUG SAMPLE_CONFIG_CC with DEVICE_CONFIG {n+1}")
        check(f"SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_CONFIG == {n+1}")