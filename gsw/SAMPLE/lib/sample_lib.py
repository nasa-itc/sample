import time
from openc3.script import cmd, tlm, check, wait_check_packet

SAMPLE_CMD_SLEEP = 0.25
SAMPLE_RESPONSE_TIMEOUT = 5
SAMPLE_TEST_LOOP_COUNT = 1
SAMPLE_DEVICE_LOOP_COUNT = 5

def get_sample_hk():
    cmd("SAMPLE_DEBUG SAMPLE_REQ_HK")
    wait_check_packet("SAMPLE_DEBUG", "SAMPLE_HK_TLM", 1, SAMPLE_RESPONSE_TIMEOUT)
    time.sleep(SAMPLE_CMD_SLEEP)

def get_sample_data():
    cmd("SAMPLE_DEBUG SAMPLE_REQ_DATA")
    wait_check_packet("SAMPLE_DEBUG", "SAMPLE_DATA_TLM", 1, SAMPLE_RESPONSE_TIMEOUT)
    time.sleep(SAMPLE_CMD_SLEEP)

def sample_cmd(command_string):
    count = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT") + 1
    if count == 256:
        count = 0

    cmd(command_string)
    get_sample_hk()
    current = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT")
    
    if current != count:
        cmd(command_string)
        get_sample_hk()
        current = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT")
        if current != count:
            cmd(command_string)
            get_sample_hk()
            current = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT")
            
    check(f"SAMPLE_DEBUG SAMPLE_HK_TLM CMD_COUNT >= {count}")

def enable_sample():
    sample_cmd("SAMPLE_DEBUG SAMPLE_ENABLE_CC")
    check("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ENABLED == 'ENABLED'")

def disable_sample():
    sample_cmd("SAMPLE_DEBUG SAMPLE_DISABLE_CC")
    check("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ENABLED == 'DISABLED'")

def safe_sample():
    get_sample_hk()
    state = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ENABLED")
    if state != "DISABLED":
        disable_sample()

def confirm_sample_data():
    dev_cmd_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_COUNT")
    dev_cmd_err_cnt = tlm("SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ERR_COUNT")
    
    get_sample_data()
    raw_x = tlm("SAMPLE_DEBUG SAMPLE_DATA_TLM RAW_SAMPLE_X")
    check(f"SAMPLE_DEBUG SAMPLE_DATA_TLM RAW_SAMPLE_Y >= {raw_x * 2}")
    check(f"SAMPLE_DEBUG SAMPLE_DATA_TLM RAW_SAMPLE_Z >= {raw_x * 3}")

    get_sample_hk()
    check(f"SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_COUNT >= {dev_cmd_cnt}")
    check(f"SAMPLE_DEBUG SAMPLE_HK_TLM DEVICE_ERR_COUNT == {dev_cmd_err_cnt}")

def confirm_sample_data_loop():
    for _ in range(SAMPLE_DEVICE_LOOP_COUNT):
        confirm_sample_data()

def sample_prepare_ast():
    sample_sim_enable()
    safe_sample()
    enable_sample()
    confirm_sample_data_loop()

def sample_sim_enable():
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_SIM_ENABLE")
    time.sleep(1)

def sample_sim_disable():
    cmd("SIM_CMDBUS_BRIDGE SAMPLE_SIM_DISABLE")
    time.sleep(1)

def sample_sim_set_status(status):
    cmd(f"SIM_CMDBUS_BRIDGE SAMPLE_SIM_SET_STATUS with STATUS {status}")
    time.sleep(1)

def restore_sample():
    sample_sim_enable()
    sample_sim_set_status(0)
    safe_sample()
    enable_sample()
    confirm_sample_data_loop()