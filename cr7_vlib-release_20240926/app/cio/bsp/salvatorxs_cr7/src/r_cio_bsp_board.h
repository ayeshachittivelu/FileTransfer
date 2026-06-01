
#ifndef CIO_BSP_BOARD_H
#define CIO_BSP_BOARD_H

typedef enum {
    R_BOARD_EBISU,
    R_BOARD_SALVATOR_XS,
} r_cio_board_family_t;

typedef enum {
    R_BOARD_EBISU_E3,
    R_BOARD_SALVATOR_XS_H3,
    R_BOARD_SALVATOR_XS_M3W,
    R_BOARD_SALVATOR_XS_M3N,
} r_cio_board_t;

r_cio_board_family_t prvGetBoardFamilyId(void);

r_cio_board_t prvGetBoardId(void);

#endif /* CIO_BSP_BOARD_H */
