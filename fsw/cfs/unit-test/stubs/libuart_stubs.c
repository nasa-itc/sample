#include "utgenstub.h"
#include "hwlib.h"

int32_t uart_init_port(uart_info_t *device)
{
    UT_GenStub_SetupReturnBuffer(uart_init_port, int32_t);

    UT_GenStub_AddParam(uart_init_port, uart_info_t *, device);

    UT_GenStub_Execute(uart_init_port, Basic, NULL);

    return UT_GenStub_GetReturnValue(uart_init_port, int32_t);
}

int32_t uart_close_port(uart_info_t *device)
{
    UT_GenStub_SetupReturnBuffer(uart_close_port, int32_t);

    UT_GenStub_AddParam(uart_close_port, uart_info_t *, device);

    UT_GenStub_Execute(uart_close_port, Basic, NULL);

    return UT_GenStub_GetReturnValue(uart_close_port, int32_t);
}
