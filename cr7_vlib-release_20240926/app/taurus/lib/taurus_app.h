/*************************************************************************************************************
* taurus_app
* Copyright (c) [2021-2022] Renesas Electronics Corporation
* This software is released under the MIT License
* http://opensource.org/licenses/mit-license.php
*************************************************************************************************************/
/**
 * Function taurus_main
 * @brief   Application that communicates with the guest application in R-Car H3 CA5x.
 *          The application starts a task of communication server and the task waits to receive a command from CA5x.
 *          When a command is received the task sends a responce back to CA5x, executes the command and sends the result of the command back to CA5x.
 */
int taurus_main(void);
