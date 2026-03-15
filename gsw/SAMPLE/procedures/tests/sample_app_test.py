from openc3.script import cmd, tlm, check
from sample_lib import *

def run_sample_app_test():
    # Get to known state
    safe_sample()

    ##
    ## Housekeeping, request telemetry to be published on the software bus
    ##
    for n in range(SAMPLE_TEST_LOOP_COUNT):
        get_sample_hk()

    ##
    ## NOOP, no operation but confirm correct counters increment
    ##
    for n in range(SAMPLE_TEST_LOOP_COUNT):
        sample_cmd("SAMPLE SAMPLE_NOOP_CC")

    ##
    ## Reset counters, increment as done in NOOP and confirm ability to clear repeatably
    ##
    for n in range(SAMPLE_TEST_LOOP_COUNT):
        sample_cmd("SAMPLE SAMPLE_NOOP_CC")
        cmd("SAMPLE SAMPLE_RST_COUNTERS_CC") # Note standard `cmd` as we can't reset counters and then confirm increment
        get_sample_hk()
        check("SAMPLE SAMPLE_HK_TLM CMD_COUNT == 0")
        check("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT == 0")

    ##
    ## Invalid ground command, confirm bad lengths and codes are rejected
    ##
    for n in range(SAMPLE_TEST_LOOP_COUNT):
        # Bad length
        cmd_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
        cmd_err_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT")
        cmd(f"SAMPLE SAMPLE_NOOP_CC with CCSDS_LENGTH {n+2}") # Note +2 due to CCSDS already being +1
        get_sample_hk()
        check(f"SAMPLE SAMPLE_HK_TLM CMD_COUNT == {cmd_cnt}")
        check(f"SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT == {cmd_err_cnt+1}")

    # Bad command codes (Equivalent to Ruby's 6..(5 + SAMPLE_TEST_LOOP_COUNT))
    for n in range(6, 6 + SAMPLE_TEST_LOOP_COUNT): 
        cmd_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_COUNT")
        cmd_err_cnt = tlm("SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT")
        cmd(f"SAMPLE SAMPLE_NOOP_CC with CCSDS_FC {n+1}")
        get_sample_hk()
        check(f"SAMPLE SAMPLE_HK_TLM CMD_COUNT == {cmd_cnt}")
        check(f"SAMPLE SAMPLE_HK_TLM CMD_ERR_COUNT == {cmd_err_cnt+1}")