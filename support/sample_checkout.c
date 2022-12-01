/*******************************************************************************
** File: sample_checkout.c
**
** Purpose:
**   This checkout can be run without cFS and is used to quickly develop and 
**   test functions required for a specific component.
**
*******************************************************************************/

/*
** Include Files
*/
#include "sample_checkout.h"


/*
** Component Functions
*/
void print_help(void) 
{
    printf(PROMPT "command [args]\n"
        "------------------------------------------------------------------------\n"
        "help                                    - Display help                  \n"
        "exit                                    - Exit app                      \n"
        "send_rw_trq [0-100]                     - Send RW Trq percentage 0-100  \n"
        "send_mtr_pwr [0/1]                      - Send RW motor on = 1          \n"
        "get_hk                                  - Get HK packet                 \n"
        "\n"
    );
}


int get_command(const char* str)
{
    // convert command to lower case
    char lcmd[MAX_INPUT_TOKEN_SIZE];
    strncpy(lcmd, str, MAX_INPUT_TOKEN_SIZE);
    to_lower(lcmd);

    if(strcmp(lcmd, "help") == 0) {
        return CMD_HELP;
    }
    else if(strcmp(lcmd, "exit") == 0) {
        return CMD_EXIT;
    }
    else if(strcmp(lcmd, "send_rw_trq") == 0) {
        return CMD_SEND_RW_TRQ;
    }
    else if(strcmp(lcmd, "send_mtr_pwr") == 0) {
        return CMD_SEND_MTR_PWR;
    }
    else if (strcmp(lcmd, "get_hk") == 0) {
        return CMD_GET_HK;
    }
    return CMD_UNKNOWN;
}


int main(int argc, char *argv[]) 
{
    char input_buf[MAX_INPUT_BUF];
    char input_tokens[MAX_INPUT_TOKENS][MAX_INPUT_TOKEN_SIZE];
    int num_input_tokens;
    int cmd;    
    char* token_ptr;
    uint8_t channel;
    uint8_t exit;
    int retVal;

    int32_t status = OS_SUCCESS;

    /* Open device specific protocols */
    uart_info_t SampleUart;
    SampleUart.deviceString = SAMPLE_CFG_STRING;
    SampleUart.handle = SAMPLE_CFG_HANDLE;
    SampleUart.isOpen = PORT_CLOSED;
    SampleUart.baud = SAMPLE_CFG_BAUDRATE_HZ;

    status = uart_init_port(&SampleUart);
    if (status == OS_SUCCESS)
    {
        printf("UART device %s configured with baudrate %d \n", SampleUart.deviceString, SampleUart.baud);
    }
    else
    {
        printf("UART device %s failed to initialize! \n", SampleUart.deviceString);
    }

    print_help();
    
    exit = 0;
    while(!exit) {
        num_input_tokens = -1;
        cmd = CMD_UNKNOWN;

        // read user input
        printf(PROMPT);
        fgets(input_buf, MAX_INPUT_BUF, stdin);

        // tokenize line buffer
        token_ptr = strtok(input_buf, " \t\n");
        while((num_input_tokens < MAX_INPUT_TOKENS) && (token_ptr != NULL)) {
            if(num_input_tokens == -1) {
                // first token is command
                cmd = get_command(token_ptr);
                // printf("CMD = %s %d\n",token_ptr,cmd);
            }
            else {
                strncpy(input_tokens[num_input_tokens], token_ptr, MAX_INPUT_TOKEN_SIZE);
                // printf("Token[%d] = %s\n",num_input_tokens,token_ptr);
            }
            token_ptr = strtok(NULL, " \t\n"); // continue tokenizing the string
            num_input_tokens++;
            // printf("Num tokens = %d\n",num_input_tokens);
        }

        // empty line
        if(num_input_tokens < 0) continue;	

        //struct can_frame msg;
        char* copy;
        char* token;
        char* pos;
        uint8_t buf[8];
        uint32_t id;

        // process command
        switch(cmd) 
        {	
            case CMD_HELP:
                print_help();
                break;
            
            case CMD_EXIT:
                exit = 1;
                break;
            
            case CMD_SEND_RW_TRQ:
                if (num_input_tokens == 1)
                {		
                    //rw_cmd.msg_type = 1;
                    //rw_cmd.cmd_id = 3;
                    //rw_cmd.src_mask = 10;
                    //rw_cmd.dest_mask - 1;
                    //rw_cmd.dlc = 7;		
                    //rw_cmd.data[0] = input_tokens[0];
                    //rw_cmd.data[1] = 0x00;
                    //cubewheel_send_cmd(rw_cmd);
                    printf("Valid command format!\n");
                    printf("  Sleeping for 5 seconds...\n");
                    OS_TaskDelay(5 * 1000);
                    printf("  Back\n");
                }
                else
                {
                    printf("Invalid command format, type 'help' for more info\n");
                }
                break;    
            
            case CMD_SEND_MTR_PWR:
                if (num_input_tokens == 1)
                {		
                    //rw_cmd.msg_type = 1;
                    //rw_cmd.cmd_id = 7;
                    //rw_cmd.src_mask = 10;
                    //rw_cmd.dest_mask - 1;
                    //rw_cmd.dlc = 6;		
                    //rw_cmd.data[0] = input_tokens[0];
                    //cubewheel_send_cmd(rw_cmd);
                    printf("Valid command format!\n");
                }
                else
                {
                    printf("Invalid command format, type 'help' for more info\n");
                }
                break;    
            
            case CMD_GET_HK: 
                printf("Valid command format!\n");
                break;
                //tlm_request.msg_type = 4;
                //tlm_request.cmd_id = 128;
                //tlm_request.src_mask = 0;
                //tlm_request.dest_mask = 1;
                //tlm_request.dlc = 0;
                //tlm_request.data = NULL;
                //can_recv_buffer = calloc(CUBEWHEEL_BASE_CMD_LEN + CUBEWHEEL_ID_INFO_LEN, sizeof(uint8));
                //cubewheel_request_tlm(tlm_request, can_recv_buffer, CUBEWHEEL_BASE_CMD_LEN + CUBEWHEEL_ID_INFO_LEN);
                //printf("RW HK Response: ");
                //cubewheel_print_hex(can_recv_buffer, CUBEWHEEL_BASE_CMD_LEN + CUBEWHEEL_ID_INFO_LEN);

            default: 
                printf("Invalid command format, type 'help' for more info\n");
                break;
        }	
        
    }	

    // Close the CAN device 
    //can_close_device(&canDev);

    printf("Exiting sample application\n\n"); 
    return 1;
}


/*
** Generic Functions
*/
void delay_ms(uint32_t ms_delay)
{
    usleep(ms_delay * 1000);
    return;
}


void to_lower(char* str)
{
    char* ptr = str;
    while(*ptr)
    {
        *ptr = tolower((unsigned char) *ptr);
        ptr++;
    }
    return;
}

