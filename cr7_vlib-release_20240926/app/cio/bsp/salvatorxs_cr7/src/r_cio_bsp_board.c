#include "r_prr_api.h"
#include "r_cio_bsp_board.h"

/* TODO: We currently only support boards that can be determined by the SoC.
 * If there are two boards with the same SoC, we will probably have to use
 * a build time option to distinguish between them. */

r_cio_board_family_t prvGetBoardFamilyId(void)
{
    r_prr_DevName_t dev = R_PRR_GetDevice();
    r_cio_board_family_t family = -1;

    switch (dev)
    {
    case R_PRR_RCARE3:
        family = R_BOARD_EBISU;
        break;
    case R_PRR_RCARH3:
    case R_PRR_RCARM3W:
    case R_PRR_RCARM3N:
        family = R_BOARD_SALVATOR_XS;
        break;
    default:
        break;
    }

    return family;
}

r_cio_board_t prvGetBoardId(void)
{
    r_prr_DevName_t dev = R_PRR_GetDevice();
    r_cio_board_t board = -1;

    switch (dev)
    {
    case R_PRR_RCARE3:
        board = R_BOARD_EBISU_E3;
        break;
    case R_PRR_RCARH3:
        board = R_BOARD_SALVATOR_XS_H3;
        break;
    case R_PRR_RCARM3W:
        board = R_BOARD_SALVATOR_XS_M3W;
        break;
    case R_PRR_RCARM3N:
        board = R_BOARD_SALVATOR_XS_M3N;
        break;
    default:
        break;
    }

    return board;
}
